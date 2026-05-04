#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LANGB_H
#define LANGB_H

// ==================== LEXER ====================

typedef struct {
    int type; // An enum
    char *text;
    unsigned int line;
    unsigned int column;
} Token;

void token_show(Token *token); // YOU NEED TO IMPLEMENT THIS FUNCTION.

typedef struct {
    void *data;
    size_t elementSize;
    size_t size;
    size_t capacity;
} ArrayList;

ArrayList* array_list_create(size_t elementSize, size_t initialCapacity);
void array_list_add(ArrayList *list, void *element);
void * array_list_get(ArrayList *list, size_t index);

void token_list_show(ArrayList *list); 

typedef struct {
    char *source;
    unsigned int pos;
    unsigned int line;
    unsigned int column;
    char ch;
    char next_ch;
} Lexer;

void advance(Lexer *l);
void rollback(Lexer *l, int pos);

int is_identifier(Lexer *l);
Token get_identifier(Lexer *l);

int is_digit(char ch);
Token get_number(Lexer *l);

int is_char(char ch);
int is_keyword(Lexer *l, char *keyword);

Token get_string(Lexer *l);

Token get_token(Lexer *l); // YOU NEED TO IMPLEMENT THIS FUNCTION.

ArrayList * tokenize(char *source);

// ==================== PARSER ====================

typedef struct {
    float progress;
    char *message;
    Token *token;
} ErrorInfo;

typedef struct {
    ArrayList *list;
    int pos;
    char *source;
    char *file;
    int level;
    int in_func;
    int in_case;
    int in_repeat;
    ErrorInfo error_info;
} Parser;

typedef struct AST AST; // YOU NEED TO IMPLEMENT THIS

typedef AST * (*ParseFunction)(Parser *);

AST * create_ast_node(int type);

void parser_set_error(Parser *p, float progress, char *error_message, Token *token, int priority);
void parser_set_error_and_abort(Parser *p, float progress, char *error_message, Token *token);
void parser_show_error(Parser *p);
void parser_advance(Parser *p, int amount);
Token * parser_peek(Parser *p, int offset);
void parser_report_error(Parser *p, Token *token, char *error_msg);
void parser_match(Parser* p, int token_type, char* error_msg);

#define parses_count(functions) sizeof(functions) / sizeof(ParseFunction)

int is_number(Token *token);
int is_string(Token *token);
int is_name(Token *token);

AST * parse_number(Parser *p);
AST * parse_name(Parser *p);
AST * parse_string(Parser *p);

AST * try_parses(Parser *p, ParseFunction functions[], int count);

#endif // LANGB_H

#ifdef LANGB_IMPLEMENTATION

// ==================== LEXER IMPLEMENTATIONS ====================

ArrayList * array_list_create(size_t element_size, size_t initial_capacity) {
    ArrayList *list = (ArrayList *) malloc(sizeof(ArrayList));
    list->elementSize = element_size;
    list->size = 0;
    list->capacity = initial_capacity;
    list->data = malloc(initial_capacity * element_size);
    return list;
}

void array_list_add(ArrayList *list, void *element) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        list->data = realloc(list->data, list->capacity * list->elementSize);
    }
    
    // Convert to char * to avoid compilation error and use 1 byte arithmethc.
    void *target = (char *)list->data + (list->size * list->elementSize);
    memcpy(target, element, list->elementSize);
    list->size++;
}

void* array_list_get(ArrayList *list, size_t index) {
    if (index >= list->size) return NULL;
    return (char *)list->data + (index * list->elementSize);
}

void token_list_show(ArrayList *list)
{
    for (int i = 0; i < list->size; i++) {
        token_show((Token *)array_list_get(list, i));
        printf(" ");
    }
    printf("\n");
}

void advance(Lexer *l)
{
    l->ch = l->source[++l->pos];
    if (l->next_ch != '\0') l->next_ch = l->source[l->pos+1];
    if (l->ch == '\n' && l->next_ch != '\0'){
        l->line++;
        l->column = 0;
    } else {
        l->column++;
    }
}

void rollback(Lexer *l, int pos)
{
    l->column -= (l->pos - pos) + 1;
    l->pos = pos-1;
    advance(l);
}

