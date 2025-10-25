; ModuleID = './test/inputs/simple_nested.c'
source_filename = "./test/inputs/simple_nested.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Entry = type { i32, i32, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @process_structs(i64 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i32, align 4
  %10 = alloca i64, align 8
  store i64 %0, ptr %3, align 8
  %11 = load i64, ptr %3, align 8
  %12 = mul i64 12, %11
  %13 = call noalias ptr @malloc(i64 noundef %12) #3
  store ptr %13, ptr %4, align 8
  %14 = load ptr, ptr %4, align 8
  %15 = icmp ne ptr %14, null
  br i1 %15, label %17, label %16

16:                                               ; preds = %1
  store i32 -1, ptr %2, align 4
  br label %138

17:                                               ; preds = %1
  store i64 0, ptr %5, align 8
  br label %18

18:                                               ; preds = %44, %17
  %19 = load i64, ptr %5, align 8
  %20 = load i64, ptr %3, align 8
  %21 = icmp ult i64 %19, %20
  br i1 %21, label %22, label %47

22:                                               ; preds = %18
  %23 = load i64, ptr %5, align 8
  %24 = trunc i64 %23 to i32
  %25 = load ptr, ptr %4, align 8
  %26 = load i64, ptr %5, align 8
  %27 = getelementptr inbounds %struct.Entry, ptr %25, i64 %26
  %28 = getelementptr inbounds %struct.Entry, ptr %27, i32 0, i32 0
  store i32 %24, ptr %28, align 4
  %29 = load i64, ptr %3, align 8
  %30 = load i64, ptr %5, align 8
  %31 = sub i64 %29, %30
  %32 = trunc i64 %31 to i32
  %33 = load ptr, ptr %4, align 8
  %34 = load i64, ptr %5, align 8
  %35 = getelementptr inbounds %struct.Entry, ptr %33, i64 %34
  %36 = getelementptr inbounds %struct.Entry, ptr %35, i32 0, i32 1
  store i32 %32, ptr %36, align 4
  %37 = load i64, ptr %5, align 8
  %38 = and i64 %37, 1
  %39 = trunc i64 %38 to i32
  %40 = load ptr, ptr %4, align 8
  %41 = load i64, ptr %5, align 8
  %42 = getelementptr inbounds %struct.Entry, ptr %40, i64 %41
  %43 = getelementptr inbounds %struct.Entry, ptr %42, i32 0, i32 2
  store i32 %39, ptr %43, align 4
  br label %44

44:                                               ; preds = %22
  %45 = load i64, ptr %5, align 8
  %46 = add i64 %45, 1
  store i64 %46, ptr %5, align 8
  br label %18, !llvm.loop !6

47:                                               ; preds = %18
  store i32 0, ptr %6, align 4
  store i64 0, ptr %7, align 8
  br label %48

48:                                               ; preds = %73, %47
  %49 = load i64, ptr %7, align 8
  %50 = load i64, ptr %3, align 8
  %51 = icmp ult i64 %49, %50
  br i1 %51, label %52, label %76

52:                                               ; preds = %48
  %53 = load i64, ptr %7, align 8
  %54 = and i64 %53, 1
  %55 = icmp eq i64 %54, 0
  br i1 %55, label %56, label %64

56:                                               ; preds = %52
  %57 = load ptr, ptr %4, align 8
  %58 = load i64, ptr %7, align 8
  %59 = getelementptr inbounds %struct.Entry, ptr %57, i64 %58
  %60 = getelementptr inbounds %struct.Entry, ptr %59, i32 0, i32 0
  %61 = load i32, ptr %60, align 4
  %62 = load i32, ptr %6, align 4
  %63 = add nsw i32 %62, %61
  store i32 %63, ptr %6, align 4
  br label %72

64:                                               ; preds = %52
  %65 = load ptr, ptr %4, align 8
  %66 = load i64, ptr %7, align 8
  %67 = getelementptr inbounds %struct.Entry, ptr %65, i64 %66
  %68 = getelementptr inbounds %struct.Entry, ptr %67, i32 0, i32 1
  %69 = load i32, ptr %68, align 4
  %70 = load i32, ptr %6, align 4
  %71 = add nsw i32 %70, %69
  store i32 %71, ptr %6, align 4
  br label %72

72:                                               ; preds = %64, %56
  br label %73

