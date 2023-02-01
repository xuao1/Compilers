; ModuleID = 'io.c'
source_filename = "io.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define dso_local i32 @main() #0 {   ; main
    %1 = alloca i32, align 4
    store i32 0, i32* %1, align 4

    %2 = alloca i32, align 4        ; 局部变量a的地址值
    %3 = alloca [10000 x float], align 16   ; 数组b的地址值
    %4 = alloca i32, align 4        ; 局部变量b的地址值

    %5 = getelementptr inbounds [10000 x float], [10000 x float]* %3, i64 0, i64 0
    ; b[0]的地址值
    store float 1.000000e+00, float* %5, align 4    ; b[0]=1.0
    %6 = getelementptr inbounds [10000 x float], [10000 x float]* %3, i64 0, i64 1
    ; b[1]的地址值
    store float 2.000000e+00, float* %6, align 4    ; b[1]=2.0

    ; int a = getint();
    %7 = call i32 @getint()         ; getint()
    store i32 %7, i32* %2, align 4

    ; putint(a)
    %8 = load i32, i32* %2, align 4 ; %8=a
    call void @putint(i32 %8)

    ; int n = getfarray(b);
    %9 = call i32 @getfarray(float* %5)     ; 传b[0]的地址值
    store i32 %9, i32* %4, align 4  ; n=%9

    ; putfarray(n+1, b);
    %10 = load i32, i32* %4, align 4        ; %10=n
    %11 = add nsw i32 %10, 1        ; %11=n+1
    call void @putfarray(i32 %11, float* %5)

    ; return b[0]
    %12 = load float, float* %5, align 16   ; %12=b[0]
    %13 = fptosi float %12 to i32
    ret i32 %13
}   



declare dso_local i32 @getint() #1
declare dso_local void @putint(i32) #1
declare dso_local i32 @getfarray(float*) #1
declare dso_local void @putfarray(i32, float*) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}