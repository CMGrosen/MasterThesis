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
    | assert
    ;
    
assign :
    NAME ASSIGN expr
    | NAME ASSIGN arrayLiteral
    | arrayAccess ASSIGN expr
    ;

write :
    WRITE LPAREN INT_LITERAL COMMA expr RPAREN
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

skipStmt :
    SKIPPING
    ;

assert :
    ASSERT LPAREN expr RPAREN
    ;

scope :
    BEGIN stmts END
    ;

arrayLiteral:
    (SQUARE_BRACKET_BEGIN (expr (COMMA expr)*)? SQUARE_BRACKET_END)
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

literal:
    BOOL_LITERAL
    | INT_LITERAL
    ;

read :
    READ LPAREN INT_LITERAL RPAREN
    ;

arrayAccess :
    NAME SQUARE_BRACKET_BEGIN expr SQUARE_BRACKET_END
    ;
