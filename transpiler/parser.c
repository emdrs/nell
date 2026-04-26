#include "parser.h"
#include "lexer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


AST * create_ast_node(ASTType type)
{
    AST *node = (AST *) malloc(sizeof(AST));
    node->type = type;
    return node;
}

void print_indent(int level) { for (int i = 0; i < level; i++) printf("  "); }

void show_ast(AST* node, int indent)
{
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case AST_VAR_DEF: {
            printf("VAR_DEF(%s:%s)\n", node->var_def.name, node->var_def.type);
            break;
        }
        case AST_NUMBER: {
            printf("NUMBER(%d)\n", node->number);
            break;
        }
        case AST_ASSIGN: {
            printf("ASSIGN(%s)\n", node->assign.type);
            show_ast(node->assign.left, indent + 1);
            show_ast(node->assign.right, indent + 1);
            break;
        }
        case AST_IDENTIFIER: {
            printf("IDENTIFIER(%s)\n", node->identifier);
            break;
        }
        case AST_OPERATOR: {
            printf("OPERATOR(%s)\n", node->op.type);
            show_ast(node->op.left, indent + 1);
            show_ast(node->op.right, indent + 1);
            break;
        }
        case AST_BLOCK: {
            printf("BLOCK(%d)\n", node->block.size);
            for (int i = 0; i < node->block.size; i++)
                show_ast(node->block.statements[i], indent + 1);
            break;
        }
    }
}

void parser_advance(Parser *p, int amount) {
    p->pos += amount;

    if (p->pos < p->list.size) return;

    printf("Trying to advance to a token after EOF\n");
    exit(1);
}

Token parser_peek(Parser *p, int offset) {
    if (p->pos + offset < p->list.size) return p->list.data[p->pos + offset];

    printf("Trying to peek a token after EOF\n");
    exit(1);
}

void match(Parser* p, TokenType type, const char* error_msg) {
    Token token = parser_peek(p, 0);
    if (token.type != type) {
        fprintf(stderr, "Erro de Sintaxe: %s (Found: '%s')\n", 
                error_msg, token.text);
        exit(1);
    }
}

int is_var_def(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_IDENTIFIER &&
        parser_peek(p, 2).type != TOKEN_ASSIGN /*Implicit type*/) return 0;

    return 1;
}

AST * parse_var_def(Parser *p)
{
    AST *node = create_ast_node(AST_VAR_DEF);
    node->var_def.initialized = 0;
    node->var_def.name = parser_peek(p, 0).text;

    int implicit_type = parser_peek(p, 2).type == TOKEN_ASSIGN;
    node->var_def.type = implicit_type ? strdup("int") : parser_peek(p, 2).text;

    parser_advance(p, 3 - implicit_type);

    return node;
}

int is_factor(Token token)
{
    return (token.type == TOKEN_IDENTIFIER ||
            token.type == TOKEN_NUMBER);
}

int is_assign(Parser *p)
{
    if(is_var_def(p)) {
        int offset = 3;
        int implicit_type = parser_peek(p, 0 + offset-1).type == TOKEN_ASSIGN;
        if (!implicit_type && parser_peek(p, 0 + offset).type != TOKEN_ASSIGN)
            return 0;

        // If implicit_type, var_def consume 2 tokens (name and ':')
        offset -= implicit_type; 

        if (!is_factor(parser_peek(p, 1 + offset))) return 0;
    } else {
        if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
        if (parser_peek(p, 1).type != TOKEN_ASSIGN) return 0;
        if (!is_factor(parser_peek(p, 2))) return 0;
    }

    return 1;
}

AST * parse_number(Parser *p)
{
    AST *node = create_ast_node(AST_NUMBER);
    node->number = atoi(parser_peek(p, 0).text);
    parser_advance(p, 1);

    return node;
}

AST * parse_identifier(Parser *p)
{
    AST *node = create_ast_node(AST_IDENTIFIER);
    node->identifier = parser_peek(p, 0).text;
    parser_advance(p, 1);

    return node;
}

AST * parse_factor(Parser *p)
{
    Token factor = parser_peek(p, 0);

    if (factor.type == TOKEN_NUMBER) return parse_number(p);

    return parse_identifier(p);
}

inline int is_expression(Parser *p) { return is_factor(parser_peek(p, 0)); }

int is_operator(Token token)
{
    if (token.type != TOKEN_PLUS  &&
        token.type != TOKEN_MINUS &&
        token.type != TOKEN_STAR  &&
        token.type != TOKEN_SLASH) return 0;

    return 1;
}

AST * parse_expression(Parser *p)
{
    AST *left = parse_factor(p);

    Token operator = parser_peek(p, 0);

    if (!is_operator(operator)) return left;
    
    parser_advance(p, 1);

    AST *op = create_ast_node(AST_OPERATOR);
    op->op.left = left;
    op->op.right = parse_expression(p);
    op->op.type = operator.text;

    return op;
}

AST * parse_assign(Parser *p)
{
    AST *node = create_ast_node(AST_ASSIGN);
    if (is_var_def(p)) {
        node->assign.left = parse_var_def(p);
        node->assign.left->var_def.initialized = 1;
    } else {
        node->assign.left = parse_identifier(p);
    }
    node->assign.type = parser_peek(p, 0).text;
    parser_advance(p, 1);
    node->assign.right = parse_expression(p);

    return node;
}

AST * parse(TokenList list)
{
    Parser p = { list, 0 };

    AST *ast;
    if (is_assign(&p)) {
        ast = parse_assign(&p);
        match(&p, TOKEN_SEMICOLON, "; expected to define a assignment");
        parser_advance(&p, 1);
    } else if (is_var_def(&p)) {
        ast = parse_var_def(&p);
        match(&p, TOKEN_SEMICOLON, "; expected to define variable");
        parser_advance(&p, 1);
    }

    Token t = parser_peek(&p, 0);
    if (t.type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t.text);
        exit(1);
    }

    return ast;
}
