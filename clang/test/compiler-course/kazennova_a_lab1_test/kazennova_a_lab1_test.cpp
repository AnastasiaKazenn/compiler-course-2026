// RUN: %clang_cc1 -load %llvmshlibdir/kazennova_a_lab1_ClangAST%pluginext -plugin kazennova_a_lab1_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK-LABEL: test_simple
void test_simple() {
  int x = 10;
  float y = (float)x;
  // CHECK: float y = static_cast<float>(x);
}

// CHECK-LABEL: test_expression
void test_expression() {
  int a = 5, b = 3;
  double d = (double)(a + b);
  // CHECK: double d = static_cast<double>(a + b);
}

// CHECK-LABEL: test_pointer
void test_pointer() {
  int *ptr = 0;
  void *v = (void *)ptr;
  // CHECK: void *v = static_cast<void *>(ptr);
}

// CHECK-LABEL: test_multiple
void test_multiple() {
  int x = 10, y = 20;
  float f = (float)x + (float)y;
  // CHECK: float f = static_cast<float>(x) + static_cast<float>(y);
}

// CHECK-LABEL: test_complex
void test_complex() {
  int x = 1, y = 2;
  float f = (float)(x + y) * (float)(x - y);
  // CHECK: float f = static_cast<float>(x + y) * static_cast<float>(x - y);
}