; RUN: opt -load-pass-plugin=%llvmshlibdir/kazennova_a_lab2_LLVM_IR%pluginext -passes="decompose-fmuladd" -S %s | FileCheck %s

define float @test_float(float %a, float %b, float %c) {
entry:
  %r = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %r
}

; CHECK-LABEL: define float @test_float
; CHECK: entry:
; CHECK: [[MUL:%[0-9]+]] = fmul float %a, %b
; CHECK: [[ADD:%[0-9]+]] = fadd float [[MUL]], %c
; CHECK: ret float [[ADD]]

define double @test_double(double %a, double %b, double %c) {
entry:
  %r = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  ret double %r
}

; CHECK-LABEL: define double @test_double
; CHECK: entry:
; CHECK: [[MUL:%[0-9]+]] = fmul double %a, %b
; CHECK: [[ADD:%[0-9]+]] = fadd double [[MUL]], %c
; CHECK: ret double [[ADD]]

define half @test_half(half %a, half %b, half %c) {
entry:
  %r = call half @llvm.fmuladd.f16(half %a, half %b, half %c)
  ret half %r
}

; CHECK-LABEL: define half @test_half
; CHECK: entry:
; CHECK: [[MUL:%[0-9]+]] = fmul half %a, %b
; CHECK: [[ADD:%[0-9]+]] = fadd half [[MUL]], %c
; CHECK: ret half [[ADD]]

; Тест для fast-math флагов

define float @test_fast_math(float %a, float %b, float %c) {
entry:
  %r = call fast float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %r
}

; CHECK-LABEL: define float @test_fast_math
; CHECK: entry:
; CHECK: [[MUL:%[0-9]+]] = fmul fast float %a, %b
; CHECK: [[ADD:%[0-9]+]] = fadd fast float [[MUL]], %c
; CHECK: ret float [[ADD]]