#include "circt/Dialect/Comb/CombOps.h"
#include "circt/Dialect/HW/HWOps.h"
#include "circt/Dialect/HW/HWPasses.h"
#include "circt/Dialect/HW/PortImplementation.h"
#include "circt/Support/LLVM.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/SmallVector.h"

namespace circt {
namespace hw {
#define GEN_PASS_DEF_HWSPLITBITVECTORPORTS
#include "circt/Dialect/HW/Passes.h.inc"
} // namespace hw
} // namespace circt

using namespace mlir;
using namespace circt;
using namespace hw;

static bool isNeedSplitPort(hw::PortInfo port) {
  // Only split the Input Integer Ports (Width > 1)
  bool direction = port.isInput();
  auto portType = mlir::dyn_cast<mlir::IntegerType>(port.type);
  bool needSplit = direction && portType && portType.getWidth() > 1;
  return needSplit;
}

namespace {
struct HWSplitBitVectorPortsPass
    : circt::hw::impl::HWSplitBitVectorPortsBase<HWSplitBitVectorPortsPass> {
  using circt::hw::impl::HWSplitBitVectorPortsBase<
      HWSplitBitVectorPortsPass>::HWSplitBitVectorPortsBase;

  void runOnOperation() override;
};

} // namespace

void HWSplitBitVectorPortsPass::runOnOperation() {
  mlir::ModuleOp design = getOperation();
  hw::HWModuleOp op = design.lookupSymbol<hw::HWModuleOp>(moduleName);

  if (!op) {
    design->emitError("Cannot find hw.module @top");
    signalPassFailure();
    return;
  }

  SmallVector<hw::PortInfo> portsInfoToSplit;

  for (hw::PortInfo port : op.getPortList()) {
    if (isNeedSplitPort(port)) {
      // Record Ports Info to Split
      portsInfoToSplit.push_back(port);
    } else {
      continue;
    }
  }
  // if there is nothing to split, abort
  if (portsInfoToSplit.empty())
    return;
  // Split the Ports
  for (hw::PortInfo port : portsInfoToSplit) {
    auto index = port.argNum;
    auto portType = mlir::dyn_cast<mlir::IntegerType>(port.type);
    auto portWidth = portType.getWidth();

    SmallVector<Value> newOpBits;
    auto prevArgs = op.getBody().getArgument(index);
    //  1. 先保存 BlockArgument oldArg（插入前）；
    //  2. 循环 insertInput，收集 8 个新参数到 newOpBits；
    //  3. 检查 newOpBits 的顺序是否符合 concat 的 MSB→LSB 约定（需要反转）；
    //  4. 用正确构造的 builder 在 body 开头创建 concat；
    //  5. oldArg.replaceAllUsesWith(concat 结果)；
    //  6. 删除旧端口和旧 body 参数（注意此时旧参数的编号已经移动了）。
    for (unsigned i = 0; i < portWidth; ++i) {
      auto [name, arg] = op.insertInput(
          index + i,
          StringAttr::get(op->getContext(), port.name.str() + "_" + Twine(i)),
          mlir::IntegerType::get(op.getContext(), 1));
      newOpBits.push_back(arg); // Record new BlockArgument
    }
    // Reverse the order of newOpBits, should be MSB -> LSB
    std::reverse(newOpBits.begin(), newOpBits.end());

    // Get builder at the beginning of the block to create concat op
    // for example:
    // ```
    // hw.module @top (...) { %child.y = hw.instance @child(...) }
    // ```
    // We want to create a concat op like this:
    // ```
    // hw.module @top (...) {
    //   %a_combined = comb.concat %a_7, ..., %a_0
    //   <- This is the beginning of the block
    //   %child.y = hw.instance @child(x:%a_combined, ...)
    // }
    // ```
    // In order to create insert point at the beginning of the block, we need to
    // use `OpBuilder::atBlockBegin()`. `builder` is a pointer to the beginning
    // of the block, so we can use it to create the concat op.
    auto builder = OpBuilder::atBlockBegin(op.getBodyBlock());
    // Create a concat op to combine the new inputs into the original bitvector
    // Use `prevArgs` to get the original argument, because the new arguments
    // have been inserted, the original argument has been shifted to the right.
    // So `getLoc()` and `replaceAllUsesWith()` should be called on `prevArgs`,
    // not `op.getBody().getArgument(index)`.
    auto concatOp =
        comb::ConcatOp::create(builder, prevArgs.getLoc(), newOpBits);
    prevArgs.replaceAllUsesWith(concatOp);

    assert(prevArgs.use_empty() && "Old port still has uses after replacement");
    op.erasePorts(index + portWidth, {});
    // Erase old body argument
    op.getBodyBlock()->eraseArgument(index + portWidth);
  }
};
