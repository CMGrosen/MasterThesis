grammar Small;

import tokens;

file :
    dcls funcs main EOF
    ;

main :
    MAIN scope
    ;

funcs :
    func funcs
    |
    ;

func :
    TYPE NAME LPAREN params RPAREN scope
    | TYPE NAME LPAREN RPAREN scope
    ;


dcls :
    dcl DELIMETER dcls
    |
    ;
    
dcl :
    TYPE NAME (assign)?
    | TYPE SQUARE_BRACKET_BEGIN literal SQUARE_BRACKET_END NAME (assign)?
    ;

stmts :
    stmt stmts
    |
    ;    

stmt :
    NAME assign DELIMETER
    | read
    | write
    | expr DELIMETER
    | iter
    | ifs
    | returnStmt DELIMETER
    | FORK thread
    | event
    ;
    
assign :
    ASSIGN expr;
    
iter :
    WHILE LPAREN expr RPAREN scope
    ;

ifs :
    IF LPAREN expr RPAREN scope ELSE scope
    ;

returnStmt :
    RETURN expr
    ;

thread :
    scope (PAR threads+=scope)+
    ;
    
event :
    WHEN expr DO scope;

scope : BEGIN dcls stmts END;

read :
    READ IN NAME DELIMETER
    ;

write :
    WRITE output DELIMETER
    ;

output :
    OUT LPAREN expr RPAREN output
    | OUT LPAREN expr RPAREN
    ;

expr :
    LPAREN expr RPAREN
    | (OP_SUB | OP_NOT) expr
    | left=expr op=(OP_MUL | OP_DIV | OP_MOD) right=expr
    | left=expr op=(OP_ADD | OP_SUB) right=expr
    | left=expr op=(OP_EQ | OP_NEQ | OP_LT | OP_GT | OP_LEQ | OP_GEQ) right=expr
    | left=expr op=OP_AND right=expr
    | left=expr op=OP_OR right=expr
    | functionCall
    | literal
    | NAME
    | arrayAccess
    | (SQUARE_BRACKET_BEGIN (expr (COMMA expr)*)? SQUARE_BRACKET_END)
    ;

arrayAccess
    : NAME SQUARE_BRACKET_BEGIN expr SQUARE_BRACKET_END
    ;

functionCall
    : NAME LPAREN (expr (COMMA expr)*)? RPAREN
    ;

params :
    param COMMA params
    | param
    ;

param :
    TYPE NAME;

literal:
    BOOL_LITERAL
    | INT_LITERAL
    ;