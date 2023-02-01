; ModuleID = 'assign_test.c'
source_filename = "assign_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() #0 {   ; main
    %1 = alloca i32, align 4        ; 返回值地址值  
    store i32 0, i32* %1, align 4       
    %2 = alloca float, align 4      ; 局部变量b的地址值
    ;store float 1.8, float* %2, align 4
    store float 0x3FFCCCCCC0000000, float* %2, align 4
    %3 = alloca [2 x i32], align 4  ; 局部数组变量a[2]的地址值
    
    ; %4 = load i32, i32* getelementptr inbounds ([2 x i32], [2 x i32]* %3, i64 0, i64 0), align 4
    ; 后半部分是a[0]的地址值，%4=a[0]
    
    %4 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0
    ; %4是a[0]的地址值
    %5 = load i32, i32* %4, align 4 ; %5=a[0]

    %6 = sitofp i32 %5 to float     ; %6=(float)a[0]
    %7 = load float, float* %2, align 4     ; %7=b
    %8 = fmul float %6, %7          ; %8=b*a[0]
    %9 = fptosi float %8 to i32     ; %9=(int)%8

    %10 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1
    ; %10是a[1]的地址值
    store i32 %9, i32* %10, align 4 ; a[1]=%9

    %11 = fptosi float %7 to i32
    ret i32 %11                   ; 返回
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}