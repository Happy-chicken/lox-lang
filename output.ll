; ModuleID = 'lox'
source_filename = "lox"

%base = type { i32 }

@LOX_VERSION = global i32 42
@base = external global %base

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  ret i32 0
}

declare i32 @base_printer(i32)

define i32 @printer(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %x2 = load i32, ptr %x1, align 4
  ret i32 %x2
}
