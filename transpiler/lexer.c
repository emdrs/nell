#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void advance(Lexer *l)
{
    l->ch = l->source[++l->pos];
    if (l->ch == '\n'){
        l->line++;
        l->column = 0;
    } else {
        l->column++;
    }
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
    switch (token.type) {
        case TOKEN_IDENTIFIER:
            printf("IDENTIFIER(%s) ", token.text);
            break;
        case TOKEN_INT:
            printf("INT(%s) ", token.text);
            break;
        case TOKEN_FLOAT:
            printf("FLOAT(%s) ", token.text);
            break;
        case TOKEN_EQUALS:
            printf("EQUALS ");
            break;
        case TOKEN_PLUS_EQUALS:
            printf("PLUS_EQUALS ");
            break;
        case TOKEN_MINUS_EQUALS:
            printf("MINUS_EQUALS ");
            break;
        case TOKEN_STAR_EQUALS:
            printf("STAR_EQUALS ");
            break;
        case TOKEN_SLASH_EQUALS:
            printf("SLASH_EQUALS ");
            break;
        case TOKEN_SEMICOLON:
            printf("SEMICOLON ");
            break;
        case TOKEN_COLON:
            printf("COLON ");
            break;
        case TOKEN_EOF:
            printf("EOF ");
            break;
        case TOKEN_PLUS:
            printf("PLUS ");
            break;
        case TOKEN_MINUS:
            printf("MINUS ");
            break;
        case TOKEN_STAR:
            printf("STAR ");
            break;
        case TOKEN_SLASH:
            printf("SLASH ");
            break;
        case TOKEN_LBRACE:
            printf("LBRACE ");
            break;
        case TOKEN_RBRACE:
            printf("RBRACE ");
            break;
        }
}

void show_token_list(TokenList list)
{
    for (int i = 0; i < list.size; i++) show_token(list.data[i]);
    printf("\n");
}

void skip_comment(Lexer *l);

void skip_whitespaces(Lexer *l)
{
    while (l->ch == ' ' || l->ch == '\n' || l->ch == '\t') advance(l);
    skip_comment(l);
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

int is_digit(Lexer *l) { return l->next_ch >= '0' && l->next_ch <= '9'; }

Token number(Lexer *l)
{
    int start = l->pos;
    int is_float = 0;

    while (is_digit(l)) advance(l);

    if (l->next_ch == '.') {
        is_float = 1;
        advance(l);

        if (!is_digit(l)) {
            printf("Expected a digit after '.'\n");
            exit(1);
        }

        while (is_digit(l)) advance(l);
    }

    return (Token) {
        is_float ? TOKEN_FLOAT : TOKEN_INT,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

Token next_token(Lexer *l)
{
    skip_whitespaces(l);
    skip_comment(l);

    switch (l->ch) {
        case ';':  return (Token) { TOKEN_SEMICOLON,    ";" };
        case '{':  return (Token) { TOKEN_LBRACE,       "{" };
        case '}':  return (Token) { TOKEN_RBRACE,       "}" };
        case '=':  return (Token) { TOKEN_EQUALS,       "=" };
        case '\0': return (Token) { TOKEN_EOF,           "" };
    }

    if (l->ch == ':') {
        return (Token){ TOKEN_COLON, ":" };
    }

    if (l->ch == '+') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_PLUS_EQUALS, "+=" };
        }
        return (Token){ TOKEN_PLUS, "+" };
    }

    if (l->ch == '-') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_MINUS_EQUALS, "-=" };
        }
        return (Token){ TOKEN_MINUS, "-" };
    }

    if (l->ch == '*') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_STAR_EQUALS, "*=" };
        }
        return (Token){ TOKEN_STAR, "*" };
    }

    if (l->ch == '/') {
        if (l->next_ch == '=') {
            advance(l);
            return (Token){ TOKEN_SLASH_EQUALS, "/=" };
        }
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

        tk.line = l.line;
        tk.column = l.column;

        push(&list, tk);

        advance(&l);
    } while(tk.type != TOKEN_EOF);

    return list;
}
