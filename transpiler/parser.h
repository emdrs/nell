#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_OPERATOR,
    AST_VAR_DEF,
    AST_CONST_DEF,
    AST_BLOCK,
    AST_ASSIGN
} ASTType;

typedef struct AST {
    ASTType type;

    union {
        char *identifier;

        struct {
            char *value;
            char *type;
        } number;

        struct {
            struct AST *left;
            struct AST *right;
            char *type;
        } op;

        struct {
            char *name;
            char *type;
            int initialized;
        } var_def;

        struct {
            char *name;
            char *type;
            struct AST *value;
        } const_def;

        struct {
            struct AST *left;
            struct AST *right;
            char *type;
        } assign;

        struct {
            struct AST **statements;
            int size;
            int capacity;
            int level;
        } block;
    };
} AST;

typedef struct {
    TokenList list;
    int pos;
    char *source;
} Parser;

void show_ast(AST* node, int indent);

AST * parse(TokenList list, char *source);
AST * create_ast_node(ASTType type);

#endif
