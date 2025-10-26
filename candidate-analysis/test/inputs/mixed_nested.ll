; ModuleID = 'test/inputs/mixed_nested.c'
source_filename = "test/inputs/mixed_nested.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Point = type { i32, i32 }
%struct.Entry = type { i32, i32, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mixed_process(i64 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i32, align 4
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i32, align 4
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  store i64 %0, ptr %3, align 8
  %13 = load i64, ptr %3, align 8
  %14 = mul i64 8, %13
  %15 = call noalias ptr @malloc(i64 noundef %14) #3
  store ptr %15, ptr %4, align 8
  %16 = load i64, ptr %3, align 8
  %17 = mul i64 12, %16
  %18 = call noalias ptr @malloc(i64 noundef %17) #3
  store ptr %18, ptr %5, align 8
  %19 = load ptr, ptr %4, align 8
  %20 = icmp ne ptr %19, null
  br i1 %20, label %21, label %24

21:                                               ; preds = %1
  %22 = load ptr, ptr %5, align 8
  %23 = icmp ne ptr %22, null
  br i1 %23, label %27, label %24

24:                                               ; preds = %21, %1
  %25 = load ptr, ptr %4, align 8
  call void @free(ptr noundef %25) #4
  %26 = load ptr, ptr %5, align 8
  call void @free(ptr noundef %26) #4
  store i32 -1, ptr %2, align 4
  br label %181

27:                                               ; preds = %21
  store i64 0, ptr %6, align 8
  br label %28

28:                                               ; preds = %70, %27
  %29 = load i64, ptr %6, align 8
  %30 = load i64, ptr %3, align 8
  %31 = icmp ult i64 %29, %30
  br i1 %31, label %32, label %73

32:                                               ; preds = %28
  %33 = load i64, ptr %6, align 8
  %34 = trunc i64 %33 to i32
  %35 = load ptr, ptr %4, align 8
  %36 = load i64, ptr %6, align 8
  %37 = getelementptr inbounds %struct.Point, ptr %35, i64 %36
  %38 = getelementptr inbounds %struct.Point, ptr %37, i32 0, i32 0
  store i32 %34, ptr %38, align 4
  %39 = load i64, ptr %3, align 8
  %40 = load i64, ptr %6, align 8
  %41 = sub i64 %39, %40
  %42 = trunc i64 %41 to i32
  %43 = load ptr, ptr %4, align 8
  %44 = load i64, ptr %6, align 8
  %45 = getelementptr inbounds %struct.Point, ptr %43, i64 %44
  %46 = getelementptr inbounds %struct.Point, ptr %45, i32 0, i32 1
  store i32 %42, ptr %46, align 4
  %47 = load i64, ptr %6, align 8
  %48 = mul i64 %47, 2
  %49 = trunc i64 %48 to i32
  %50 = load ptr, ptr %5, align 8
  %51 = load i64, ptr %6, align 8
  %52 = getelementptr inbounds %struct.Entry, ptr %50, i64 %51
  %53 = getelementptr inbounds %struct.Entry, ptr %52, i32 0, i32 0
  store i32 %49, ptr %53, align 4
  %54 = load i64, ptr %3, align 8
  %55 = load i64, ptr %6, align 8
  %56 = mul i64 %55, 2
  %57 = sub i64 %54, %56
  %58 = trunc i64 %57 to i32
  %59 = load ptr, ptr %5, align 8
  %60 = load i64, ptr %6, align 8
  %61 = getelementptr inbounds %struct.Entry, ptr %59, i64 %60
  %62 = getelementptr inbounds %struct.Entry, ptr %61, i32 0, i32 1
  store i32 %58, ptr %62, align 4
  %63 = load i64, ptr %6, align 8
  %64 = and i64 %63, 1
  %65 = trunc i64 %64 to i32
  %66 = load ptr, ptr %5, align 8
  %67 = load i64, ptr %6, align 8
  %68 = getelementptr inbounds %struct.Entry, ptr %66, i64 %67
  %69 = getelementptr inbounds %struct.Entry, ptr %68, i32 0, i32 2
  store i32 %65, ptr %69, align 4
  br label %70

70:                                               ; preds = %32
  %71 = load i64, ptr %6, align 8
  %72 = add i64 %71, 1
  store i64 %72, ptr %6, align 8
  br label %28, !llvm.loop !6

73:                                               ; preds = %28
  store i32 0, ptr %7, align 4
  store i64 0, ptr %8, align 8
  br label %74

74:                                               ; preds = %97, %73
  %75 = load i64, ptr %8, align 8
  %76 = load i64, ptr %3, align 8
  %77 = icmp ult i64 %75, %76
  br i1 %77, label %78, label %100

78:                                               ; preds = %74
  %79 = load ptr, ptr %4, align 8
  %80 = load i64, ptr %8, align 8
  %81 = getelementptr inbounds %struct.Point, ptr %79, i64 %80
  %82 = getelementptr inbounds %struct.Point, ptr %81, i32 0, i32 1
  %83 = load i32, ptr %82, align 4
  %84 = load ptr, ptr %4, align 8
  %85 = load i64, ptr %8, align 8
  %86 = getelementptr inbounds %struct.Point, ptr %84, i64 %85
  %87 = getelementptr inbounds %struct.Point, ptr %86, i32 0, i32 0
  %88 = load i32, ptr %87, align 4
  %89 = add nsw i32 %88, %83
  store i32 %89, ptr %87, align 4
  %90 = load ptr, ptr %4, align 8
  %91 = load i64, ptr %8, align 8
  %92 = getelementptr inbounds %struct.Point, ptr %90, i64 %91
  %93 = getelementptr inbounds %struct.Point, ptr %92, i32 0, i32 0
  %94 = load i32, ptr %93, align 4
  %95 = load i32, ptr %7, align 4
  %96 = add nsw i32 %95, %94
  store i32 %96, ptr %7, align 4
  br label %97

97:                                               ; preds = %78
  %98 = load i64, ptr %8, align 8
  %99 = add i64 %98, 1
  store i64 %99, ptr %8, align 8
  br label %74, !llvm.loop !8

100:                                              ; preds = %74
  store i64 0, ptr %9, align 8
  br label %101

101:                                              ; preds = %156, %100
  %102 = load i64, ptr %9, align 8
  %103 = load i64, ptr %3, align 8
  %104 = icmp ult i64 %102, %103
  br i1 %104, label %105, label %159

105:                                              ; preds = %101
  store i32 0, ptr %10, align 4
  %106 = load i64, ptr %9, align 8
  %107 = add i64 %106, 1
  store i64 %107, ptr %11, align 8
  br label %108

108:                                              ; preds = %149, %105
  %109 = load i64, ptr %11, align 8
  %110 = load i64, ptr %3, align 8
  %111 = icmp ult i64 %109, %110
  br i1 %111, label %112, label %152

112:                                              ; preds = %108
  %113 = load ptr, ptr %5, align 8
  %114 = load i64, ptr %11, align 8
  %115 = getelementptr inbounds %struct.Entry, ptr %113, i64 %114
  %116 = getelementptr inbounds %struct.Entry, ptr %115, i32 0, i32 2
  %117 = load i32, ptr %116, align 4
  %118 = and i32 %117, 1
  %119 = icmp eq i32 %118, 0
  br i1 %119, label %120, label %134

120:                                              ; preds = %112
  %121 = load ptr, ptr %5, align 8
  %122 = load i64, ptr %9, align 8
  %123 = getelementptr inbounds %struct.Entry, ptr %121, i64 %122
  %124 = getelementptr inbounds %struct.Entry, ptr %123, i32 0, i32 0
  %125 = load i32, ptr %124, align 4
  %126 = load ptr, ptr %5, align 8
  %127 = load i64, ptr %11, align 8
  %128 = getelementptr inbounds %struct.Entry, ptr %126, i64 %127
  %129 = getelementptr inbounds %struct.Entry, ptr %128, i32 0, i32 1
  %130 = load i32, ptr %129, align 4
  %131 = mul nsw i32 %125, %130
  %132 = load i32, ptr %10, align 4
  %133 = add nsw i32 %132, %131
  store i32 %133, ptr %10, align 4
  br label %148

134:                                              ; preds = %112
  %135 = load ptr, ptr %5, align 8
  %136 = load i64, ptr %9, align 8
  %137 = getelementptr inbounds %struct.Entry, ptr %135, i64 %136
  %138 = getelementptr inbounds %struct.Entry, ptr %137, i32 0, i32 1
  %139 = load i32, ptr %138, align 4
  %140 = load ptr, ptr %5, align 8
  %141 = load i64, ptr %11, align 8
  %142 = getelementptr inbounds %struct.Entry, ptr %140, i64 %141
  %143 = getelementptr inbounds %struct.Entry, ptr %142, i32 0, i32 0
  %144 = load i32, ptr %143, align 4
  %145 = mul nsw i32 %139, %144
  %146 = load i32, ptr %10, align 4
  %147 = add nsw i32 %146, %145
  store i32 %147, ptr %10, align 4
  br label %148

148:                                              ; preds = %134, %120
  br label %149

149:                                              ; preds = %148
  %150 = load i64, ptr %11, align 8
  %151 = add i64 %150, 1
  store i64 %151, ptr %11, align 8
  br label %108, !llvm.loop !9

152:                                              ; preds = %108
  %153 = load i32, ptr %10, align 4
  %154 = load i32, ptr %7, align 4
  %155 = add nsw i32 %154, %153
  store i32 %155, ptr %7, align 4
  br label %156

156:                                              ; preds = %152
  %157 = load i64, ptr %9, align 8
  %158 = add i64 %157, 1
  store i64 %158, ptr %9, align 8
  br label %101, !llvm.loop !10

159:                                              ; preds = %101
  store i64 0, ptr %12, align 8
  br label %160

160:                                              ; preds = %174, %159
  %161 = load i64, ptr %12, align 8
  %162 = load i64, ptr %3, align 8
  %163 = icmp ult i64 %161, %162
  br i1 %163, label %164, label %177

164:                                              ; preds = %160
  %165 = load ptr, ptr %5, align 8
  %166 = load i64, ptr %12, align 8
  %167 = getelementptr inbounds %struct.Entry, ptr %165, i64 %166
  %168 = getelementptr inbounds %struct.Entry, ptr %167, i32 0, i32 2
  %169 = load i32, ptr %168, align 4
  %170 = load ptr, ptr %4, align 8
  %171 = load i64, ptr %12, align 8
  %172 = getelementptr inbounds %struct.Point, ptr %170, i64 %171
  %173 = getelementptr inbounds %struct.Point, ptr %172, i32 0, i32 1
  store i32 %169, ptr %173, align 4
  br label %174

174:                                              ; preds = %164
  %175 = load i64, ptr %12, align 8
  %176 = add i64 %175, 1
  store i64 %176, ptr %12, align 8
  br label %160, !llvm.loop !11

177:                                              ; preds = %160
  %178 = load ptr, ptr %4, align 8
  call void @free(ptr noundef %178) #4
  %179 = load ptr, ptr %5, align 8
  call void @free(ptr noundef %179) #4
  %180 = load i32, ptr %7, align 4
  store i32 %180, ptr %2, align 4
  br label %181

181:                                              ; preds = %177, %24
  %182 = load i32, ptr %2, align 4
  ret i32 %182
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
!11 = distinct !{!11, !7}
