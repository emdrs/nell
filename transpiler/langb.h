#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LANGB_H
#define LANGB_H

// ==================== UTILS ====================

typedef struct token Token;

typedef struct {
    void *data;
    size_t elementSize;
    size_t size;
    size_t capacity;
} ArrayList;

ArrayList * array_list_create(size_t elementSize, size_t initialCapacity);
void array_list_add(ArrayList *list, void *element);
void * array_list_get(ArrayList *list, size_t index);
void token_list_show(ArrayList *list); 

void report_error(char *file_path, char *source, Token *token, char *error_msg);

// ==================== LEXER ====================

typedef struct token {
    int type; // An enum
    char *text;
    unsigned int line;
    unsigned int column;
} Token;

void token_show(Token *token); // YOU NEED TO IMPLEMENT THIS FUNCTION.

typedef struct {
    char *source;
    unsigned int pos;
    unsigned int line;
    unsigned int column;
    char ch;
    char next_ch;
} Lexer;

void lexer_advance(Lexer *l);
void lexer_rollback(Lexer *l, int pos);

int is_char(char ch);
Token get_identifier(Lexer *l);

int is_digit(char ch);
Token get_number(Lexer *l);

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
    char *file_name;
    int level;
    int in_func;
    int in_case;
    int in_repeat;
    ErrorInfo error_info;
} Parser;

typedef struct ASTNode {
    int type;
    Token *token;
    int pointer_level;
    
    struct ASTNode *left;
    struct ASTNode *right;
    ArrayList *children;
    
    char *resolved_type;
} ASTNode;

ASTNode * create_ast_node(int type);

typedef ASTNode * (*ParseFunction)(Parser *);

ASTNode * create_ast_node(int type);

void parser_set_error(Parser *p, float progress, char *error_message, Token *token, int priority);
void parser_set_error_and_abort(Parser *p, float progress, char *error_message, Token *token);
void parser_report_error(Parser *p);
void parser_advance(Parser *p, int amount);
Token * parser_peek(Parser *p, int offset);
void parser_match(Parser* p, int token_type, char* error_msg);


#define parses_count(functions) sizeof(functions) / sizeof(ParseFunction)

int is_number(Token *token);
int is_string(Token *token);
int is_name(Token *token);

ASTNode * parse_number(Parser *p);
ASTNode * parse_name(Parser *p);
ASTNode * parse_string(Parser *p);

ASTNode * try_parses(Parser *p, ParseFunction functions[], int count);

// ==================== SEMA ====================

#define BUCKET_SIZE 256

typedef struct Symbol {
    char *name;
    int kind;
    char *type_name;
    int pointer_level;
    
    struct SymbolTable *nested_scope;
    
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    Symbol *buckets[BUCKET_SIZE];
    struct SymbolTable *parent;
    char *scope_name;
} SymbolTable;

typedef struct {
    SymbolTable *global_scope;
    SymbolTable *current_scope;

    int error_count;
    char *file_name;
    char *source;
} SemanticAnalyzer;

unsigned int hash(char *name);

int sema_analize_node(SemanticAnalyzer *sema, ASTNode *node); // YOU NEED TO IMPLEMENT THIS

void sema_analize(char *file_name, char *source, ASTNode *root);
void sema_scope_push(SemanticAnalyzer *sema, char *name);
void sema_scope_pop(SemanticAnalyzer *sema);
void sema_define(SemanticAnalyzer *sema, char *name, int kind, char *type,
        int pointer_level, Token *token);
Symbol * sema_lookup(SemanticAnalyzer *sema, char *name);

void sema_report_error(SemanticAnalyzer *sema, Token *token, char *error_msg);

#endif // LANGB_H

#ifdef LANGB_IMPLEMENTATION

// ==================== UTILS IMPLEMENTATIONS ====================

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

char * get_token_source_line(char *source, Token *token)
{
    int pos = 0;
    int line = 1;
    char ch;
    while (1) {
        ch = source[pos];
        if (ch == '\n') {
            line++;
            pos++;
            continue;
        }

        if (line == token->line) {
            int start = pos;
            while(source[pos++] != '\n');
            return strndup(source + start, (pos - start));
        }
        pos++;
    }
}


// ==================== LEXER IMPLEMENTATIONS ====================

void lexer_advance(Lexer *l)
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

void lexer_rollback(Lexer *l, int pos)
{
    l->column -= (l->pos - pos) + 1;
    l->pos = pos-1;
    lexer_advance(l);
}

