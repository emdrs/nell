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

    char *old;

    switch (node->type) {
        case AST_NUMBER: {
            printf("NUMBER(%s)\n", node->token->text);
            break;
        }
        case AST_NAME: {
            printf("NAME(%s)\n", node->token->text);
            break;
        }
        case AST_TYPE: {
            char *name = strdup(node->token->text);
            for (int i = 0; i < node->pointer_level; i++) {
                old = name;
                asprintf(&name, "%s%c", name, '*');
                free(old);
            }
            printf("TYPE(%s)\n", name);
            free(name);
            break;
        }
        case AST_VAR_DEF: {
            printf("VAR_DEF(%s)\n", node->token->text);
            show_ast_node(node->left, indent+1);
            show_ast_node(node->right, indent+1);
            break;
        }
        case AST_CONST_DEF: {
            printf("CONST_DEF(%s)\n", node->token->text);
            show_ast_node(node->left, indent+1);
            show_ast_node(node->right, indent+1);
            break;
        }
        case AST_BLOCK: {
            printf("BLOCK\n");
            for (int i = 0; i < node->children->size; i++)
                show_ast_node(array_list_get(node->children, i), indent+1);
            break;
        }
    }
}

// name, struct name, type (*name)(type, type)
int is_type(Parser *p, int offset) { return is_name(parser_peek(p, offset)); }

ASTNode * parse_type(Parser *p)
{
    if(!is_type(p, 0)) return NULL;

    ASTNode *node = create_ast_node(AST_TYPE);
    node->token = parser_peek(p, 0);
    parser_advance(p, 1); // name

    while (parser_peek(p, 0)->type == TOKEN_STAR) {
        node->pointer_level++;
        parser_advance(p, 1); // *
    }

    return node;
}

int is_factor(Token *token) { return is_number(token) || is_name(token); }

ASTNode * parse_factor(Parser *p)
{
    Token *token = parser_peek(p, 0);
    if(!is_factor(token)) return NULL;

    if (token->type == TOKEN_INT || token->type == TOKEN_FLOAT) return parse_number(p);
    if (is_name(token)) return parse_name(p);

    return NULL;
}

int is_assign(Token *token)
{
    return token->type == TOKEN_ASSIGN       ||
           token->type == TOKEN_PLUS_ASSIGN  ||
           token->type == TOKEN_MINUS_ASSIGN ||
           token->type == TOKEN_STAR_ASSIGN  ||
           token->type == TOKEN_SLASH_ASSIGN;
}

ASTNode * parse_var_def(Parser *p)
{
    if(!is_type(p, 0)) return NULL;

    ASTNode *node = create_ast_node(AST_VAR_DEF);
    node->left = parse_type(p);

    Token *token = parser_peek(p, 0);
    if(!is_name(token))
        parser_set_error_and_abort(p, 1.0f/4.0f, "Name needed to define a variable",
                token);

    node->token = token;
    parser_advance(p, 1); // name

    token = parser_peek(p, 0);
    if (!is_assign(token)) return node;

    if (token->type != TOKEN_ASSIGN) 
        parser_set_error_and_abort(p, 2.0f/4.0f,
                "Assign operator not allowed to define a variable", token);
    
    parser_advance(p, 1); // =
    
    token = parser_peek(p, 0);
    if (!is_factor(token))
        parser_set_error_and_abort(p, 3.0f/4.0f,
                "Expression needed to initializa a variable", token);

    node->right = parse_factor(p);

    return node;
}

int is_const_def(Parser *p)
{
    if(parser_peek(p, 0)->type != TOKEN_CONST) return 0;
    return 1;
}

ASTNode * parse_const_def(Parser *p)
{
    if (!is_const_def(p)) return NULL;

    parser_advance(p, 1); // const

    if(!is_type(p, 0))
        parser_set_error_and_abort(p, 1.0f/5.0f, "Type needed to define a const", parser_peek(p, 1));

    ASTNode *node = create_ast_node(AST_CONST_DEF);
    node->left = parse_type(p);

    Token *token = parser_peek(p, 0);
    if(!is_name(token))
        parser_set_error_and_abort(p, 2.0f/5.0f, "Name needed to define a const",
                token);

    node->token = token;
    parser_advance(p, 1); // name

    token = parser_peek(p, 0);
    if (!is_assign(token)) {
        parser_set_error_and_abort(p, 3.0f/5.0f, "Assign is needed to define a const",
                token);
    };

    if (token->type != TOKEN_ASSIGN) 
        parser_set_error_and_abort(p, 3.0f/5.0f,
                "Assign operator not allowed to define a const", token);
    
    parser_advance(p, 1); // =
    
    token = parser_peek(p, 0);
    if (!is_factor(token))
        parser_set_error_and_abort(p, 4.0f/4.0f, "Expression needed to define a const",
                token);

    node->right = parse_factor(p);

    return node;
}

ASTNode * parse_command(Parser *p)
{
    ParseFunction parses[] = {
        parse_var_def,
        parse_const_def
    };

    ASTNode *node = try_parses(p, parses, parses_count(parses));

    if (node == NULL) {
        parser_report_error(p);
        exit(1);
    }

    parser_match(p, TOKEN_SEMICOLON, "';' needed end a command");

    return node;
}

ASTNode * parse_statement(Parser *p)
{
    ParseFunction parses[] = {
        parse_command
    };

    ASTNode *node = try_parses(p, parses, parses_count(parses));

    if (node == NULL) {
        parser_report_error(p);
        exit(1);
    }

    return node;
}

ASTNode * parse_block(Parser *p)
{
    ParseFunction parses[] = {
        parse_statement
    };

    int is_root = p->level == 0;
    if (!is_root) parser_match(p, TOKEN_SEMICOLON, "'{' needed start a block");

    ASTNode *block = create_ast_node(AST_BLOCK);
    block->children = array_list_create(sizeof(ASTNode), 1);

    p->level++;

    while (parser_peek(p, 0)->type != (is_root ? TOKEN_EOF : TOKEN_RBRACE)) {
        ASTNode *node = try_parses(p, parses, parses_count(parses));

        if (node == NULL) {
            parser_report_error(p);
            exit(1);
        }

        array_list_add(block->children, node);
    }

    if (!is_root) parser_match(p, TOKEN_SEMICOLON, "'}' needed end a block");

    p->level--;

    return block;
}

ASTNode * parse(ArrayList *list, char *source, char *file_name)
{
    Parser p = {0}; 
    p.list = list;
    p.source = source;
    p.file_name = file_name;
    p.error_info.progress = -1;

    ASTNode *ast = parse_block(&p);

    Token *t = parser_peek(&p, 0);
    if (t->type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t->text);
        exit(1);
    }

    return ast;
}

