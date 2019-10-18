lexer grammar tokens;

NEWLINE: [\n];

SIMPLE_IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;

INT: '0' | [1-9] [0-9]*;
ASSIGN: '=';
BRACKET_START: '[';
BRACKET_END: ']';
COMMA: ',';
ARR: 'arr';