Token get_number(Lexer *l)
{
    int start = l->pos;
    int is_float = 0;

    while (is_digit(l->next_ch)) lexer_advance(l);

    if (l->next_ch == '.') {
        lexer_advance(l);
        if (l->next_ch == '.' || l->next_ch == '<') {
            lexer_rollback(l, l->pos - 1);
        } else {
            is_float = 1;
            if (!is_digit(l->next_ch)) {
                printf("Expected a digit after '.'\n");
                exit(1);
            }

            while (is_digit(l->next_ch)) lexer_advance(l);
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

    while (is_char(l->next_ch) || is_digit(l->next_ch)) lexer_advance(l);

    return (Token) {
        TOKEN_IDENTIFIER,
        strndup(l->source + start, (l->pos - start) + 1)
    };
}

int is_keyword(Lexer *l, char *keyword)
{
    if (l->ch == keyword[0]) {
        int start = l->pos;
        while (is_char(l->next_ch)) lexer_advance(l);

        int is =
            strcmp(keyword, strndup(l->source + start, (l->pos - start) + 1)) == 0;

        if (!is) lexer_rollback(l, start);
        return is;
    }
    return 0;
}

Token get_string(Lexer *l)
{
    lexer_advance(l);
    int start = l->pos;
    while (1) {
        if (l->ch == '\\') {
            lexer_advance(l);
            lexer_advance(l);
            continue;
        }

        if (l->ch == '"') break;

        if (l->next_ch == '\0') {
            printf("String withous '\"' on end");
            exit(1);
        }
        lexer_advance(l);
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

        lexer_advance(&l);
    } while(tk.type != TOKEN_EOF);

    return list;
}

// ==================== PARSER IMPLEMENTATIONS ====================

ASTNode * create_ast_node(int type)
{
    ASTNode *node = (ASTNode *) malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
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

void report_error(char *file_path, char *source, Token *token, char *error_msg)
{
    if (token == NULL) {
        printf("Unexpected error.\n");
        exit(1);
    }

    printf("%s:%d:%d: error: %s\n", file_path, token->line, token->column, error_msg);
    printf("%4d | %s", token->line, get_token_source_line(source, token));
    printf("%4c | %*c\n", ' ', token->column, '^');
}

void parser_match(Parser* p, int type, char* error_msg)
{
    Token *token = parser_peek(p, 0);
    if (token->type != type) {
        parser_set_error_and_abort(p, 0, error_msg, token);
        exit(1);
    }
    parser_advance(p, 1);
}

void parser_set_error(Parser *p, float progress, char *error_message, Token *token, int priority)
{
    if(priority || p->error_info.progress < progress) p->error_info =
        (ErrorInfo) { progress, error_message, token };
}

void parser_set_error_and_abort(Parser *p, float progress, char *error_message, Token *token)
{
    parser_set_error(p, progress, error_message, token, 1);
    parser_report_error(p);
    exit(1);
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

ASTNode * parse_number(Parser *p)
{
    ASTNode *node = create_ast_node(AST_NUMBER);
    node->token = parser_peek(p, 0);
    parser_advance(p, 1);

    return node;
}

ASTNode * parse_name(Parser *p)
{
    ASTNode *node = create_ast_node(AST_NAME);
    node->token = parser_peek(p, 0);
    parser_advance(p, 1);

    return node;
}

ASTNode * parse_string(Parser *p)
{
    ASTNode *node = create_ast_node(AST_STRING);
    node->token = parser_peek(p, 0);
    parser_advance(p, 1);

    return node;
}

ASTNode * try_parses(Parser *p, ParseFunction functions[], int count)
{
    ASTNode *node = NULL;
    int start = p->pos;
    for (int i = 0; node == NULL && i < count; i++) {
        p->pos = start; // Reset in case a parser consume a token and got error.
        node = functions[i](p);
    }

    return node;
}

void parser_report_error(Parser *p)
{
    report_error(p->file_name, p->source, p->error_info.token, p->error_info.message);
}

// ==================== SEMA IMPLEMENTATIONS ====================

unsigned int hash(char *name)
{
    unsigned int h = 0;
    while (*name != '\0') h = (h << 5) + *name++;
    return h % BUCKET_SIZE;
}

void sema_scope_push(SemanticAnalyzer *sema, char *name)
{
    SymbolTable *new_scope = calloc(1, sizeof(SymbolTable));
    new_scope->parent = sema->current_scope;
    new_scope->scope_name = strdup(name);
    
    sema->current_scope = new_scope;
    
    if (sema->global_scope == NULL) sema->global_scope = new_scope;
}

void sema_scope_pop(SemanticAnalyzer *sema)
{
    if (sema->current_scope->parent == NULL) return; // Root
    
    sema->current_scope = sema->current_scope->parent;
}

void sema_define(SemanticAnalyzer *sema, char *name, int kind, char *type,
        int pointer_level, Token *token)
{
    unsigned int h = hash(name);

    Symbol *s = sema->current_scope->buckets[h];
    while (s != NULL) {
        if (strcmp(s->name, name) == 0) {
            char *error_msg = NULL;
            asprintf(&error_msg, "Redefinition of '%s' in %s", name,
                    sema->current_scope->scope_name);
            report_error(sema->file_name, sema->source, token, error_msg);
            return;
        }
        s = s->next;
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->kind = kind;
    symbol->type_name = strdup(type);
    symbol->pointer_level = pointer_level;

    symbol->next = sema->current_scope->buckets[h];
    sema->current_scope->buckets[h] = symbol;
}

Symbol * sema_lookup(SemanticAnalyzer *sema, char *name)
{
    SymbolTable *current_scope = sema->current_scope;
    
    while (current_scope != NULL) {
        unsigned int h = hash(name);
        Symbol *symbol = current_scope->buckets[h];
        while (symbol != NULL) {
            if (strcmp(symbol->name, name) == 0) return symbol;
            symbol = symbol->next;
        }
        current_scope = current_scope->parent;
    }
    return NULL;
}

void sema_analize(char *file_name, char *source, ASTNode *root) {
    SemanticAnalyzer sema = {0};
    sema.file_name = file_name;
    sema.source = source;
    sema_scope_push(&sema, "global");

    sema_define(&sema, "int", SK_STRUCT, "int", 0, NULL);
    sema_define(&sema, "float", SK_STRUCT, "int", 0, NULL);
    sema_define(&sema, "char", SK_STRUCT, "int", 0, NULL);
    
    sema_analize_node(&sema, root);

    if (sema.error_count > 0) {
        printf("\nErrors: %d\n", sema.error_count);
        exit(1);
    }
}

void sema_report_error(SemanticAnalyzer *sema, Token *token, char *error_msg)
{
    report_error(sema->file_name, sema->source, token, error_msg);
    sema->error_count++;
}

#endif // LANGB_IMPLEMENTATION
