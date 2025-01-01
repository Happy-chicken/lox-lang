; ModuleID = 'lox'
source_filename = "lox"

%base = type {}

@LOX_VERSION = global i32 42
@base = external global %base

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  ret i32 0
}
