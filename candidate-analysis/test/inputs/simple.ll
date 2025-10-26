; ModuleID = 'test/inputs/simple.c'
source_filename = "test/inputs/simple.c"
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
  store i32 0, ptr %1, align 4
  %7 = call noalias ptr @malloc(i64 noundef 120) #3
  store ptr %7, ptr %2, align 8
  %8 = load ptr, ptr %2, align 8
  %9 = icmp ne ptr %8, null
  br i1 %9, label %11, label %10

10:                                               ; preds = %0
  store i32 -1, ptr %1, align 4
  br label %77

11:                                               ; preds = %0
  store i64 0, ptr %3, align 8
  br label %12

12:                                               ; preds = %34, %11
  %13 = load i64, ptr %3, align 8
  %14 = icmp ult i64 %13, 10
  br i1 %14, label %15, label %37

15:                                               ; preds = %12
  %16 = load i64, ptr %3, align 8
  %17 = trunc i64 %16 to i32
  %18 = load ptr, ptr %2, align 8
  %19 = load i64, ptr %3, align 8
  %20 = getelementptr inbounds %struct.Entry, ptr %18, i64 %19
  %21 = getelementptr inbounds %struct.Entry, ptr %20, i32 0, i32 0
  store i32 %17, ptr %21, align 4
  %22 = load i64, ptr %3, align 8
  %23 = trunc i64 %22 to i32
  %24 = load ptr, ptr %2, align 8
  %25 = load i64, ptr %3, align 8
  %26 = getelementptr inbounds %struct.Entry, ptr %24, i64 %25
  %27 = getelementptr inbounds %struct.Entry, ptr %26, i32 0, i32 1
  store i32 %23, ptr %27, align 4
  %28 = load i64, ptr %3, align 8
  %29 = trunc i64 %28 to i32
  %30 = load ptr, ptr %2, align 8
  %31 = load i64, ptr %3, align 8
  %32 = getelementptr inbounds %struct.Entry, ptr %30, i64 %31
  %33 = getelementptr inbounds %struct.Entry, ptr %32, i32 0, i32 2
  store i32 %29, ptr %33, align 4
  br label %34

34:                                               ; preds = %15
  %35 = load i64, ptr %3, align 8
  %36 = add i64 %35, 1
  store i64 %36, ptr %3, align 8
  br label %12, !llvm.loop !6

37:                                               ; preds = %12
  store i32 0, ptr %4, align 4
  store i64 0, ptr %5, align 8
  br label %38

38:                                               ; preds = %49, %37
  %39 = load i64, ptr %5, align 8
  %40 = icmp ult i64 %39, 10
  br i1 %40, label %41, label %52

41:                                               ; preds = %38
  %42 = load ptr, ptr %2, align 8
  %43 = load i64, ptr %5, align 8
  %44 = getelementptr inbounds %struct.Entry, ptr %42, i64 %43
  %45 = getelementptr inbounds %struct.Entry, ptr %44, i32 0, i32 0
  %46 = load i32, ptr %45, align 4
  %47 = load i32, ptr %4, align 4
  %48 = add nsw i32 %47, %46
  store i32 %48, ptr %4, align 4
  br label %49

49:                                               ; preds = %41
  %50 = load i64, ptr %5, align 8
  %51 = add i64 %50, 1
  store i64 %51, ptr %5, align 8
  br label %38, !llvm.loop !8

52:                                               ; preds = %38
  store i64 0, ptr %6, align 8
  br label %53

53:                                               ; preds = %71, %52
  %54 = load i64, ptr %6, align 8
  %55 = icmp ult i64 %54, 10
  br i1 %55, label %56, label %74

56:                                               ; preds = %53
  %57 = load ptr, ptr %2, align 8
  %58 = load i64, ptr %6, align 8
  %59 = getelementptr inbounds %struct.Entry, ptr %57, i64 %58
  %60 = getelementptr inbounds %struct.Entry, ptr %59, i32 0, i32 0
  %61 = load i32, ptr %60, align 4
  %62 = load i32, ptr %4, align 4
  %63 = add nsw i32 %62, %61
  store i32 %63, ptr %4, align 4
  %64 = load ptr, ptr %2, align 8
  %65 = load i64, ptr %6, align 8
  %66 = getelementptr inbounds %struct.Entry, ptr %64, i64 %65
  %67 = getelementptr inbounds %struct.Entry, ptr %66, i32 0, i32 1
  %68 = load i32, ptr %67, align 4
  %69 = load i32, ptr %4, align 4
  %70 = add nsw i32 %69, %68
  store i32 %70, ptr %4, align 4
  br label %71

71:                                               ; preds = %56
  %72 = load i64, ptr %6, align 8
  %73 = add i64 %72, 1
  store i64 %73, ptr %6, align 8
  br label %53, !llvm.loop !9

74:                                               ; preds = %53
  %75 = load ptr, ptr %2, align 8
  call void @free(ptr noundef %75) #4
  %76 = load i32, ptr %4, align 4
  store i32 %76, ptr %1, align 4
  br label %77

77:                                               ; preds = %74, %10
  %78 = load i32, ptr %1, align 4
  ret i32 %78
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
