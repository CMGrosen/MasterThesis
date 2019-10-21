grammar Small;

import tokens;

file :
    dcls funcs main EOF
    ;

main :
    MAIN BEGIN dcls stmts END
    ;

funcs :
    func funcs
    |
    ;

func :
    NAME LPAREN params RPAREN BEGIN dcls stmts END
    | NAME LPAREN RPAREN BEGIN dcls stmts END
    ;

stmts :
    stmt DELIMETER stmts
    |
    ;

stmt :
    assign
    | expr
    | iter
    | ifs
    ;

iter :
    WHILE LPAREN expr RPAREN BEGIN dcls stmts END
    ;

ifs :
    IF LPAREN expr RPAREN BEGIN dcls stmts END ELSE BEGIN dcls stmts END
    ;

dcls :
    dcl DELIMETER dcls
    |
    ;

dcl :
    TYPE NAME (assign)?
    | TYPE SQUARE_BRACKET_BEGIN literal SQUARE_BRACKET_END NAME (assign)?
    ;

assign :
    ASSIGN expr;

expr
    : orexpr
    ;

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