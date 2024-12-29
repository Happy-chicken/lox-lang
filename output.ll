; ModuleID = 'lox'
source_filename = "lox"

@LOX_VERSION = global i32 42

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = call i32 @square()
  ret i32 0
}

define i32 @square(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %x2 = load i32, ptr %x1, align 4
  %x3 = load i32, ptr %x1, align 4
  %tmpmul = mul i32 %x2, %x3
  ret i32 %tmpmul
}
