#include "circt/Dialect/Comb/CombOps.h"
#include "circt/Dialect/HW/HWOps.h"
#include "circt/Dialect/HW/HWPasses.h"
#include "mlir/IR/AttrTypeSubElements.h"
#include "mlir/IR/Iterators.h"
#include "mlir/Pass/Pass.h"

namespace circt {
namespace hw {
#define GEN_PASS_DEF_HWSPLITBITVECTORPORTS
#include "circt/Dialect/HW/Passes.h.inc"
} // namespace hw
} // namespace circt

using namespace mlir;
using namespace circt;
using namespace hw;

namespace {
struct HWSplitBitVectorPortsPass
    : circt::hw::impl::HWSplitBitVectorPortsBase<HWSplitBitVectorPortsPass> {
  using circt::hw::impl::HWSplitBitVectorPortsBase<
      HWSplitBitVectorPortsPass>::HWSplitBitVectorPortsBase;

  void runOnOperation() override;
  // static bool isTypeSupported(Type ty);
  // LogicalResult convertBitcastOp(OpBuilder builder, BitcastOp bitcastOp);
};

} // namespace
void HWSplitBitVectorPortsPass::runOnOperation() {};
