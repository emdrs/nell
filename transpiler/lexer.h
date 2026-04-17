#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;

    char *value;
} Token;

typedef struct {
    Token* data;
    int size;
    int capacity;
} TokenList;

void push(TokenList* list, Token token);

void show_token(Token token);
void show_token_list(TokenList list);

TokenList tokenize(const char *source);

#endif