Token get_number(Lexer *l)
{
    int start = l->pos;
    int is_float = 0;

    while (is_digit(l->next_ch)) advance(l);

    if (l->next_ch == '.') {
        advance(l);
        if (l->next_ch == '.' || l->next_ch == '<') {
            rollback(l, l->pos - 1);
        } else {
            is_float = 1;
            if (!is_digit(l->next_ch)) {
                printf("Expected a digit after '.'\n");
                exit(1);
            }

            while (is_digit(l->next_ch)) advance(l);
        }
    }

    return (Token) {
        is_float ? TOKEN_FLOAT : TOKEN_INT,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

// is 0-9
int is_digit(char ch) { return ch >= '0' && ch <= '9'; }

// is A-Z or a-z or _
int is_char(char ch)
{
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') ||
            ch == '_';
}

Token get_identifier(Lexer *l)
{
    int start = l->pos;

    while (is_char(l->next_ch) || is_digit(l->next_ch)) advance(l);

    return (Token) {
        TOKEN_IDENTIFIER,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

int is_keyword(Lexer *l, char *keyword)
{
    if (l->ch == keyword[0]) {
        int start = l->pos;
        while (is_char(l->next_ch)) advance(l);

        int is =
            strcmp(keyword, strndup(l->source + start, (l->pos - start) + 1)) == 0;

        if (!is) rollback(l, start);
        return is;
    }
    return 0;
}

Token get_string(Lexer *l)
{
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
    return (Token) { TOKEN_STRING, strndup(l->source + start, (l->pos - start)) };
}

ArrayList * tokenize(char *source)
{
    ArrayList *list = array_list_create(sizeof(Token), 2);
    
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
        tk = get_token(&l);

        tk.line = l.line;
        tk.column = l.column;

        array_list_add(list, &tk);

        advance(&l);
    } while(tk.type != TOKEN_EOF);

    return list;
}

// ==================== PARSER IMPLEMENTATIONS ====================

char * get_token_source_line(Parser *p, Token *token)
{
    int pos = 0;
    int line = 1;
    char ch;
    while (1) {
        ch = p->source[pos];
        if (ch == '\n') {
            line++;
            pos++;
            continue;
        }

        if (line == token->line) {
            int start = pos;
            while(p->source[pos++] != '\n');
            return strndup(p->source + start, (pos - start));
        }
        pos++;
    }
}

AST * create_ast_node(int type)
{
    AST *node = (AST *) malloc(sizeof(AST));
    node->type = type;
    return node;
}

void parser_advance(Parser *p, int amount)
{
    p->pos += amount;

    if (p->pos < p->list->size) return;

    printf("Trying to advance to a token after EOF\n");
    exit(1);
}

Token * parser_peek(Parser *p, int offset)
{
    if (p->pos + offset < p->list->size)
        return array_list_get(p->list, p->pos + offset);

    printf("Trying to peek a token after EOF\n");
    exit(1);
}

void parser_report_error(Parser *p, Token *token, char *error_msg)
{
    int offset = 0;
    // Use token previous EOF and add 1 char offset to better visualization
    if(token->type == TOKEN_EOF) {
        token = array_list_get(p->list, p->list->size-2);
        offset = 1;
    }

    printf("%s:%d:%d: error: %s\n", p->file, token->line, token->column, error_msg);
    printf("%4d | %s", token->line, get_token_source_line(p, token));
    printf("%4c | %*c\n", ' ', token->column + offset, '^');
}

void parser_match(Parser* p, int type, char* error_msg)
{
    Token *token = parser_peek(p, 0);
    if (token->type != type) {
        parser_report_error(p, token, error_msg);
        exit(1);
    }
}

void parser_set_error(Parser *p, float progress, char *error_message, Token *token, int priority)
{
    if(priority || p->error_info.progress < progress) p->error_info =
        (ErrorInfo) { progress, error_message, token };
}

void parser_set_error_and_abort(Parser *p, float progress, char *error_message, Token *token)
{
    parser_set_error(p, progress, error_message, token, 1);
    parser_show_error(p);
    exit(1);
}

void parser_show_error(Parser *p)
{
    parser_report_error(p, p->error_info.token, p->error_info.message);
}

int is_number(Token *token)
{
    return token->type == TOKEN_INT || token->type == TOKEN_FLOAT;
}

int is_name(Token *token)
{
    return token->type == TOKEN_IDENTIFIER;
}

int is_string(Token *token)
{
    return token->type == TOKEN_STRING;
}

AST * parse_number(Parser *p)
{
    AST *node = create_ast_node(AST_NUMBER);
    Token *number = parser_peek(p, 0);
    node->number.type = strdup(number->type == TOKEN_INT ? "int" : "float");
    node->number.value = number->text;
    parser_advance(p, 1);

    return node;
}

AST * parse_name(Parser *p)
{
    AST *node = create_ast_node(AST_NAME);
    node->name = parser_peek(p, 0)->text;
    parser_advance(p, 1);

    return node;
}

AST * parse_string(Parser *p)
{
    AST *node = create_ast_node(AST_STRING);
    Token *str = parser_peek(p, 0);
    node->str = str->text;
    parser_advance(p, 1);

    return node;
}

AST * try_parses(Parser *p, ParseFunction functions[], int count)
{
    AST *node = NULL;
    for (int i = 0; node == NULL && i < count; i++) node = functions[i](p);

    return node;
}

#endif // LANGB_IMPLEMENTATION
