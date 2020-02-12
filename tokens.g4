lexer grammar tokens;

LPAREN 	: '(';
RPAREN 	: ')';

WHILE   : 'while';

FORK    : 'fork';

WHEN    : 'when';
DO      : 'do';
AND     : 'and';
IF      : 'if';
ELSE    : 'else';
READ    : 'read';
WRITE   : 'write';

SQUARE_BRACKET_BEGIN: '[';
SQUARE_BRACKET_END: ']';

BEGIN 	: '{';
END 	: '}';

ASSIGN	: '=';
COMMA   : ',';

INT_LITERAL: '0' | [1-9] [0-9]*; // no leading zeros
BOOL_LITERAL: 'true' | 'false';


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

SKIPPING : 'skip';

NAME 	: [a-zA-Z_][0-9a-zA-Z_]*;

WS: [ \r\t\n]+ -> skip; // skip spaces, tabs, newlines
DELIMETER : ';';

COMMENT  :  '//' ~[\r\n]* '\r'? '\n' -> skip ;

//WS: [ \r\t]+ -> skip; // skip spaces, tabs, newlines
//SEMI_COLON: ';';

