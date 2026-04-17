#include "lexer.h"
#include <_string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *src;
unsigned int pos = 0;
char ch;

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
    switch (token.type) {
        case TOKEN_NUMBER:
            type_str = "NUMBER";
            break;
        case TOKEN_PLUS:
            type_str = "PLUS";
            break;
        case TOKEN_MINUS:
            type_str = "MINUS";
            break;
        case TOKEN_EOF:
            type_str = "EOF";
            break;
    }

    printf("%s(%s) ", type_str, token.value);
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
    pos++;
    ch = src[pos];
}

void skip_whitespaces() { while (ch == ' ' || ch == '\n' || ch == '\t') advance(); }

Token number()
{
    int start = pos;

    while (ch >= '0' && ch <= '9') advance();

    return (Token) {TOKEN_NUMBER, strndup(src + start, pos - start) };
}

Token sign()
{
    TokenType type;
    switch (ch) {
        case '+':
            type = TOKEN_PLUS;
            break;
        case '-':
            type = TOKEN_MINUS;
            break;
    }

    char* string = malloc(2);
    string[0] = ch;
    string[1] = '\0';

    Token t = (Token) { type, string  };
    
    advance();
    return t;
}

Token next_token()
{
    skip_whitespaces();

    if (ch > '0' && ch < '9') return number();

    if (ch == '+' || ch == '-') return sign();

    if (ch == '\0' ) return (Token) { TOKEN_EOF, "" };

    printf("Unexpected character");
    exit(1);
}

TokenList tokenize(const char *source)
{

    src = source;
    ch = src[pos];


    TokenList list = { NULL, 0, 2 };
    list.data = (Token*) malloc(sizeof(Token) * list.capacity);

    Token tk;
    do {
        tk = next_token();
        push(&list, tk);
    } while (tk.type != TOKEN_EOF);

    return list;
}
