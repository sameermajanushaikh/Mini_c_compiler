
# Mini C Compiler

## Overview
This is a simple Mini C Compiler implemented in C.
It supports:
- Assignments, arithmetic (+,-,*,/)
- If, if-else, while
- Blocks {...}

## Compile & Run (Windows)
1. Open CMD or Terminal
2. Compile:
   gcc mini_c_compiler.c -o mini_c_compiler.exe
3. Run:
   mini_c_compiler.exe
4. Input code (single line, Ctrl+Z to end input):
   a = 5;
   b = a + 3;
   if (b > 5) { c = b * 2; }
