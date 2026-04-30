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

void rollback(Lexer *l, int pos)
{
    l->pos = pos-1;
    advance(l);
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
            printf("IDENTIFIER(%s)", token.text);
            break;
        case TOKEN_INT:
            printf("INT(%s)", token.text);
            break;
        case TOKEN_FLOAT:
            printf("FLOAT(%s)", token.text);
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
            printf("STRING(%s)", token.text);
            break;
        }
}

void show_token_list(TokenList list)
{
    for (int i = 0; i < list.size; i++) {
        show_token(list.data[i]);
        printf(" ");
    }
    printf("\n");
}

void skip_comment(Lexer *l);

void skip_whitespaces(Lexer *l)
{
    while (l->ch == ' ' || l->ch == '\n' || l->ch == '\t') advance(l);
    skip_comment(l);
}

void skip_comment(Lexer *l) {
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
        advance(l);
        if (l->next_ch == '.' || l->next_ch == '<') {
            rollback(l, l->pos - 1);
        } else {
            is_float = 1;
            if (!is_digit(l)) {
                printf("Expected a digit after '.'\n");
                exit(1);
            }

            while (is_digit(l)) advance(l);
        }
    }

    return (Token) {
        is_float ? TOKEN_FLOAT : TOKEN_INT,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

int is_char(Lexer *l)
{
    return (l->next_ch >= 'a' && l->next_ch <= 'z') ||
           (l->next_ch >= 'A' && l->next_ch <= 'Z') ||
            l->next_ch == '_';
}

int is_keyword(Lexer *l, char *keyword)
{
    if (l->ch == keyword[0]) {
        int start = l->pos;
        while (is_char(l)) advance(l);

        int is =
            strcmp(keyword, strndup(l->source + start, (l->pos - start) + 1)) == 0;

        if (!is) rollback(l, start);
        return is;
    }
    return 0;
}

Token next_token(Lexer *l)
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
        return (Token){ TOKEN_COLON, ":" };
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

    if (l->ch == '"') {
        advance(l);
        int start = l->pos;
        while (1) {
            if (l->ch == '\\') {
                advance(l);
                advance(l);
                continue;
            }

            if (l->ch == '"') break;

            if (l->next_ch == '\0') {
                printf("String withous '\"' on end");
                exit(1);
            }
            advance(l);
        }
        return (Token) { TOKEN_STRING,
            strndup(l->source + start, (l->pos - start)) };
    }

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
    if (is_keyword(l, "return")) return (Token){ TOKEN_RETURN, "return" };

    if (l->ch >= '0' && l->ch < '9') return number(l);

    if ((l->ch >= 'a' && l->ch <= 'z') ||
        (l->ch >= 'A' && l->ch <= 'Z') ||
         l->ch == '_') return identifier(l);

    printf("Unexpected character: '%c'", l->ch);
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
