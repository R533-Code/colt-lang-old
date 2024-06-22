set disassembly-flavor intel
set print asm-demangle on
file build/colt
add-symbol-file build/colt
set pagination off
break main
layout split