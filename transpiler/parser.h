#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING,
    AST_OPERATOR,
    AST_VAR_DEF,
    AST_CONST_DEF,
    AST_UPDATE_IDENTIFIER,
    AST_BLOCK,
    AST_COMMAND,
    AST_IF,
    AST_SWITCH,
    AST_CASE,
    AST_BREAK,
    AST_WHILE,
    AST_FOR,
    AST_FUNC_DEF,
    AST_FUNC_DEF_PARAM,
    AST_FUNC_EXEC,
    AST_RETURN,
    AST_ASSIGN
} ASTType;

typedef struct AST {
    ASTType type;

    union {
        char *identifier;
        char *str;
        struct AST *command;

        struct {
            char *value;
            char *type;
        } number;

        struct {
            struct AST *left;
            struct AST *right;
            char *type;
            int has_paren;
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
            struct AST *if_block;
            struct AST *else_block;
        } if_statement;

        struct {
            struct AST *value;
            struct AST *block;
        } switch_statement;

        struct {
            struct AST *value;
            struct AST *block;
            int is_default;
        } case_statement;

        struct {
            struct AST *expression;
            struct AST *block;
        } while_statement;

        struct {
            struct AST *start;
            int is_start_inclusive;
            int is_end_inclusive;
            struct AST *end;
            struct AST *block;
        } for_statement;

        struct {
            struct AST **statements;
            int size;
            int capacity;
            int level;
        } block;

        struct {
            char *name;
            char *type;
        } func_def_param;

        struct {
            char *name;
            char *return_type;
            struct AST **params;
            int size;
            int capacity;
            struct AST *block;
        } func_def;

        struct {
            char *name;
            struct AST **params;
            int size;
            int capacity;
        } func_exec;

        struct AST *return_statement;
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
