#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_VARIABLE,
    AST_ASSIGN,
    AST_NUMBER,
    AST_BLOCK,
    AST_FUNC_DEF,
    AST_OPERATOR
} ASTType;

typedef struct parameter {
    char *name;
    char *type;
    struct parameter *next;
} Parameter;

typedef struct AST {
    ASTType type;

    union {
        int number;

        struct {
            char sign;
            struct AST *left;
            struct AST *right;
        } op;

        char *identifier;

        struct {
            char *assignment;
            struct AST *to;
            struct AST *value;
        } assign;

        struct {
            struct AST **statements;
            int count;
            int capacity;
        } block;

        struct {
            char* name;
            Parameter *params;
            int param_count;
            char *return_type;
            struct AST *body;
        } func_def;
    };
} AST;

typedef struct {
    TokenList list;
    int pos;
} Parser;


void show_ast(AST* node, int indent);

AST * create_number(Token token);
AST * create_operator(Token token, AST *left, AST *right);

AST * parse(TokenList list);

#endif
