; ModuleID = 'while_test.c'
source_filename = "while_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* %2, align 4
  store i32 3, i32* @a, align 4
  br label %3

3:                                                ; preds = %11, %0
  %4 = load i32, i32* @a, align 4
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %9

6:                                                ; preds = %3
  %7 = load i32, i32* %2, align 4
  %8 = icmp slt i32 %7, 10
  br label %9

9:                                                ; preds = %6, %3
  %10 = phi i1 [ false, %3 ], [ %8, %6 ]
  br i1 %10, label %11, label %17

11:                                               ; preds = %9
  %12 = load i32, i32* %2, align 4
  %13 = load i32, i32* @a, align 4
  %14 = add nsw i32 %12, %13
  store i32 %14, i32* %2, align 4
  %15 = load i32, i32* @a, align 4
  %16 = sub nsw i32 %15, 1
  store i32 %16, i32* @a, align 4
  br label %3

17:                                               ; preds = %9
  %18 = load i32, i32* %1, align 4
  ret i32 %18
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}
