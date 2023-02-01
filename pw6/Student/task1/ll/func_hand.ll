; ModuleID = 'func_test.c'
source_filename = "func_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define dso_local i32 @add(i32 %0, i32 %1) #0 {  ; int add(int a, int b)
    ; instruction expected to be %3
    %3 = alloca i32, align 4    ; 返回值存储的地址值
    %4 = add nsw i32 %0, %1     ; %4=a+b
    ret i32 %4
}


define dso_local i32 @main() #0 {   ; main 函数
    %1 = alloca i32, align 4
    store i32 0, i32* %1, align 4

    %2 = alloca i32, align 4    ; 局部变量a，%2是a的地址值
    %3 = alloca i32, align 4    ; 局部变量c，%3是c的地址值
    store i32 3, i32* %2, align 4   ; a=3
    store i32 5, i32* %3, align 4   ; c=5

    %4 = load i32, i32* %2, align 4 ; %4=a 
    %5 = load i32, i32* %3, align 4 ; %5=c
    %6 = add nsw i32 %4, %4         ; %6=a+a

    %7 = call i32 @add(i32 %4, i32 %6)  ; 函数调用

    %8 = add nsw i32 %5, %7     ; %8=c+add(a,a+a)
    ret i32 %8
}


attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}