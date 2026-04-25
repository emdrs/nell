#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void advance(Lexer *lexer)
{
    lexer->ch = lexer->source[++lexer->pos];
    if (lexer->next_ch != '\0') lexer->next_ch = lexer->source[lexer->pos+1];
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
        case TOKEN_EQUALS:
            type_str = "EQUALS";
            break;
        case TOKEN_NUMBER:
            type_str = "NUMBER";
            print_type = descritive;
            break;
        case TOKEN_STAR:
            type_str = "STAR";
            break;
        case TOKEN_SLASH:
            type_str = "SLASH";
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
        case TOKEN_RETURN:
            type_str = "RETURN";
            break;
        case TOKEN_EOL:
            type_str = "EOL";
            break;
        case TOKEN_EOF:
            type_str = "EOF";
            break;
          break;
        default:
          type_str = "UNEXPECTED";
          break;
        }

    printf(print_type, type_str, token.text);
}

void show_token_list(TokenList list)
{
    for (int i = 0; i < list.size; i++) show_token(list.data[i]);
    printf("\n");
}

void skip_whitespaces(Lexer *lexer) {
    while (lexer->ch == ' ' || lexer->ch == '\n' || lexer->ch == '\t') advance(lexer);
}

void skip_comment(Lexer *lexer)
{
    if (lexer->ch == '/' && lexer->next_ch == '/') {
        while (lexer->ch != '\n' && lexer->ch != '\0') advance(lexer);
        skip_whitespaces(lexer);
    }
}

Token number(Lexer *lexer)
{
    int start = lexer->pos;

    while (lexer->next_ch >= '0' && lexer->next_ch <= '9') advance(lexer);

    return (Token) {TOKEN_NUMBER, strndup(lexer->source + start, (lexer->pos - start) + 1) };
}

Token identifier(Lexer *lexer)
{
    int start = lexer->pos;

    while ((lexer->next_ch >= 'a' && lexer->next_ch <= 'z') ||
           (lexer->next_ch >= 'A' && lexer->next_ch <= 'Z') ||
           (lexer->next_ch >= '0' && lexer->next_ch <= '9') ||
            lexer->next_ch == '_') advance(lexer);

    return (Token) {
        TOKEN_IDENTIFIER,
        strndup(lexer->source + start, (lexer->pos - start) + 1)
    };
}

Token next_token(Lexer *lexer)
{
    skip_whitespaces(lexer);
    skip_comment(lexer);

    switch (lexer->ch) {
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

    if (lexer->ch == 'r') {
        if (strcmp("return", strndup(lexer->source + lexer->pos, 6)) == 0) {
            for (int i = 0; i < 5; i++) advance(lexer);
            return (Token) { TOKEN_RETURN, "return" };
        }
    }

    if (lexer->ch == ':') {
        if(lexer->next_ch == ':') {
            advance(lexer);
            return (Token){ TOKEN_DOUBLE_COLON, "::" }; 
        }
        return (Token){ TOKEN_COLON, ":" };
    }

    if (lexer->ch == '*') {
        if (lexer->next_ch == '=') {
            advance(lexer);
            return (Token){ TOKEN_EQUALS, "*=" };
        }
        return (Token){ TOKEN_STAR, "*" };
        
        }
    if (lexer->ch == '/') {
        if (lexer->next_ch == '=') {
            advance(lexer);
            return (Token){ TOKEN_EQUALS, "/=" };
        }
        return (Token){ TOKEN_SLASH, "/" };
    }

    if (lexer->ch == '-') {
        if (lexer->next_ch == '>') {
            advance(lexer);
            return (Token){ TOKEN_ARROW, "->" };
        }
        if (lexer->next_ch == '=') {
            advance(lexer);
            return (Token){ TOKEN_EQUALS, "-=" };
        }
        return (Token){ TOKEN_MINUS, "-" };
    }

    if (lexer->ch == '+') {
        if (lexer->next_ch == '=') {
            advance(lexer);
            return (Token){ TOKEN_EQUALS, "+=" };
        }
        return (Token){ TOKEN_PLUS, "+" };
    }

    if (lexer->ch >= '0' && lexer->ch <= '9') return number(lexer);

    if ((lexer->ch >= 'a' && lexer->ch <= 'z') ||
        (lexer->ch >= 'A' && lexer->ch <= 'Z') ||
         lexer->ch == '_') return identifier(lexer);

    printf("Unexpected lexer->character: %c", lexer->ch);
    exit(1);
}

TokenList tokenize(char *source)
{
    TokenList list = { NULL, 0, 2 };
    list.data = (Token*) malloc(sizeof(Token) * list.capacity);

    Lexer lexer;
    lexer.source = source;
    lexer.pos = 0;
    lexer.line = 1;
    lexer.column = 1;
    lexer.ch = source[0];
    if (lexer.ch == '\0') return list;
    lexer.next_ch = source[1];

    Token tk;
    do {
        tk = next_token(&lexer);
        push(&list, tk);

        advance(&lexer);
    } while(tk.type != TOKEN_EOF);

    return list;
}
