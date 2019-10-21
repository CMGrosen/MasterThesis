lexer grammar tokens;

LPAREN 	: '(';
RPAREN 	: ')';

MAIN    : 'main';


SQUARE_BRACKET_BEGIN: '[';
SQUARE_BRACKET_END: ']';

BEGIN 	: '{';
END 	: '}';


ASSIGN	: ':=';
COMMA	: ',';

TYPE	: ('int' | 'bool');

NAME 	: [a-zA-Z_][0-9a-zA-Z_]*;

OP_OR  : '||';
OP_AND : '&&';
OP_EQ  : '==';
OP_NEQ : '!=';
OP_LT  : '<' ;
OP_GT  : '>' ;
OP_LEQ : '<=';
OP_GEQ : '>=';
OP_ADD : '+' ;
OP_SUB : '-' ;
OP_MUL : '*' ;
OP_DIV : '/' ;
OP_MOD : '%';
OP_NOT : '!' ;

WHILE   : 'while';
IF      : 'if';
ELSE    : 'else';

INT_LITERAL: '0' | [1-9] [0-9]*; // no leading zeros
BOOL_LITERAL: 'true' | 'false';

WS: [ \r\t\n]+ -> skip; // skip spaces, tabs, newlines


DELIMETER : ';';


//WS: [ \r\t]+ -> skip; // skip spaces, tabs, newlines
//SEMI_COLON: ';';