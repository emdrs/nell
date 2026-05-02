#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void skip_whitespaces(Lexer *l);
void skip_comment(Lexer *l);

int is_identifier(Lexer *l);
Token get_identifier(Lexer *l);

int is_digit(char ch);
Token get_number(Lexer *l);

int is_char(char ch);
int is_keyword(Lexer *l, char *keyword);

Token get_string(Lexer *l);

Token get_token(Lexer *l); // YOU NEED TO IMPLEMENT THIS FUNCTION.

ArrayList * tokenize(char *source);

#ifdef LANGB_IMPLEMENTATION

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

void skip_whitespaces(Lexer *l)
{
    while (l->ch == ' ' || l->ch == '\n' || l->ch == '\t') advance(l);
}

void skip_comment(Lexer *l)
{
    if (l->ch == '/' && l->next_ch == '/') {
        while (l->ch != '\n' && l->ch != '\0') advance(l);
        skip_whitespaces(l);
    }
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
    
    printf("%s\n", source);

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

#endif // LANGB_IMPLEMENTATION