73:                                               ; preds = %72
  %74 = load i64, ptr %7, align 8
  %75 = add i64 %74, 1
  store i64 %75, ptr %7, align 8
  br label %48, !llvm.loop !8

76:                                               ; preds = %48
  store i64 0, ptr %8, align 8
  br label %77

77:                                               ; preds = %132, %76
  %78 = load i64, ptr %8, align 8
  %79 = load i64, ptr %3, align 8
  %80 = icmp ult i64 %78, %79
  br i1 %80, label %81, label %135

81:                                               ; preds = %77
  store i32 0, ptr %9, align 4
  %82 = load i64, ptr %8, align 8
  %83 = add i64 %82, 1
  store i64 %83, ptr %10, align 8
  br label %84

84:                                               ; preds = %125, %81
  %85 = load i64, ptr %10, align 8
  %86 = load i64, ptr %3, align 8
  %87 = icmp ult i64 %85, %86
  br i1 %87, label %88, label %128

88:                                               ; preds = %84
  %89 = load ptr, ptr %4, align 8
  %90 = load i64, ptr %10, align 8
  %91 = getelementptr inbounds %struct.Entry, ptr %89, i64 %90
  %92 = getelementptr inbounds %struct.Entry, ptr %91, i32 0, i32 2
  %93 = load i32, ptr %92, align 4
  %94 = and i32 %93, 1
  %95 = icmp eq i32 %94, 0
  br i1 %95, label %96, label %110

96:                                               ; preds = %88
  %97 = load ptr, ptr %4, align 8
  %98 = load i64, ptr %8, align 8
  %99 = getelementptr inbounds %struct.Entry, ptr %97, i64 %98
  %100 = getelementptr inbounds %struct.Entry, ptr %99, i32 0, i32 0
  %101 = load i32, ptr %100, align 4
  %102 = load ptr, ptr %4, align 8
  %103 = load i64, ptr %10, align 8
  %104 = getelementptr inbounds %struct.Entry, ptr %102, i64 %103
  %105 = getelementptr inbounds %struct.Entry, ptr %104, i32 0, i32 1
  %106 = load i32, ptr %105, align 4
  %107 = mul nsw i32 %101, %106
  %108 = load i32, ptr %9, align 4
  %109 = add nsw i32 %108, %107
  store i32 %109, ptr %9, align 4
  br label %124

110:                                              ; preds = %88
  %111 = load ptr, ptr %4, align 8
  %112 = load i64, ptr %8, align 8
  %113 = getelementptr inbounds %struct.Entry, ptr %111, i64 %112
  %114 = getelementptr inbounds %struct.Entry, ptr %113, i32 0, i32 1
  %115 = load i32, ptr %114, align 4
  %116 = load ptr, ptr %4, align 8
  %117 = load i64, ptr %10, align 8
  %118 = getelementptr inbounds %struct.Entry, ptr %116, i64 %117
  %119 = getelementptr inbounds %struct.Entry, ptr %118, i32 0, i32 0
  %120 = load i32, ptr %119, align 4
  %121 = mul nsw i32 %115, %120
  %122 = load i32, ptr %9, align 4
  %123 = add nsw i32 %122, %121
  store i32 %123, ptr %9, align 4
  br label %124

124:                                              ; preds = %110, %96
  br label %125

125:                                              ; preds = %124
  %126 = load i64, ptr %10, align 8
  %127 = add i64 %126, 1
  store i64 %127, ptr %10, align 8
  br label %84, !llvm.loop !9

128:                                              ; preds = %84
  %129 = load i32, ptr %9, align 4
  %130 = load i32, ptr %6, align 4
  %131 = add nsw i32 %130, %129
  store i32 %131, ptr %6, align 4
  br label %132

132:                                              ; preds = %128
  %133 = load i64, ptr %8, align 8
  %134 = add i64 %133, 1
  store i64 %134, ptr %8, align 8
  br label %77, !llvm.loop !10

135:                                              ; preds = %77
  %136 = load ptr, ptr %4, align 8
  call void @free(ptr noundef %136) #4
  %137 = load i32, ptr %6, align 4
  store i32 %137, ptr %2, align 4
  br label %138

138:                                              ; preds = %135, %16
  %139 = load i32, ptr %2, align 4
  ret i32 %139
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
!10 = distinct !{!10, !7}
