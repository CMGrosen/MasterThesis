grammar Small;

import tokens;

file : stmts EOF;

stmts :
    stmt DELIMETER stmts
    | stmt DELIMETER
    ;    

stmt :
    assign
    | write
    | iter
    | ifs
    | FORK thread
    | event
    | skipStmt
    ;
    
assign :
    NAME ASSIGN expr
    | NAME ASSIGN arrayLiteral
    | arrayAccess ASSIGN expr
    ;
    
iter :
    WHILE LPAREN expr RPAREN scope
    ;

ifs :
    IF LPAREN expr RPAREN scope ELSE scope
    ;

thread :
    threads+=scope (AND threads+=scope)+
    ;
    
event :
    WHEN LPAREN expr RPAREN;

skipStmt : SKIPPING;

scope : BEGIN stmts END;

read :
    READ LPAREN INT_LITERAL RPAREN
    ;

write :
    WRITE LPAREN INT_LITERAL COMMA expr RPAREN
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
