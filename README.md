<img src="resources/icon/Colt%20Logo.svg" width=30% height=30%>

---
## Colt:
A programming language in its early stages of development.

![Clang16](https://github.com/R533-Code/colt-lang/actions/workflows/cmake_clang16.yml/badge.svg)
![g++12](https://github.com/R533-Code/colt-lang/actions/workflows/cmake_g++12.yml/badge.svg)
![MSVC](https://github.com/R533-Code/colt-lang/actions/workflows/cmake_msvc.yml/badge.svg)

## Features:
For the eleventh time, a whole codebase rewrite is in progress.
Currently, only the lexer is written and functional.
In this rewrite, the design of the compiler is following a more data-oriented
approach, as seen in [Chandler Carruth's talk](https://www.youtube.com/watch?v=ZI198eFghJk).

<!--
## Features:
The syntax is partly borrowed from Rust, but is still subject to change.
For now, the 'compiler' is more of a transpiler to `C`.

The current supported language features are:
- Functions
- Arguments specifiers (`out`, `in`, `mut`...) (still in progress)
- Auto-deducing of return type
- Local Variables (still in progress)
- While Loops and named While Loops
- Modules (equivalent of namespaces) and using declarations

The following static analysis are implemented:
- Prohibited use of (partially) initialized variables
- Unreachable code (but no support for `[[noreturn]]` functions)

The following features are the next targets:
- Move semantics and copy semantics
- `move` and `copy` argument specifiers
- Interpreter for the language

## Syntax:
```
module a
{
public:
    fn test(in i64 a, in i64 b): return a + b;
}

fn main()
{
  using module a;
  var mut a = 10;
  var b = 20;
  test(&a, &b);
  // ^ The syntax will be improved in the future
}
```

## Purpose:
The `Colt` programming language aims to be a safe, statically compiled language, with compile time programming support:
This would include compile-time reflection and support for running functions at compile time (as in `constexpr`/`consteval` C++).
-->