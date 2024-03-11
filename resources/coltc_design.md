# PARSING:
## Lexer:
The result of lexing is a `TokenBuffer`. A `TokenBuffer` contains the array of `Token`, that is a `Lexeme` and source code informations.
Rather than generating a single `Token` using a `getNext()` method, all the tokens are generated and stored in the `TokenBuffer`.
Each file has a matching `TokenBuffer`.

## Types:
Types are represented using a `TypeVariant`. This allows for better control and hopefully better performance.
Types are stored inside a `TypeBuffer`, which contains an array of these `TypeVariants`. As with `TokenBuffer`, a token allows to index that array. A `TypeToken` represents a type: comparing them for equality yields if a type is exactly equal to another.

## AST:
The result of parsing is an `AST`. An `AST` contains the nodes and leaves representing a parsed file.
For performance, rather then using pointer to other nodes, we use indices arrays of different nodes.
The arrays contain unions of different node types of approximately the same size.
Each file has a matching `AST`.

## Globals:
Functions and global variables must be stored in a global table with their visibility.
They are organized into modules.
Each module is comprised of functions, global variables, types, and using declarations.
Modules are organized as a tree: lookups are first done in the current module, followed
by the parent modules, then followed by the `using module` declarations.

# PIPELINE:
A file is first lexed: we obtain a `TokenBuffer`.
This `TokenBuffer` is then used to generate an `AST`, which results in a `ParsedUnit`.
Each `ParsedUnit` has a dependency list of other `ParsedUnit`.
If an import directive is hit, another `ParsedUnit` is created for that file.
All `ParsedUnits` are stored in a `ParsedProgram`, which contains the global table.

A `ParsedProgram` can be sent to any backends to be compiled.