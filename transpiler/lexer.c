#include "lexer.h"
#include <_string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *src;
unsigned int pos = 0;
char ch;
char next_ch;

void push(TokenList *list, Token token)
{
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->data = realloc(list->data, sizeof(Token) * list->capacity);
    }

    list->data[list->size++] = token;
}

void show_token(Token token)
{
    char *type_str;
    char *simple = "%s ";
    char *descritive = "%s(%s) ";
    char *print_type = simple;
    switch (token.type) {
        case TOKEN_IDENTIFIER:
            type_str = "IDENTIFIER";
            print_type = descritive;
            break;
        case TOKEN_EQUALS:
            type_str = "EQUALS";
            break;
        case TOKEN_NUMBER:
            type_str = "NUMBER";
            print_type = descritive;
            break;
        case TOKEN_PLUS:
            type_str = "PLUS";
            break;
        case TOKEN_MINUS:
            type_str = "MINUS";
            break;
        case TOKEN_LBRACE:
            type_str = "LBRACE";
            break;
        case TOKEN_RBRACE:
            type_str = "RBRACE";
            break;
        case TOKEN_LPAREN:
            type_str = "LPAREN";
            break;
        case TOKEN_RPAREN:
            type_str = "RPAREN";
            break;
        case TOKEN_LBRACKET:
            type_str = "LBRACKET";
            break;
        case TOKEN_RBRACKET:
            type_str = "RBRACKET";
            break;
        case TOKEN_SEMICOLON:
            type_str = "SEMICOLON";
            break;
        case TOKEN_COLON:
            type_str = "COLON";
            break;
        case TOKEN_DOUBLE_COLON:
            type_str = "DOUBLE_COLON";
            break;
        case TOKEN_COMMA:
            type_str = "COMMA";
            break;
        case TOKEN_DOT:
            type_str = "DOT";
            break;
        case TOKEN_ARROW:
            type_str = "ARROW";
            break;
        case TOKEN_EOL:
            type_str = "EOL";
            break;
        case TOKEN_EOF:
            type_str = "EOF";
            break;
          break;
        }

    printf(print_type, type_str, token.text);
}

void show_token_list(TokenList list)
{
    for (int i = 0; i < list.size; i++) {
        Token token = list.data[i];

        show_token(token);
    }
    printf("\n");
}

void advance()
{
    ch = src[++pos];
    if (next_ch != '\0') next_ch = src[pos+1];
}

void skip_whitespaces() { while (ch == ' ' || ch == '\n' || ch == '\t') advance(); }

Token number()
{
    int start = pos;

    while (next_ch >= '0' && next_ch <= '9') advance();

    return (Token) {TOKEN_NUMBER, strndup(src + start, (pos - start) + 1) };
}

Token identifier()
{
    int start = pos;

    while ((next_ch >= 'a' && next_ch <= 'z') ||
           (next_ch >= 'A' && next_ch <= 'Z') ||
           (next_ch >= '0' && next_ch <= '9') ||
            next_ch == '_') advance();

    return (Token) {TOKEN_IDENTIFIER, strndup(src + start, (pos - start) + 1) };
}

Token next_token()
{
    skip_whitespaces();

    switch (ch) {
        // case '\n': return (Token) { TOKEN_EOL,         "\n" };
        case ';':  return (Token) { TOKEN_SEMICOLON,    ";" };
        case '{':  return (Token) { TOKEN_LBRACE,       "{" };
        case '}':  return (Token) { TOKEN_RBRACE,       "}" };
        case '(':  return (Token) { TOKEN_LPAREN,       "(" };
        case ')':  return (Token) { TOKEN_RPAREN,       ")" };
        case '=':  return (Token) { TOKEN_EQUALS,       "=" };
        case ',':  return (Token) { TOKEN_COMMA,        "," };
        case '.':  return (Token) { TOKEN_DOT,          "." };
        case '\0': return (Token) { TOKEN_EOF,           "" };
    }

    if (ch == ':') {
        if(next_ch == ':') {
            advance();
            return (Token){ TOKEN_DOUBLE_COLON, "::" }; 
        }
        return (Token){ TOKEN_COLON, ":" };
    }

    if (ch == '-') {
        if (next_ch == '>') {
            advance();
            return (Token){ TOKEN_ARROW, "->" };
        }
        return (Token){ TOKEN_MINUS, "-" };
    }

    if (ch == '+') return (Token){ TOKEN_PLUS, "+" };

    if (ch >= '0' && ch <= '9') return number();

    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
         ch == '_') return identifier();

    printf("Unexpected character: %c", ch);
    exit(1);
}

TokenList tokenize(char *source)
{
    src = source;
    ch = src[pos];
    next_ch = src[pos+1];

    TokenList list = { NULL, 0, 2 };
    list.data = (Token*) malloc(sizeof(Token) * list.capacity);

    Token tk;
    do {
        tk = next_token();
        advance();

        push(&list, tk);
    } while (tk.type != TOKEN_EOF);

    return list;
}
