
; RUN: llc -mtriple=arm64-darwin-unknown < %s | FileCheck %s

%T = type { i32, i32, i32, i32 }

; Test if the constant base address gets only materialized once.
define i32 @test1() nounwind {
; CHECK-LABEL:  test1
; CHECK:        mov  w8, #49152
; CHECK-NEXT:   movk  w8, #1039, lsl #16
; CHECK-NEXT:   ldp w10, w9, [x8, #4]
; CHECK:        ldr w8, [x8]
  %at = inttoptr i64 68141056 to ptr
  ; Check that addresspacecast(gep ...) is hoisted
  %o1 = getelementptr %T, ptr %at, i32 0, i32 1
  %c1 = addrspacecast ptr %o1 to ptr addrspace(1)
  %t1 = load i32, ptr addrspace(1) %c1
  ; Check that gep(addresspacecast ...) is hoisted
  %c2 = addrspacecast ptr %at to ptr addrspace(1)
  %o2 = getelementptr %T, ptr addrspace(1) %c2, i32 0, i32 2
  %t2 = load i32, ptr addrspace(1) %o2
  %a1 = add i32 %t1, %t2
  ; Check that direct load of addresspacecast is hoisted
  %c3 = addrspacecast ptr %at to ptr addrspace(1)
  %t3 = load i32, ptr addrspace(1) %c3
  %a2 = add i32 %a1, %t3
  ret i32 %a2
}

; Test if several related constants share one materialized base
define i32 @test2() nounwind {
; CHECK-LABEL:  test2
; CHECK:        mov  w8, #49152
; CHECK-NEXT:   movk  w8, #1039, lsl #16
; CHECK-NEXT:   ldr w10, [x8, #4]
; CHECK:        ldp w8, w9, [x8, #8]
  %at1 = inttoptr i64 68141056 to ptr
  %at2 = inttoptr i64 68141060 to ptr
  %at3 = inttoptr i64 68141064 to ptr
  ; Check that addresspacecast(gep ...) is hoisted
  %o1 = getelementptr %T, ptr %at1, i32 0, i32 1
  %c1 = addrspacecast ptr %o1 to ptr addrspace(1)
  %t1 = load i32, ptr addrspace(1) %c1
  ; Check that gep(addresspacecast ...) is hoisted
  %c2 = addrspacecast ptr %at2 to ptr addrspace(1)
  %o2 = getelementptr %T, ptr addrspace(1) %c2, i32 0, i32 2
  %t2 = load i32, ptr addrspace(1) %o2
  %a1 = add i32 %t1, %t2
  ; Check that direct load of addresspacecast is hoisted
  %c3 = addrspacecast ptr %at3 to ptr addrspace(1)
  %t3 = load i32, ptr addrspace(1) %c3
  %a2 = add i32 %a1, %t3
  ret i32 %a2
}