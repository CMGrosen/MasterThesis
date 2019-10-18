grammar Small;

import tokens;

file
    : decls EOF;// exprs;

decls
    : decl decls
    | NEWLINE
    ;

decl
    : SIMPLE_IDENTIFIER (ASSIGN expr);/*?
    | ARR SIMPLE_IDENTIFIER (ASSIGN BRACKET_START values BRACKET_END)?
    ;

values
    : INT (COMMA INT)*;
*/
expr: INT;