#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_VAR_DEF
} ASTType;

typedef struct AST {
    ASTType type;

    union {
        struct {
            char *name;
            char *type;
        } var_def;
    };
} AST;

typedef struct {
    TokenList list;
    int pos;
} Parser;

void show_ast(AST* node, int indent);

AST * parse(TokenList list);

#endif
