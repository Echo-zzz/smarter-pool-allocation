; ModuleID = 'test/inputs/simple_nested.c'
source_filename = "test/inputs/simple_nested.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @process_array(ptr noundef %0, i64 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i32, align 4
  %10 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store i32 0, ptr %6, align 4
  %11 = load i64, ptr %5, align 8
  %12 = icmp eq i64 %11, 0
  br i1 %12, label %16, label %13

13:                                               ; preds = %2
  %14 = load ptr, ptr %4, align 8
  %15 = icmp eq ptr %14, null
  br i1 %15, label %16, label %17

16:                                               ; preds = %13, %2
  store i32 -1, ptr %3, align 4
  br label %73

17:                                               ; preds = %13
  store i64 0, ptr %7, align 8
  br label %18

18:                                               ; preds = %34, %17
  %19 = load i64, ptr %7, align 8
  %20 = load i64, ptr %5, align 8
  %21 = icmp ult i64 %19, %20
  br i1 %21, label %22, label %37

22:                                               ; preds = %18
  %23 = load i64, ptr %7, align 8
  %24 = and i64 %23, 1
  %25 = icmp eq i64 %24, 0
  br i1 %25, label %26, label %33

26:                                               ; preds = %22
  %27 = load ptr, ptr %4, align 8
  %28 = load i64, ptr %7, align 8
  %29 = getelementptr inbounds i32, ptr %27, i64 %28
  %30 = load i32, ptr %29, align 4
  %31 = load i32, ptr %6, align 4
  %32 = add nsw i32 %31, %30
  store i32 %32, ptr %6, align 4
  br label %33

33:                                               ; preds = %26, %22
  br label %34

34:                                               ; preds = %33
  %35 = load i64, ptr %7, align 8
  %36 = add i64 %35, 1
  store i64 %36, ptr %7, align 8
  br label %18, !llvm.loop !6

37:                                               ; preds = %18
  store i64 0, ptr %8, align 8
  br label %38

38:                                               ; preds = %68, %37
  %39 = load i64, ptr %8, align 8
  %40 = load i64, ptr %5, align 8
  %41 = icmp ult i64 %39, %40
  br i1 %41, label %42, label %71

42:                                               ; preds = %38
  store i32 0, ptr %9, align 4
  %43 = load i64, ptr %8, align 8
  %44 = add i64 %43, 1
  store i64 %44, ptr %10, align 8
  br label %45

45:                                               ; preds = %61, %42
  %46 = load i64, ptr %10, align 8
  %47 = load i64, ptr %5, align 8
  %48 = icmp ult i64 %46, %47
  br i1 %48, label %49, label %64

49:                                               ; preds = %45
  %50 = load ptr, ptr %4, align 8
  %51 = load i64, ptr %8, align 8
  %52 = getelementptr inbounds i32, ptr %50, i64 %51
  %53 = load i32, ptr %52, align 4
  %54 = load ptr, ptr %4, align 8
  %55 = load i64, ptr %10, align 8
  %56 = getelementptr inbounds i32, ptr %54, i64 %55
  %57 = load i32, ptr %56, align 4
  %58 = mul nsw i32 %53, %57
  %59 = load i32, ptr %9, align 4
  %60 = add nsw i32 %59, %58
  store i32 %60, ptr %9, align 4
  br label %61

61:                                               ; preds = %49
  %62 = load i64, ptr %10, align 8
  %63 = add i64 %62, 1
  store i64 %63, ptr %10, align 8
  br label %45, !llvm.loop !8

64:                                               ; preds = %45
  %65 = load i32, ptr %9, align 4
  %66 = load i32, ptr %6, align 4
  %67 = add nsw i32 %66, %65
  store i32 %67, ptr %6, align 4
  br label %68

68:                                               ; preds = %64
  %69 = load i64, ptr %8, align 8
  %70 = add i64 %69, 1
  store i64 %70, ptr %8, align 8
  br label %38, !llvm.loop !9

71:                                               ; preds = %38
  %72 = load i32, ptr %6, align 4
  store i32 %72, ptr %3, align 4
  br label %73

73:                                               ; preds = %71, %16
  %74 = load i32, ptr %3, align 4
  ret i32 %74
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

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
