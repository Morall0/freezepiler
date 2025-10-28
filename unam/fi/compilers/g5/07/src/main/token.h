#ifndef TOKEN_H 
#define TOKEN_H

typedef enum {
    KEYWORD,
    IDENTIFIER,
    PUNCTUATOR,
    OPERATOR,
    CONSTANT,
    LITERAL,
    END_OF_FILE,
    NOT_A_TOKEN
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
} token;

#endif
