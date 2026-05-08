#ifndef PARSER_H
#define PARSER_H

#include "langb.h"

typedef enum {
    AST_NUMBER,            /*NEEDED*/
    AST_NAME,              /*NEEDED*/
    AST_STRING,            /*NEEDED*/
    AST_TYPE,
    AST_EXPRESSION,
    AST_VAR_DEF,
    AST_CONST_DEF,
    AST_FUNC_DEF,
    AST_ASSIGNMENT,
    AST_FUNC_DEF_PARAM,
    AST_BLOCK,
} ASTType;

ASTNode * parse(ArrayList *list, char *source, char *file);
void print_indent(int level);
void show_ast_node(ASTNode *node, int indent);

// =================== GRAMMAR ===================

int is_type(Parser *p, int offset);
ASTNode * parse_type(Parser *p);

int is_assign(Token *token);

int is_operator(Token *token);
ASTNode * parse_expression(Parser *p);

int is_var_def(Parser *p);
ASTNode * parse_var_def(Parser *p);

int is_const_def(Parser *p);
ASTNode * parse_const_def(Parser *p);

ASTNode * parse_command(Parser *p);

ASTNode * parse_statement(Parser *p);

ASTNode * parse_block(Parser *p);

#endif
