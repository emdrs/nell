#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "langb.h"

typedef enum {
    AST_NUMBER,            /*NEEDED*/
    AST_NAME,              /*NEEDED*/
    AST_STRING,            /*NEEDED*/
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
        char *name;             /*NEEDED*/
        char *str;              /*NEEDED*/
        struct AST *command;

        struct {
            char *value;
            char *type;
        } number;               /*NEEDED*/

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
            ArrayList *params;
        } func_exec;

        struct AST *return_statement;
    };
} AST;


AST * parse(ArrayList *list, char *source, char *file);
void show_ast(AST* node, int indent);

// =================== GRAMMAR ===================
int is_var_def(Parser *p);
AST * parse_var_def(Parser *p);

int is_identifier_updater(Token *token);
int is_identifier_update(Parser *p, int offset);
AST * parse_identifier_update(Parser *p);

int is_func_exec(Parser *p);
AST * parse_func_exec(Parser *p);

int is_factor(Parser *p, int offset);
AST * parse_factor(Parser *p);

int is_operator(Token *token);
int is_expression(Parser *p);
AST * parse_expression(Parser *p);

int is_assign(Token *token);
int is_assignment(Parser *p);
AST * parse_assignment(Parser *p);

int is_block(Parser *p, int level);
AST * parse_block(Parser *p, int level);

int is_const_def(Parser *p);
AST * parse_const_def(Parser *p);

int is_return(Parser *p);
AST * parse_return_expression(Parser *p);

int is_break(Parser *p);
int is_default(Parser *p);

int is_command(Parser *p);
AST * parse_command(Parser *p);

AST * parse_if(Parser *p, int level);

AST * parse_case(Parser *p, int level);

AST * parse_switch(Parser *p, int level);

AST * parse_while(Parser *p, int level);

AST * parse_for(Parser *p, int level);

int is_func_def(Parser *p);
AST * parse_func_def(Parser *p, int level);
AST * parse_func_def_param(Parser *p);

AST * parse_statement(Parser *p, int level);

#endif
