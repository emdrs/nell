#include "lexer.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void print_indent(int level) { for (int i = 0; i < level; i++) printf("  "); }

void show_ast_node(ASTNode *node, int indent)
{
    if (node == NULL) return;

    print_indent(indent);

    switch (node->type) {
        case AST_TYPE: {
            printf("TYPE(%s)\n", node->token->text);
            break;
        }
        case AST_VAR_DEF: {
            printf("VAR_DEF(%s)\n", node->token->text);
            show_ast_node(node->left, indent+1);
            show_ast_node(node->right, indent+1);
            break;
        }
    }
}

// name, struct name, type (*name)(type, type)
int is_type(Parser *p) { return is_name(parser_peek(p, 0)); }

ASTNode * parse_type(Parser *p)
{
    if(!is_type(p)) return NULL;

    ASTNode *node = create_ast_node(AST_TYPE);
    node->token = parser_peek(p, 0);
    parser_advance(p, 1);
    return node;
}

ASTNode * parse_var_def(Parser *p)
{
    if(!is_type(p)) return NULL;

    ASTNode *node = create_ast_node(AST_VAR_DEF);
    node->left = parse_type(p);

    Token *token = parser_peek(p, 0);
    if(!is_name(token))
        parser_set_error_and_abort(p, 1.0f/4.0f, "Name needed to define a variable",
                token);

    node->token = token;
    parser_advance(p, 1);

    return node;
}

ASTNode * parse(ArrayList *list, char *source, char *file)
{
    Parser p = {0}; 
    p.list = list;
    p.source = source;
    p.file = file;
    p.error_info.progress = -1;

    ASTNode *ast = parse_var_def(&p);
    parser_match(&p, TOKEN_SEMICOLON, "';' needed to define a variable");
    parser_advance(&p, 1);

    Token *t = parser_peek(&p, 0);
    if (t->type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t->text);
        exit(1);
    }

    return ast;
}

