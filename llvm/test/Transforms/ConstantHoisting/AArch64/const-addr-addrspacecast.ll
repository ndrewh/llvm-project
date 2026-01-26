; RUN: opt -mtriple=arm64-darwin-unknown -S -passes=consthoist < %s | FileCheck %s

%T = type { i32, i32, i32, i32 }

define i32 @test1() nounwind {
; CHECK-LABEL: test1
; CHECK: %1 = inttoptr i64 68141056 to ptr
; CHECK: %o1 = getelementptr %T, ptr %1, i32 0, i32 1
; CHECK: %c2 = addrspacecast ptr %1 to ptr addrspace(1)
; CHECK: %o2 = getelementptr %T, ptr addrspace(1) %c2, i32 0, i32 2
; CHECK: %c3 = addrspacecast ptr %1 to ptr addrspace(1)
; CHECK: %t3 = load i32, ptr addrspace(1) %c3, align 4
  %1 = inttoptr i64 68141056 to ptr
  ; Check that addresspacecast(gep ...) is hoisted
  %o1 = getelementptr %T, ptr %1, i32 0, i32 1
  %c1 = addrspacecast ptr %o1 to ptr addrspace(1)
  %t1 = load i32, ptr addrspace(1) %c1
  ; Check that gep(addresspacecast ...) is hoisted
  %c2 = addrspacecast ptr %1 to ptr addrspace(1)
  %o2 = getelementptr %T, ptr addrspace(1) %c2, i32 0, i32 2
  %t2 = load i32, ptr addrspace(1) %o2
  %a1 = add i32 %t1, %t2
  ; Check that direct load of addresspacecast is hoisted
  %c3 = addrspacecast ptr %1 to ptr addrspace(1)
  %t3 = load i32, ptr addrspace(1) %c3
  %a2 = add i32 %a1, %t3
  ret i32 %a2
}

