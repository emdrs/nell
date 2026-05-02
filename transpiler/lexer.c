#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define LANGB_IMPLEMENTATION
#include "langb.h"

void token_show(Token *token)
{
    switch (token->type) {
        case TOKEN_IDENTIFIER:
            printf("IDENTIFIER(%s)", token->text);
            break;
        case TOKEN_INT:
            printf("INT(%s)", token->text);
            break;
        case TOKEN_FLOAT:
            printf("FLOAT(%s)", token->text);
            break;
        case TOKEN_ASSIGN:
            printf("ASSIGN");
            break;
        case TOKEN_PLUS_ASSIGN:
            printf("PLUS_ASSIGN");
            break;
        case TOKEN_MINUS_ASSIGN:
            printf("MINUS_ASSIGN");
            break;
        case TOKEN_STAR_ASSIGN:
            printf("STAR_ASSIGN");
            break;
        case TOKEN_SLASH_ASSIGN:
            printf("SLASH_ASSIGN");
            break;
        case TOKEN_SEMICOLON:
            printf("SEMICOLON");
            break;
        case TOKEN_COLON:
            printf("COLON");
            break;
        case TOKEN_EOF:
            printf("EOF");
            break;
        case TOKEN_PLUS:
            printf("PLUS");
            break;
        case TOKEN_MINUS:
            printf("MINUS");
            break;
        case TOKEN_STAR:
            printf("STAR");
            break;
        case TOKEN_SLASH:
            printf("SLASH");
            break;
        case TOKEN_LBRACE:
            printf("LBRACE");
            break;
        case TOKEN_RBRACE:
            printf("RBRACE");
            break;
        case TOKEN_DOUBLE_COLON:
            printf("DOUBLE_COLON");
            break;
        case TOKEN_INCREMENT:
            printf("INCREMENT");
            break;
        case TOKEN_DECREMENT:
            printf("DECREMENT");
            break;
        case TOKEN_EQUALS:
            printf("EQUALS");
            break;
        case TOKEN_NOT_EQUALS:
            printf("NOT_EQUALS");
            break;
        case TOKEN_GREATER:
            printf("GREATER");
            break;
        case TOKEN_LESS:
            printf("LESS");
            break;
        case TOKEN_GREATER_EQUALS:
            printf("GREATER_EQUALS");
            break;
        case TOKEN_LESS_EQUALS:
            printf("LESS_EQUALS");
            break;
        case TOKEN_LPAREN:
            printf("LPAREN");
            break;
        case TOKEN_RPAREN:
            printf("RPAREN");
            break;
        case TOKEN_IF:
            printf("IF");
            break;
        case TOKEN_WHILE:
            printf("WHILE");
            break;
        case TOKEN_FOR:
            printf("FOR");
            break;
        case TOKEN_DOUBLE_DOT:
            printf("DOUBLE_DOT");
            break;
        case TOKEN_GREATER_DOT:
            printf("GREATER_DOT");
            break;
        case TOKEN_DOT_LESS:
            printf("DOT_LESS");
            break;
        case TOKEN_GREATER_LESS:
            printf("GREATER_LESS");
            break;
        case TOKEN_OR:
            printf("OR");
            break;
        case TOKEN_AND:
            printf("AND");
            break;
        case TOKEN_RETURN:
            printf("RETURN");
            break;
        case TOKEN_ARROW:
            printf("ARROW");
            break;
        case TOKEN_COMMA:
            printf("COMMA");
            break;
        case TOKEN_STRING:
            printf("STRING(%s)", token->text);
            break;
        case TOKEN_SWITCH:
            printf("SWITCH");
            break;
        case TOKEN_BREAK:
            printf("BREAK");
            break;
        case TOKEN_CASE:
            printf("CASE");
            break;
        case TOKEN_DEFAULT:
            printf("DEFAULT");
            break;
        }
}

