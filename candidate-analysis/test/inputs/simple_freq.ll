; ModuleID = 'simple_freq.c'
source_filename = "simple_freq.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Entry = type { i32, i32, i32 }

@hot_flag = internal global i32 1, align 4
@warm_flag = internal global i32 1, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i32, align 4
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  store i32 0, ptr %1, align 4
  store i64 8, ptr %2, align 8
  store i64 1000, ptr %3, align 8
  store i64 64, ptr %4, align 8
  %12 = call noalias ptr @malloc(i64 noundef 96) #3
  store ptr %12, ptr %5, align 8
  %13 = load ptr, ptr %5, align 8
  %14 = icmp ne ptr %13, null
  br i1 %14, label %16, label %15

15:                                               ; preds = %0
  store i32 -1, ptr %1, align 4
  br label %91

16:                                               ; preds = %0
  store i64 0, ptr %6, align 8
  br label %17

17:                                               ; preds = %41, %16
  %18 = load i64, ptr %6, align 8
  %19 = icmp ult i64 %18, 8
  br i1 %19, label %20, label %44

20:                                               ; preds = %17
  %21 = load i64, ptr %6, align 8
  %22 = trunc i64 %21 to i32
  %23 = load ptr, ptr %5, align 8
  %24 = load i64, ptr %6, align 8
  %25 = getelementptr inbounds %struct.Entry, ptr %23, i64 %24
  %26 = getelementptr inbounds %struct.Entry, ptr %25, i32 0, i32 0
  store i32 %22, ptr %26, align 4
  %27 = load i64, ptr %6, align 8
  %28 = mul i64 %27, 2
  %29 = trunc i64 %28 to i32
  %30 = load ptr, ptr %5, align 8
  %31 = load i64, ptr %6, align 8
  %32 = getelementptr inbounds %struct.Entry, ptr %30, i64 %31
  %33 = getelementptr inbounds %struct.Entry, ptr %32, i32 0, i32 1
  store i32 %29, ptr %33, align 4
  %34 = load i64, ptr %6, align 8
  %35 = mul i64 %34, 3
  %36 = trunc i64 %35 to i32
  %37 = load ptr, ptr %5, align 8
  %38 = load i64, ptr %6, align 8
  %39 = getelementptr inbounds %struct.Entry, ptr %37, i64 %38
  %40 = getelementptr inbounds %struct.Entry, ptr %39, i32 0, i32 2
  store i32 %36, ptr %40, align 4
  br label %41

41:                                               ; preds = %20
  %42 = load i64, ptr %6, align 8
  %43 = add i64 %42, 1
  store i64 %43, ptr %6, align 8
  br label %17, !llvm.loop !6

44:                                               ; preds = %17
  store i32 0, ptr %7, align 4
  %45 = load volatile i32, ptr @hot_flag, align 4
  %46 = sext i32 %45 to i64
  %47 = icmp ne i64 %46, 0
  br i1 %47, label %48, label %66

48:                                               ; preds = %44
  store i64 0, ptr %8, align 8
  br label %49

49:                                               ; preds = %62, %48
  %50 = load i64, ptr %8, align 8
  %51 = icmp ult i64 %50, 1000
  br i1 %51, label %52, label %65

52:                                               ; preds = %49
  %53 = load i64, ptr %8, align 8
  %54 = urem i64 %53, 8
  store i64 %54, ptr %9, align 8
  %55 = load ptr, ptr %5, align 8
  %56 = load i64, ptr %9, align 8
  %57 = getelementptr inbounds %struct.Entry, ptr %55, i64 %56
  %58 = getelementptr inbounds %struct.Entry, ptr %57, i32 0, i32 0
  %59 = load i32, ptr %58, align 4
  %60 = load i32, ptr %7, align 4
  %61 = add nsw i32 %60, %59
  store i32 %61, ptr %7, align 4
  br label %62

62:                                               ; preds = %52
  %63 = load i64, ptr %8, align 8
  %64 = add i64 %63, 1
  store i64 %64, ptr %8, align 8
  br label %49, !llvm.loop !8

65:                                               ; preds = %49
  br label %66

66:                                               ; preds = %65, %44
  %67 = load volatile i32, ptr @warm_flag, align 4
  %68 = sext i32 %67 to i64
  %69 = icmp ne i64 %68, 0
  br i1 %69, label %70, label %88

70:                                               ; preds = %66
  store i64 0, ptr %10, align 8
  br label %71

71:                                               ; preds = %84, %70
  %72 = load i64, ptr %10, align 8
  %73 = icmp ult i64 %72, 64
  br i1 %73, label %74, label %87

74:                                               ; preds = %71
  %75 = load i64, ptr %10, align 8
  %76 = urem i64 %75, 8
  store i64 %76, ptr %11, align 8
  %77 = load ptr, ptr %5, align 8
  %78 = load i64, ptr %11, align 8
  %79 = getelementptr inbounds %struct.Entry, ptr %77, i64 %78
  %80 = getelementptr inbounds %struct.Entry, ptr %79, i32 0, i32 1
  %81 = load i32, ptr %80, align 4
  %82 = load i32, ptr %7, align 4
  %83 = add nsw i32 %82, %81
  store i32 %83, ptr %7, align 4
  br label %84

84:                                               ; preds = %74
  %85 = load i64, ptr %10, align 8
  %86 = add i64 %85, 1
  store i64 %86, ptr %10, align 8
  br label %71, !llvm.loop !9

87:                                               ; preds = %71
  br label %88

88:                                               ; preds = %87, %66
  %89 = load ptr, ptr %5, align 8
  call void @free(ptr noundef %89) #4
  %90 = load i32, ptr %7, align 4
  store i32 %90, ptr %1, align 4
  br label %91

91:                                               ; preds = %88, %15
  %92 = load i32, ptr %1, align 4
  ret i32 %92
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
