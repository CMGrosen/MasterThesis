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
    scope PAR threads
    ;
    
threads :
    scope PAR threads 
    | scope
    ; 
    
event :
    WHEN expr DO scope;

scope : BEGIN dcls stmts END;

expr: orexpr;

orexpr
    : orexpr OP_OR andexpr
    | andexpr
    ;

andexpr
    : andexpr OP_AND bexpr
    | bexpr
    ;

bexpr
    : bexpr op=(OP_EQ | OP_NEQ | OP_LT | OP_GT | OP_LEQ | OP_GEQ) aexpr1
    | aexpr1
    ;


aexpr1
    : aexpr1 op=(OP_ADD | OP_SUB) aexpr2
    | aexpr2
    ;

aexpr2
    : aexpr2 op=(OP_MUL | OP_DIV | OP_MOD) term
    | term
    ;

term
    : (OP_SUB | OP_NOT)? value
    ;

value
    : literal
    | NAME
    | functionCall
    | arrayAccess
    | LPAREN expr RPAREN
    | (SQUARE_BRACKET_BEGIN (aexpr1 (COMMA aexpr1)*)? SQUARE_BRACKET_END)
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