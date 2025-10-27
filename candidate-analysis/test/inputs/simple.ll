; ModuleID = 'simple.c'
source_filename = "simple.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Entry = type { i32, i32, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  %4 = alloca i32, align 4
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  store i32 0, ptr %1, align 4
  %9 = call noalias ptr @malloc(i64 noundef 120) #3
  store ptr %9, ptr %2, align 8
  %10 = load ptr, ptr %2, align 8
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %0
  store i32 -1, ptr %1, align 4
  br label %83

13:                                               ; preds = %0
  store i64 0, ptr %3, align 8
  br label %14

14:                                               ; preds = %36, %13
  %15 = load i64, ptr %3, align 8
  %16 = icmp ult i64 %15, 10
  br i1 %16, label %17, label %39

17:                                               ; preds = %14
  %18 = load i64, ptr %3, align 8
  %19 = trunc i64 %18 to i32
  %20 = load ptr, ptr %2, align 8
  %21 = load i64, ptr %3, align 8
  %22 = getelementptr inbounds %struct.Entry, ptr %20, i64 %21
  %23 = getelementptr inbounds %struct.Entry, ptr %22, i32 0, i32 0
  store i32 %19, ptr %23, align 4
  %24 = load i64, ptr %3, align 8
  %25 = trunc i64 %24 to i32
  %26 = load ptr, ptr %2, align 8
  %27 = load i64, ptr %3, align 8
  %28 = getelementptr inbounds %struct.Entry, ptr %26, i64 %27
  %29 = getelementptr inbounds %struct.Entry, ptr %28, i32 0, i32 1
  store i32 %25, ptr %29, align 4
  %30 = load i64, ptr %3, align 8
  %31 = trunc i64 %30 to i32
  %32 = load ptr, ptr %2, align 8
  %33 = load i64, ptr %3, align 8
  %34 = getelementptr inbounds %struct.Entry, ptr %32, i64 %33
  %35 = getelementptr inbounds %struct.Entry, ptr %34, i32 0, i32 2
  store i32 %31, ptr %35, align 4
  br label %36

36:                                               ; preds = %17
  %37 = load i64, ptr %3, align 8
  %38 = add i64 %37, 1
  store i64 %38, ptr %3, align 8
  br label %14, !llvm.loop !6

39:                                               ; preds = %14
  store i32 0, ptr %4, align 4
  store i64 0, ptr %5, align 8
  br label %40

40:                                               ; preds = %53, %39
  %41 = load i64, ptr %5, align 8
  %42 = icmp ult i64 %41, 10000
  br i1 %42, label %43, label %56

43:                                               ; preds = %40
  %44 = load i64, ptr %5, align 8
  %45 = urem i64 %44, 10
  store i64 %45, ptr %6, align 8
  %46 = load ptr, ptr %2, align 8
  %47 = load i64, ptr %6, align 8
  %48 = getelementptr inbounds %struct.Entry, ptr %46, i64 %47
  %49 = getelementptr inbounds %struct.Entry, ptr %48, i32 0, i32 0
  %50 = load i32, ptr %49, align 4
  %51 = load i32, ptr %4, align 4
  %52 = add nsw i32 %51, %50
  store i32 %52, ptr %4, align 4
  br label %53

53:                                               ; preds = %43
  %54 = load i64, ptr %5, align 8
  %55 = add i64 %54, 1
  store i64 %55, ptr %5, align 8
  br label %40, !llvm.loop !8

56:                                               ; preds = %40
  store i64 0, ptr %7, align 8
  br label %57

57:                                               ; preds = %77, %56
  %58 = load i64, ptr %7, align 8
  %59 = icmp ult i64 %58, 10
  br i1 %59, label %60, label %80

60:                                               ; preds = %57
  %61 = load i64, ptr %7, align 8
  %62 = urem i64 %61, 10
  store i64 %62, ptr %8, align 8
  %63 = load ptr, ptr %2, align 8
  %64 = load i64, ptr %8, align 8
  %65 = getelementptr inbounds %struct.Entry, ptr %63, i64 %64
  %66 = getelementptr inbounds %struct.Entry, ptr %65, i32 0, i32 0
  %67 = load i32, ptr %66, align 4
  %68 = load i32, ptr %4, align 4
  %69 = add nsw i32 %68, %67
  store i32 %69, ptr %4, align 4
  %70 = load ptr, ptr %2, align 8
  %71 = load i64, ptr %8, align 8
  %72 = getelementptr inbounds %struct.Entry, ptr %70, i64 %71
  %73 = getelementptr inbounds %struct.Entry, ptr %72, i32 0, i32 1
  %74 = load i32, ptr %73, align 4
  %75 = load i32, ptr %4, align 4
  %76 = add nsw i32 %75, %74
  store i32 %76, ptr %4, align 4
  br label %77

77:                                               ; preds = %60
  %78 = load i64, ptr %7, align 8
  %79 = add i64 %78, 1
  store i64 %79, ptr %7, align 8
  br label %57, !llvm.loop !9

80:                                               ; preds = %57
  %81 = load ptr, ptr %2, align 8
  call void @free(ptr noundef %81) #4
  %82 = load i32, ptr %4, align 4
  store i32 %82, ptr %1, align 4
  br label %83

83:                                               ; preds = %80, %12
  %84 = load i32, ptr %1, align 4
  ret i32 %84
}

; Function Attrs: nounwind allocsize(0)
declare noalias ptr @malloc(i64 noundef) #1

; Function Attrs: nounwind
declare void @free(ptr noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind allocsize(0) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.1.7 (++20250804090312+cd708029e0b2-1~exp1~20250804210325.79)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
