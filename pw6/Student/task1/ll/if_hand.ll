; ModuleID = 'if_test.c'
source_filename = "if_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 全部变量 int a
@a = dso_local global i32 0, align 4

define dso_local i32 @main() #0 {   ; main
    %1 = alloca i32, align 4
    store i32 0, i32* %1, align 4

    store i32 10, i32* @a, align 4  ; a=10
    %2 = load i32, i32* @a, align 4 ; %2=a

    %3 = icmp sgt i32 %2, 0         ; a>0
    ; %3 是比较结果  
    br i1 %3, label %4, label %5    ; a>0,到 lable4;否则到label5

4: ; a<0
    ret i32 %2                      ; return a

5: ; a>=0
    %6 = icmp slt i32 %2, 6         ; a<6
    br i1 %6, label %4, label %7    ; a<6,到label4;否则到lable7

7:  ; if不成立
    %8 = sub nsw i32 0, %2          ; %8=-a
    ret i32 %8                      ; return -a
}


attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}