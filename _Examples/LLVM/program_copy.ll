; ModuleID = 'Main Module'
source_filename = "Main Module"

declare i32 @printf(i8*, ...)
declare i32 @scanf(i8*, ...)

@.str = private unnamed_addr constant [3 x i8] c"%i\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"Lol: %i\00", align 1

define i32 @main()  {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = alloca i32, align 4
  store i32 3, i32* %2, align 4
  ;<ASD>:
  %3 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i32* %2)
  %4 = load i32, i32* %2, align 4
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %4)
  ret i32 0
}