Token get_token(Lexer *l)
{
    skip_whitespaces(l);
    skip_comment(l);

    switch (l->ch) {
        case ',':  return (Token) { TOKEN_COMMA,     "," };
        case ';':  return (Token) { TOKEN_SEMICOLON, ";" };
        case '{':  return (Token) { TOKEN_LBRACE,    "{" };
        case '}':  return (Token) { TOKEN_RBRACE,    "}" };
        case '(':  return (Token) { TOKEN_LPAREN,    "(" };
        case ')':  return (Token) { TOKEN_RPAREN,    ")" };
        case '\0': return (Token) { TOKEN_EOF,        "" };
    }

    if (l->ch == '|' && l->next_ch == '|') {
        advance(l);
        return (Token){ TOKEN_OR, "||" };
    }
    if (l->ch == '&' && l->next_ch == '&') {
        advance(l);
        return (Token){ TOKEN_AND, "&&" };
    }

    if (l->ch == ':') {
        if (l->next_ch == ':') {
            advance(l);
            return (Token){ TOKEN_DOUBLE_COLON, "::" };
        }
        return (Token){ TOKEN_COLON, strdup(":") };
    }

    if (l->ch == '+') {
        if (l->next_ch == '+') {
            advance(l);
            return (Token){ TOKEN_INCREMENT, "++" };
        }
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_PLUS_ASSIGN, "+=" };
        }
        return (Token){ TOKEN_PLUS, "+" };
    }

    if (l->ch == '-') {
        if (l->next_ch == '-') {
            advance(l);
            return (Token){ TOKEN_DECREMENT, "--" };
        }
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_MINUS_ASSIGN, "-=" };
        }
        if (l->next_ch == '>') {
            advance(l);
            return (Token){ TOKEN_ARROW, "->" };
        }
        return (Token){ TOKEN_MINUS, "-" };
    }

    if (l->ch == '*') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_STAR_ASSIGN, "*=" };
        }
        return (Token){ TOKEN_STAR, "*" };
    }

    if (l->ch == '/') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_SLASH_ASSIGN, "/=" };
        }
        return (Token){ TOKEN_SLASH, "/" };
    }

    if (l->ch == '"') return get_string(l);

    if (l->ch == '>') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_GREATER_EQUALS, ">=" };
        }
        if (l->next_ch == '.') {
            advance(l);
            return (Token){ TOKEN_GREATER_DOT, ">." };
        }
        if (l->next_ch == '<') {
            advance(l);
            return (Token){ TOKEN_GREATER_LESS, "><" };
        }
        return (Token){ TOKEN_GREATER, ">" };
    }
    
    if (l->ch == '<') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_LESS_EQUALS, "<=" };
        }
        return (Token){ TOKEN_LESS, "<" };
    }

    if (l->ch == '=') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_EQUALS, "==" };
        }
        return (Token){ TOKEN_ASSIGN, "=" };
    }

    if (l->ch == '.') {
        if (l->next_ch == '.') {
            advance(l);
            return (Token){ TOKEN_DOUBLE_DOT, ".." };
        }
        if (l->next_ch == '<') {
            advance(l);
            return (Token){ TOKEN_DOT_LESS, ".<" };
        }
    }

    if (is_keyword(l, "if")) return (Token){ TOKEN_IF, "if" };
    if (is_keyword(l, "while")) return (Token){ TOKEN_WHILE, "while" };
    if (is_keyword(l, "for")) return (Token){ TOKEN_FOR, "for" };
    if (is_keyword(l, "switch")) return (Token){ TOKEN_SWITCH, "switch" };
    if (is_keyword(l, "case")) return (Token){ TOKEN_CASE, "case" };
    if (is_keyword(l, "default")) return (Token){ TOKEN_DEFAULT, "default" };
    if (is_keyword(l, "return")) return (Token){ TOKEN_RETURN, "return" };
    if (is_keyword(l, "break")) return (Token){ TOKEN_BREAK, "break" };

    if (is_digit(l->ch)) return get_number(l);
    if (is_char(l->ch)) return get_identifier(l);

    printf("Unexpected character: '%c'", l->ch);
    exit(1);
}
