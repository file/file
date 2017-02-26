; ModuleID = 'opencl.o'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a64:64:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-applecl-macosx10.9.0"

@bar = addrspace(2) global i32 0, align 4
