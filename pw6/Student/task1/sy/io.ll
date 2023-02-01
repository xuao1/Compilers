; ModuleID = 'io.c'
source_filename = "io.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca [10000 x float], align 16
  %4 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %5 = call i32 @getint()
  store i32 %5, i32* %2, align 4
  %6 = load i32, i32* %2, align 4
  call void @putint(i32 %6)
  %7 = bitcast [10000 x float]* %3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %7, i8 0, i64 40000, i1 false)
  %8 = bitcast i8* %7 to <{ float, float, [9998 x float] }>*
  %9 = getelementptr inbounds <{ float, float, [9998 x float] }>, <{ float, float, [9998 x float] }>* %8, i32 0, i32 0
  store float 1.000000e+00, float* %9, align 16
  %10 = getelementptr inbounds <{ float, float, [9998 x float] }>, <{ float, float, [9998 x float] }>* %8, i32 0, i32 1
  store float 2.000000e+00, float* %10, align 4
  %11 = getelementptr inbounds [10000 x float], [10000 x float]* %3, i64 0, i64 0
  %12 = call i32 @getfarray(float* %11)
  store i32 %12, i32* %4, align 4
  %13 = load i32, i32* %4, align 4
  %14 = add nsw i32 %13, 1
  %15 = getelementptr inbounds [10000 x float], [10000 x float]* %3, i64 0, i64 0
  call void @putfarray(i32 %14, float* %15)
  %16 = getelementptr inbounds [10000 x float], [10000 x float]* %3, i64 0, i64 0
  %17 = load float, float* %16, align 16
  %18 = fptosi float %17 to i32
  ret i32 %18
}

declare dso_local i32 @getint() #1

declare dso_local void @putint(i32) #1

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2

declare dso_local i32 @getfarray(float*) #1

declare dso_local void @putfarray(i32, float*) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}
