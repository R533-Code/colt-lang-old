## Keywords:
```
mut, var, global, undefined,
using, module, import, private, public,
fn, in, out, inout,
```

## Grammar:
```
<GLOBAL_DECL> ::=
        <IMPORT_EXPR> | <MODULE_DECL>     |
        <FN_DECL>     | <GLOBAL_VAR_DECL> |
        <USING_DECL>

<USING_DECL>  ::= using module <MODULE_NAME>;      |
        using module <IDENTIFIER> = <MODULE_NAME>; |
        using <IDENTIFIER> = <IDENTIFIER>;

**** VARIABLE RELATED ****

<GLOBAL_VAR_DECL> ::=
        global mut? <IDENTIFIER>: <TYPE> = undefined;        |
        global mut? <IDENTIFIER>(: <TYPE>)? = <BINARY_EXPR>;

<LOCAL_VAR_DECL>  ::=        
        var mut? <IDENTIFIER>: <TYPE> = undefined;           |
        var mut? <IDENTIFIER>(: <TYPE>)? = <BINARY_EXPR>;

**** FUNCTION RELATED ****

<FN_DECL>    ::=
        fn <IDENTIFIER>\(<FN_PARAM>?(, <FN_PARAM>)*\);   |
        fn <IDENTIFIER>\(<FN_PARAM>?(, <FN_PARAM>)*\) <FN_BODY>

<PARAM_QUAL> ::= const? (in | out | inout)?
<FN_PARAM>   ::= <IDENTIFIER>: <PARAM_QUAL> <TYPE>

**** MODULES RELATED  ****

<IMPORT_EXPR> ::= import <MODULE_NAME>;
<VISIBILITY>  ::= private: | public:

<MODULE_NAME> ::= <IDENTIFIER>(::<IDENTIFIER>)*
<MODULE_DECL> ::= module <MODULE_NAME> <MODULE_BODY>
<MODULE_BODY> ::= {
        <MODULE_DECL> | <FN_DECL>  |
        <VISIBILITY>  | <VAR_DECL> |
        <USING_DECL>
        }

```