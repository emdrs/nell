#ifndef PARSER_H
#define PARSER_H

#include "langb.h"

typedef enum {
    AST_NUMBER,            /*NEEDED*/
    AST_NAME,              /*NEEDED*/
    AST_STRING,            /*NEEDED*/
    AST_TYPE,
    AST_VAR_DEF,
} ASTType;

ASTNode * parse(ArrayList *list, char *source, char *file);
void print_indent(int level);
void show_ast_node(ASTNode *node, int indent);

// =================== GRAMMAR ===================

int is_type(Parser *p);
ASTNode * parse_type(Parser *p);

int is_var_def(Parser *p);
ASTNode * parse_var_def(Parser *p);

#endif
