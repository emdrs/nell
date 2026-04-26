#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void advance(Lexer *l)
{
    l->ch = l->source[++l->pos];
    if (l->next_ch != '\0') l->next_ch = l->source[l->pos+1];
}

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
        case TOKEN_NUMBER:
            type_str = "NUMBER";
            print_type = descritive;
            break;
        case TOKEN_ASSIGN:
            type_str = "ASSIGN";
            print_type = descritive;
            break;
        case TOKEN_SEMICOLON:
            type_str = "SEMICOLON";
            break;
        case TOKEN_COLON:
            type_str = "COLON";
            break;
        case TOKEN_EOF:
            type_str = "EOF";
            break;
          break;
        case TOKEN_PLUS:
            type_str = "PLUS";
            break;
        case TOKEN_MINUS:
            type_str = "MINUS";
            break;
        case TOKEN_STAR:
            type_str = "STAR";
            break;
        case TOKEN_SLASH:
            type_str = "SLASH";
            break;
        }

    printf(print_type, type_str, token.text);
}

void show_token_list(TokenList list)
{
    for (int i = 0; i < list.size; i++) show_token(list.data[i]);
    printf("\n");
}

void skip_whitespaces(Lexer *l) {
    while (l->ch == ' ' || l->ch == '\n' || l->ch == '\t') advance(l);
}

void skip_comment(Lexer *l)
{
    if (l->ch == '/' && l->next_ch == '/') {
        while (l->ch != '\n' && l->ch != '\0') advance(l);
        skip_whitespaces(l);
    }
}

Token identifier(Lexer *l)
{
    int start = l->pos;

    while ((l->next_ch >= 'a' && l->next_ch <= 'z') ||
           (l->next_ch >= 'A' && l->next_ch <= 'Z') ||
           (l->next_ch >= '0' && l->next_ch <= '9') ||
            l->next_ch == '_') advance(l);

    return (Token) {
        TOKEN_IDENTIFIER,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

Token number(Lexer *l)
{
    int start = l->pos;

    while (l->next_ch >= '0' && l->next_ch <= '9') advance(l);

    return (Token) {
        TOKEN_NUMBER,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

Token next_token(Lexer *l)
{
    skip_whitespaces(l);
    skip_comment(l);

    switch (l->ch) {
        case ';':  return (Token) { TOKEN_SEMICOLON,    ";" };
        case '=':  return (Token) { TOKEN_ASSIGN,       "=" };
        case '\0': return (Token) { TOKEN_EOF,           "" };
    }

    if (l->ch == ':') {
        return (Token){ TOKEN_COLON, ":" };
    }

    if (l->ch == '+') {
        return (Token){ TOKEN_PLUS, "+" };
    }

    if (l->ch == '-') {
        return (Token){ TOKEN_MINUS, "-" };
    }

    if (l->ch == '*') {
        return (Token){ TOKEN_STAR, "*" };
    }

    if (l->ch == '/') {
        return (Token){ TOKEN_SLASH, "/" };
    }

    if (l->ch >= '0' && l->ch <= '9') return number(l);

    if ((l->ch >= 'a' && l->ch <= 'z') ||
        (l->ch >= 'A' && l->ch <= 'Z') ||
         l->ch == '_') return identifier(l);

    printf("Unexpected character: %c", l->ch);
    exit(1);
}

TokenList tokenize(char *source)
{
    TokenList list = { NULL, 0, 2 };
    list.data = (Token*) malloc(sizeof(Token) * list.capacity);

    Lexer l;
    l.source = source;
    l.pos = 0;
    l.line = 1;
    l.column = 1;
    l.ch = source[0];
    if (l.ch == '\0') return list;
    l.next_ch = source[1];

    Token tk;
    do {
        tk = next_token(&l);
        push(&list, tk);

        advance(&l);
    } while(tk.type != TOKEN_EOF);

    return list;
}
