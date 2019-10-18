grammar Small;

import tokens;

file
    : decls EOF;// exprs;

decls
    : decl decls
    | NEWLINE
    ;

decl
    : SIMPLE_IDENTIFIER ('=' expr)?
    | 'arr' SIMPLE_IDENTIFIER ('=' '[' values ']')?
    ;

values
    : INT (',' INT)*;

expr: INT;