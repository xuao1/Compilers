; ModuleID = 'while_test.c'
source_filename = "while_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 全部变量 int a
@a = dso_local global i32 0, align 4

define dso_local i32 @main() #0 {   ; main
    %1 = alloca i32, align 4
    store i32 0, i32* %1, align 4

    %2 = alloca i32, align 4        ; %2是局部变量b的地址值
    store i32 0, i32* %2, align 4   ; b=0
    %3 = load i32, i32* %2, align 4 ; %3=b

    store i32 3, i32* @a, align 4   ; a=3
    %4 = load i32, i32* @a, align 4 ; %4=a

    br label %5                     ; 跳转到while的条件判断句

5:  ; while的条件判断句
    %6 = icmp sgt i32 %4, 0         ; a>0
    ; %6 是比较结果  
    br i1 %6, label %7, label %12    ; a>0,到 lable7;否则到label12

7:  ; a>0
    %8 = icmp slt i32 %3, 0         ; b<10
    ; %8 是比较结果  
    br i1 %8, label %9, label %12    ; b<10,到 lable9;否则到label12

9:  ; while的条件满足
    %10 = add nsw i32 %3, %4        ; %10=b+a
    store i32 %10, i32* %2, align 4 ; b=b+a
    %11 = sub nsw i32 %4, 1         ; %11=a-1    
    store i32 %11, i32* @a, align 4 ; a=a-1
    br label %5                     ; 执行完while体，回到while的条件判断语句

12: ;while的条件不满足
    %13 = load i32, i32* %1, align 4
    ret i32 %13
}


attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}