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

define i32 @base_printer(ptr %self, i32 %x) {
entry:
  %self1 = alloca ptr, align 8
  store ptr %self, ptr %self1, align 8
  %x2 = alloca i32, align 4
  store i32 %x, ptr %x2, align 4
  %x3 = load i32, ptr %x2, align 4
  ret i32 %x3
}
