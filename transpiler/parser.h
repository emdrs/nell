#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_VAR_DEF,
    AST_ASSIGN
} ASTType;

typedef struct AST {
    ASTType type;

    union {
        int number;

        struct {
            char *name;
            char *type;
        } var_def;

        struct {
            struct AST *left;
            struct AST *right;
            char *type;
        } assign;
    };
} AST;

typedef struct {
    TokenList list;
    int pos;
} Parser;

void show_ast(AST* node, int indent);

AST * parse(TokenList list);
AST * create_ast_node(ASTType type);

#endif
