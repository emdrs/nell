#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_OPERATOR,
    AST_VAR_DEF,
    AST_ASSIGN
} ASTType;

typedef struct AST {
    ASTType type;

    union {
        int number;
        char *identifier;

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
