grammar Small;

import tokens;

file : stmts EOF;

stmts :
    stmt stmts
    | stmt
    ;    

stmt :
    assign DELIMETER
    | write DELIMETER
    | iter
    | ifs
    | FORK thread
    | event
    ;
    
assign :
    NAME ASSIGN expr
    | arrayAccess ASSIGN expr;
    
iter :
    WHILE LPAREN expr RPAREN scope
    ;

ifs :
    IF LPAREN expr RPAREN scope ELSE scope
    ;

thread :
    threads+=scope (PAR threads+=scope)+
    ;
    
event :
    WHEN LPAREN expr RPAREN;

scope : BEGIN stmts END;

read :
    READ LPAREN NAME RPAREN
    ;

write :
    WRITE LPAREN NAME COMMA expr RPAREN
    ;

expr :
    LPAREN expr RPAREN
    | (OP_SUB | OP_NOT) expr
    | left=expr op=(OP_MUL | OP_DIV | OP_MOD) right=expr
    | left=expr op=(OP_ADD | OP_SUB) right=expr
    | left=expr op=(OP_EQ | OP_NEQ | OP_LT | OP_GT | OP_LEQ | OP_GEQ) right=expr
    | left=expr op=OP_AND right=expr
    | left=expr op=OP_OR right=expr
    | literal
    | read
    | arrayAccess
    | arrayLiteral
    | NAME
    ;

arrayAccess
    : NAME SQUARE_BRACKET_BEGIN expr SQUARE_BRACKET_END
    ;

literal:
    BOOL_LITERAL
    | INT_LITERAL
    ;

arrayLiteral:
    (SQUARE_BRACKET_BEGIN (expr (COMMA expr)*)? SQUARE_BRACKET_END);
