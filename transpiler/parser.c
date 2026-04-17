#include "parser.h"
#include <stdlib.h>
#include <stdio.h>


void print_indent(int level)
{
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 espaços
    }
}

void show_ast(AST* node, int indent)
{
    if (!node) return;

    print_indent(indent);

    if (node->type == AST_NUMBER) {
        printf("NUMBER(%d)\n", node->number);
    } else if (node->type == AST_OPERATOR) {
        printf("OPERATOR(%c)\n", node->op.sign);

        show_ast(node->op.left, indent + 1);
        show_ast(node->op.right, indent + 1);
    }
}

AST * create_number(Token token)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_NUMBER;
    node->number = atoi(token.value);
    return node;
}

AST * create_operator(Token token, AST *left, AST *right)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_OPERATOR;
    node->op.sign = token.value[0];
    node->op.left = left;
    node->op.right = right;
    return node;
}

Token peek(Parser* p) {
    return p->list.data[p->pos];
}

void advance_parser(Parser* p) {
    p->pos++;
}

AST* parse_factor(Parser* p) {
    Token token = peek(p);

    if (token.type == TOKEN_NUMBER) {
        advance_parser(p);
        return create_number(token);
    }

    printf("Erro: esperado número\n");
    exit(1);
}

AST* parse_expr(Parser* p) {
    AST* left = parse_factor(p);

    while (1) {
        Token t = peek(p);

        if (t.type == TOKEN_PLUS || t.type == TOKEN_MINUS) {
            advance_parser(p);

            AST* right = parse_factor(p);

            char op = (t.type == TOKEN_PLUS) ? '+' : '-';
            left = create_operator(t, left, right);
        } else {
            break;
        }
    }

    return left;
}

AST *parse(TokenList list)
{
    Parser p = { list, 0 };

    AST* ast = parse_expr(&p);

    Token t = peek(&p);
    if (t.type != TOKEN_EOF) {
        printf("Erro: token inesperado após expressão\n");
        exit(1);
    }

    return ast;
}
