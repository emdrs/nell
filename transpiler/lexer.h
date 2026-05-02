#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,     /*NEEDED*/
    TOKEN_INT,            /*NEEDED*/
    TOKEN_FLOAT,          /*NEEDED*/
    TOKEN_STRING,         /*NEEDED*/
    TOKEN_EOF,            /*NEEDED*/
    TOKEN_DOUBLE_COLON,   // ::
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
    TOKEN_IF,             // if
    TOKEN_WHILE,          // while
    TOKEN_FOR,            // for
    TOKEN_SWITCH,         // switch
    TOKEN_BREAK,          // break
    TOKEN_CASE,           // case
    TOKEN_DEFAULT,        // default
    TOKEN_RETURN,         // return
    TOKEN_COMMA,          // ,
    TOKEN_DOUBLE_DOT,     // ..
    TOKEN_GREATER_DOT,    // >.
    TOKEN_DOT_LESS,       // .<
    TOKEN_GREATER_LESS,   // ><
    TOKEN_OR,             // ||
    TOKEN_AND,            // &&
    TOKEN_ARROW           // ->
} TokenType;

#endif
