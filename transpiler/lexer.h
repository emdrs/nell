#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_ASSIGN,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    unsigned int line;
    unsigned int column;
} Token;

typedef struct {
    Token* data;
    int size;
    int capacity;
} TokenList;

typedef struct {
    char *source;
    unsigned int pos;
    unsigned int line;
    unsigned int column;
    char ch;
    char next_ch;
} Lexer;

void push(TokenList* list, Token token);

void show_token(Token token);
void show_token_list(TokenList list);

TokenList tokenize(char *source);

#endif
