// RUN: circt-opt %s --hw-split-bitvector-ports | FileCheck %s
module {
  // CHECK-LABEL: hw.module @child
  // CHECK-SAME: in %x : i8
    hw.module @child(in %x : i8, out y : i8) {
      %one = hw.constant 1 : i8
      %y = comb.add %x, %one : i8
      hw.output %y : i8
    }
  // CHECK-LABEL: hw.module @top
  // CHECK-SAME: in %a_0 : i1
  // CHECK-SAME: in %a_7 : i1
    hw.module @top(in %a : i8, out y : i8) {
      // CHECK: %[[CONCAT:.+]] = comb.concat %a_7, %a_6, %a_5, %a_4, %a_3, %a_2, %a_1, %a_0
      %child.y = hw.instance "child" @child(x: %a : i8) -> (y : i8)

      hw.output %child.y : i8
    }
  }