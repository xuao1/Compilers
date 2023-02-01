; ModuleID = 'go_upstairs.c'
source_filename = "go_upstairs.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@num = dso_local global [2 x i32] [i32 4, i32 8], align 4
@tmp = dso_local global i32 1, align 4
@n = dso_local global i32 0, align 4
@x = dso_local global [1 x i32] zeroinitializer, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @climbStairs(i32 %0) #0 {
  %2 = alloca i32, align 4          ; 返回值存储的地址值
  %3 = alloca i32, align 4          ; 参数n存储的地址值
  %4 = alloca [10 x i32], align 16  ; %4是dp[10]
  %5 = alloca i32, align 4          ; 局部变量i的地址值
  store i32 %0, i32* %3, align 4    ; n的地址存入%3
  %6 = load i32, i32* %3, align 4   ; %6=n
  %7 = icmp slt i32 %6, 4           ; 比较n和4，小于
  br i1 %7, label %8, label %10     ; %7 是比较结果 

8:                                  ; n<4              ; preds = %1
  %9 = load i32, i32* %3, align 4   ; %9=n
  store i32 %9, i32* %2, align 4    ; n存入%2代表的地址
  br label %41                      ; 无条件分支,return n

10:                                               ; preds = %1
  %11 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 0
  store i32 0, i32* %11, align 16   ; dp[0]=0
  %12 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 1
  store i32 1, i32* %12, align 4    ; dp[1]=1
  %13 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 2
  store i32 2, i32* %13, align 8    ; dp[2]=2
  store i32 3, i32* %5, align 4     ; i = 3
  br label %14                      ; 无条件分支

14:                                 ; i<n+1                ; preds = %19, %10
  %15 = load i32, i32* %5, align 4  ; %15=i
  %16 = load i32, i32* %3, align 4  ; %16=n
  %17 = add nsw i32 %16, 1          ; %17=n+1
  %18 = icmp slt i32 %15, %17       ; 看i<n+1
  br i1 %18, label %19, label %36   ; i<n+1,跳至%19;否则%36

19:                                               ; preds = %14
  %20 = load i32, i32* %5, align 4  ; %20=i
  %21 = sub nsw i32 %20, 1          ; %21=i-1
  %22 = sext i32 %21 to i64         ; 
  %23 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 %22
  %24 = load i32, i32* %23, align 4 ; %24=dp[i-1]
  %25 = load i32, i32* %5, align 4  ; %25=i
  %26 = sub nsw i32 %25, 2          ; %26=i-2
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 %27
  %29 = load i32, i32* %28, align 4 ; %29=dp[i-2]
  %30 = add nsw i32 %24, %29        ; %30=dp[i-1]+dp[i-2]
  %31 = load i32, i32* %5, align 4  ; %31=i
  %32 = sext i32 %31 to i64         ; 
  %33 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 %32
  store i32 %30, i32* %33, align 4  ; dp[i]=%30
  %34 = load i32, i32* %5, align 4  ; %34=i
  %35 = add nsw i32 %34, 1          ; %35=%34+1
  store i32 %35, i32* %5, align 4   ; i=%35
  br label %14                      ; 无条件跳至%14，即while循环的条件判断语句

36:                                               ; preds = %14
  %37 = load i32, i32* %3, align 4  ; %37=n
  %38 = sext i32 %37 to i64         ; 
  %39 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 %38
  %40 = load i32, i32* %39, align 4 ; %40=dp[n]
  store i32 %40, i32* %2, align 4   ; 把%40存到返回值地址中
  br label %41                      ; 跳到%41

41:                                       ; return        ; preds = %36, %8
  %42 = load i32, i32* %2, align 4  ; 
  ret i32 %42
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {   ; main
  %1 = alloca i32, align 4          ; 
  %2 = alloca i32, align 4          ; 局部变量 res
  store i32 0, i32* %1, align 4     ; 
  %3 = load i32, i32* getelementptr inbounds ([2 x i32], [2 x i32]* @num, i64 0, i64 0), align 4
  ; 后半部分num[0]的地址，%3=num[0]
  store i32 %3, i32* @n, align 4    ; n=num[0]
  %4 = load i32, i32* @tmp, align 4 ; %4=tmp
  %5 = sext i32 %4 to i64           ; 
  %6 = getelementptr inbounds [2 x i32], [2 x i32]* @num, i64 0, i64 %5
  %7 = load i32, i32* %6, align 4   ; %7=num[tmp]
  store i32 %7, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4
  ; 后者是x[0]的地址，x[0]=num[tmp]
  %8 = load i32, i32* @n, align 4   ; %8=n 
  %9 = load i32, i32* @tmp, align 4 ; %9=tmp
  %10 = add nsw i32 %8, %9          ; %10=n+tmp
  %11 = call i32 @climbStairs(i32 %10)  ; 函数调用
  ; %11 是子函数返回值
  store i32 %11, i32* %2, align 4   ; res=%11
  %12 = load i32, i32* %2, align 4  ; %12 = res
  %13 = load i32, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4
  ; 后者是x[0]的地址值，%13=x[0]
  %14 = sub nsw i32 %12, %13        ; %14=%12-%14=res-x[0]
  ret i32 %14                       ; 返回
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}
