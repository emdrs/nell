#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#define false 0
#define true 1

typedef enum {
    AST_IDENTIFIER,
    AST_VAR_DEF,
    AST_CONST_DEF,
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
            char *name;
            char *type;
            struct AST *value;
        } field;

        struct {
            struct AST **statements;
            int count;
            int capacity;
            int main;
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
    int in_function;
} Parser;


void show_ast(AST* node, int indent);

AST * parse(TokenList list);

#endif
