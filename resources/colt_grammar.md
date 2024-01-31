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
        global mut? <IDENTIFIER>: <TYPE>; |
        global mut? <IDENTIFIER>(: <TYPE>)? = <BINARY_EXPR>;

<LOCAL_VAR_DECL>  ::=
        var mut? <IDENTIFIER>: <TYPE>;    |
        var mut? <IDENTIFIER>(: <TYPE>)? = <BINARY_EXPR>;

**** FUNCTION RELATED ****

**** MODULES RELATED  ****

<IMPORT_EXPR> ::= import <STRING_LITERAL>;
<VISIBILITY>  ::= private: | public:

<MODULE_NAME> ::= <IDENTIFIER>(::<IDENTIFIER>)*
<MODULE_DECL> ::= module <MODULE_NAME> <MODULE_BODY>
<MODULE_BODY> ::= {
        <MODULE_DECL> | <FN_DECL>  |
        <VISIBILITY>  | <VAR_DECL> |
        <USING_DECL>
        }

```