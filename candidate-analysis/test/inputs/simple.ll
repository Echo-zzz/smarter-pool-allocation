; ModuleID = 'simple'
source_filename = "simple.c"

target datalayout = ""
target triple = "x86_64-pc-linux-gnu"

%struct.Point = type { i32, i32, i32 }

; Function with an inner loop touching multiple struct fields
define void @loop_example(ptr %points, i32 %count) {
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %body ]
  %cmp = icmp slt i32 %i, %count
  br i1 %cmp, label %body, label %exit

body:
  %idx  = getelementptr inbounds %struct.Point, ptr %points, i32 %i
  %xptr = getelementptr inbounds %struct.Point, ptr %idx, i32 0, i32 0
  %yptr = getelementptr inbounds %struct.Point, ptr %idx, i32 0, i32 1
  %x    = load i32, ptr %xptr, align 4
  %y    = load i32, ptr %yptr, align 4
  %sum  = add i32 %x, %y
  store i32 %sum, ptr %xptr, align 4
  %i.next = add i32 %i, 1
  br label %loop

exit:
  ret void
}

; Straight-line code that references a different field outside of loops
define void @no_loop(ptr %points) {
entry:
  %p0  = getelementptr inbounds %struct.Point, ptr %points, i32 0
  %zptr = getelementptr inbounds %struct.Point, ptr %p0, i32 0, i32 2
  store i32 42, ptr %zptr, align 4
  ret void
}
