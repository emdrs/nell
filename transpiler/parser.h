#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_OPERATOR
} ASTType;

typedef struct {
    TokenList list;
    int pos;
} Parser;

typedef struct AST {
    ASTType type;

    union {
        int number;

        struct {
            char sign;
            struct AST* left;
            struct AST* right;
        } op;
    };
} AST;

void show_ast(AST* node, int indent);

AST * create_number(Token token);
AST * create_operator(Token token, AST *left, AST *right);

AST * parse(TokenList list);

#endif
