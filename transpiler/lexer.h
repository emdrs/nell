#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_IDENTIFIER,
    TOKEN_EQUALS,
    TOKEN_OPENBRACE,
    TOKEN_CLOSEBRACE,
    TOKEN_OPENBRACKET,
    TOKEN_CLOSEBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOUBLE_COLON,
    TOKEN_ARROW,
    TOKEN_EOL,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;

    char *text;
} Token;

typedef struct {
    Token* data;
    int size;
    int capacity;
} TokenList;

void push(TokenList* list, Token token);

void show_token(Token token);
void show_token_list(TokenList list);

TokenList tokenize(char *source);

#endif
