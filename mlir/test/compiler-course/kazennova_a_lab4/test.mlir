// RUN: mlir-opt --load-pass-plugin=/home/az890/projects/compiler-course-2026/build/lib/kazennova_a_lab4_MLIR.so --pass-pipeline="builtin.module(max-nesting-depth)" %s | FileCheck %s

// Тест 1: Функция без блоков – глубина 0
// CHECK-LABEL: func.func @test_no_blocks
// CHECK-SAME: attributes {max_nesting_depth = "0"}
func.func @test_no_blocks() -> i32 {
  %c = arith.constant 42 : i32
  return %c : i32
}

// Тест 2: Один affine.for – глубина 1
// CHECK-LABEL: func.func @test_affine_for
// CHECK-SAME: attributes {max_nesting_depth = "1"}
func.func @test_affine_for() {
  affine.for %i = 0 to 10 {
    %c = arith.constant 0 : i32
  }
  return
}

// Тест 3: Два вложенных affine.for – глубина 2
// CHECK-LABEL: func.func @test_affine_nested
// CHECK-SAME: attributes {max_nesting_depth = "2"}
func.func @test_affine_nested() {
  affine.for %i = 0 to 10 {
    affine.for %j = 0 to 10 {
      %c = arith.constant 0 : i32
    }
  }
  return
}

// Тест 4: Три вложенных affine.for – глубина 3
// CHECK-LABEL: func.func @test_affine_nested_three
// CHECK-SAME: attributes {max_nesting_depth = "3"}
func.func @test_affine_nested_three() {
  affine.for %i = 0 to 10 {
    affine.for %j = 0 to 10 {
      affine.for %k = 0 to 10 {
        %c = arith.constant 0 : i32
      }
    }
  }
  return
}

// Тест 6: Один scf.for – глубина 1
// CHECK-LABEL: func.func @test_scf_for
// CHECK-SAME: attributes {max_nesting_depth = "1"}
func.func @test_scf_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
    %c = arith.constant 0 : i32
  }
  return
}

// Тест 7: Вложенные scf.for – глубина 2
// CHECK-LABEL: func.func @test_scf_nested
// CHECK-SAME: attributes {max_nesting_depth = "2"}
func.func @test_scf_nested() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
    scf.for %j = %c0 to %c10 step %c1 {
      %c = arith.constant 0 : i32
    }
  }
  return
}

// Тест 8: scf.if внутри scf.for – глубина 2
// CHECK-LABEL: func.func @test_scf_if_in_for
// CHECK-SAME: attributes {max_nesting_depth = "2"}
func.func @test_scf_if_in_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %true = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %true {
      %c = arith.constant 0 : i32
    }
  }
  return
}

// Тест 9: scf.while – глубина 1 (тело while считается блоком)
// CHECK-LABEL: func.func @test_scf_while
// CHECK-SAME: attributes {max_nesting_depth = "1"}
func.func @test_scf_while() -> i32 {
  %c0 = arith.constant 0 : i32
  %c10 = arith.constant 10 : i32
  %res = scf.while (%arg0 = %c0) : (i32) -> i32 {
    %cmp = arith.cmpi slt, %arg0, %c10 : i32
    scf.condition(%cmp) %arg0 : i32
  } do {
  ^bb0(%arg0: i32):
    %next = arith.addi %arg0, %c0 : i32
    scf.yield %next : i32
  }
  return %res : i32
}

// Тест 10: Смешанные scf.for + affine.for – глубина 2
// CHECK-LABEL: func.func @test_mixed_scf_affine
// CHECK-SAME: attributes {max_nesting_depth = "2"}
func.func @test_mixed_scf_affine() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
    affine.for %j = 0 to 10 {
      %c = arith.constant 0 : i32
    }
  }
  return
}

// Тест 11: Смешанные affine.for + scf.if – глубина 2
// CHECK-LABEL: func.func @test_mixed_affine_scf_if
// CHECK-SAME: attributes {max_nesting_depth = "2"}
func.func @test_mixed_affine_scf_if() {
  %true = arith.constant true
  affine.for %i = 0 to 10 {
    scf.if %true {
      %c = arith.constant 0 : i32
    }
  }
  return
}