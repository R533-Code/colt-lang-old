## Pointers and References:
The language does not have a concept of references.
Instead, annotations on parameters and pointers are used.

```
// Takes in an uninitialized variable.
// Ensures the variable is initialized before
// the function returns.
fn init_i64(a: out i64): a = 10;

// Takes an initialized variable.
// The variable can be read from.
fn print_i64(a: in i64): std::println("{}", a);

// Takes in an initialized mutable variable.
// The variable can be written to.
fn modify_i64(a: inout i64): a = 11;
```

There are two pointer types. Pointers to mutable memory `mut_ptr`, and pointers to non-mutable memory `ptr`.

```
var mut a = 10;
var     b = 10;

var     c = &a; // typeof(c) -> mut_ptr<i64>
var     d = &b; // typeof(c) -> ptr<i64>
```

## Type System:
Types are not qualified: `mut i64` does not exist. Memory is either readable (`mut` or not) or readable-writable (`mut`).

```
var mut a = 10; // typeof(a) -> i64
var     b = 10; // typeof(b) -> i64

a = 11; // as 'a' is mutable, no error.
b = 11; // ERROR: 'b' not mutable
```

## Storage and Lifetimes:
All objects need storage. Storage results from memory allocations.
There are three storage types:
- local  (stack,  managed by the compiler)
- static (static, managed by the compiler)
- global (heap,   managed by the programmer)

```
// 'a' can provide storage to an i16.
var a: opaque_mut_ptr = malloc(16_B);

// 'b' provide storage to an i16
var mut b: i16;
```

A call to a constructor is needed to start the lifetime of an object.
A function is considered a constructor if it overwrites the given storage.

```
var mut a: i16;
// Constructor, a's lifetime starts
a = 10;
```

A call to a destructor is needed to end the lifetime of an object. Only global storage must have an explicit call to its destructor.

```
type unique_ptr
{
    ~unique_ptr()
}
```