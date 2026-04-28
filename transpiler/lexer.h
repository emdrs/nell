#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_DOUBLE_COLON, // ::
    TOKEN_SEMICOLON,      // ;
    TOKEN_COLON,          // :
    TOKEN_ASSIGN,         // =
    TOKEN_EQUALS,         // ==
    TOKEN_NOT_EQUALS,     // !=
    TOKEN_GREATER,        // >
    TOKEN_LESS,           // <
    TOKEN_GREATER_EQUALS, // >=
    TOKEN_LESS_EQUALS,    // <=
    TOKEN_PLUS_ASSIGN,    // +=
    TOKEN_MINUS_ASSIGN,   // -=
    TOKEN_STAR_ASSIGN,    // *=
    TOKEN_SLASH_ASSIGN,   // /=
    TOKEN_PLUS,           // +
    TOKEN_INCREMENT,      // ++
    TOKEN_MINUS,          // -
    TOKEN_DECREMENT,      // --
    TOKEN_STAR,           // *
    TOKEN_SLASH,          // /
    TOKEN_LBRACE,         // {
    TOKEN_RBRACE,         // }
    TOKEN_LPAREN,         // (
    TOKEN_RPAREN,         // )
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
