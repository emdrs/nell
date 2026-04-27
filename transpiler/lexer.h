#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_SEMICOLON,    // ;
    TOKEN_COLON,        // :
    TOKEN_DOUBLE_COLON, // ::
    TOKEN_EQUALS,       // =
    TOKEN_PLUS_EQUALS,  // +=
    TOKEN_MINUS_EQUALS, // -=
    TOKEN_STAR_EQUALS,  // *=
    TOKEN_SLASH_EQUALS, // /=
    TOKEN_PLUS,         // +
    TOKEN_DOUBLE_PLUS,  // ++
    TOKEN_MINUS,        // -
    TOKEN_DOUBLE_MINUS, // --
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
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
