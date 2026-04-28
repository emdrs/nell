#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_OPERATOR,
    AST_VAR_DEF,
    AST_CONST_DEF,
    AST_UPDATE_IDENTIFIER,
    AST_BLOCK,
    AST_COMMAND,
    AST_IF,
    AST_ASSIGN
} ASTType;

typedef struct AST {
    ASTType type;

    union {
        char *identifier;
        struct AST *command;

        struct {
            char *value;
            char *type;
        } number;

        struct {
            struct AST *left;
            struct AST *right;
            char *type;
        } expression;

        struct {
            int is_increment;
            int is_prefix;
            struct AST *target;
        } update_identifier;

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
            struct AST *expression;
            struct AST *block;
        } condition;

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
    char *file;
} Parser;

void show_ast(AST* node, int indent);

AST * parse(TokenList list, char *source, char *file);
AST * create_ast_node(ASTType type);

#endif
