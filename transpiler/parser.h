#ifndef PARSER_H
#define PARSER_H

#include "langb.h"

typedef enum {
    AST_NUMBER,            /*NEEDED*/
    AST_IDENTIFIER,        /*NEEDED*/
    AST_STRING,            /*NEEDED*/
    AST_TYPE,
    AST_STATEMENT,
    AST_EXPRESSION,
    AST_VARIABLE,
    AST_CONSTANT,
    AST_FUNCTION,
    AST_PARAMETER,
    AST_CALL,
    AST_ARGUMENT,
    AST_ASSIGNMENT,
    AST_IF,
    AST_ELSE,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,
    AST_BREAK,
    AST_CONTINUE,
    AST_RETURN,

    AST_EMPTY, // for empty statements
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
ASTNode * parse_variable(Parser *p);

int is_const_def(Parser *p);
ASTNode * parse_constant(Parser *p);

ASTNode * parse_instruction(Parser *p);

ASTNode * parse_statement(Parser *p);

ASTNode * parse_block(Parser *p);

ASTNode * parse_function(Parser *p);

ASTNode * parse_call(Parser *p);

ASTNode * parse_else(Parser *p);

ASTNode * parse_if(Parser *p);

ASTNode * parse_while(Parser *p);

ASTNode * parse_for(Parser *p);

#endif
