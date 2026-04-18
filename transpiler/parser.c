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
    } else if(node->type == AST_VARIABLE) {
        printf("VARIABLE(%s)\n", node->identifier);
    } else if(node->type == AST_ASSIGN) {
        printf("ASSIGN(%s)\n", node->assign.assignment);
        show_ast(node->assign.to, indent + 1);
        show_ast(node->assign.value, indent + 1);
    } else if (node->type == AST_OPERATOR) {
        printf("OPERATOR(%c)\n", node->op.sign);

        show_ast(node->op.left, indent + 1);
        show_ast(node->op.right, indent + 1);
    }
}

void advance_parser(Parser* p, unsigned int amount) { p->pos += amount; }

Token parser_peek(Parser* p, int offset) { return p->list.data[p->pos + offset]; }

AST * create_number(Token token)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_NUMBER;
    node->number = atoi(token.text);
    return node;
}

AST * create_operator(Token token, AST *left, AST *right)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_OPERATOR;
    node->op.sign = token.text[0];
    node->op.left = left;
    node->op.right = right;
    return node;
}

AST * create_identifier(Token token)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_VARIABLE;
    node->identifier = token.text;
    return node;
}

AST* parse_factor(Parser* p) {
    Token token = parser_peek(p, 0);

    if (token.type == TOKEN_NUMBER) return create_number(token);
    if (token.type == TOKEN_IDENTIFIER) return create_identifier(token);

    printf("Erro: esperado número ou identificador\n");
    exit(1);
}

AST * parse_expression(Parser *p)
{
    AST* left = parse_factor(p);
    advance_parser(p, 1);

    while (1) {
        Token t = parser_peek(p, 0);

        if (t.type == TOKEN_PLUS || t.type == TOKEN_MINUS) {
            advance_parser(p, 1);

            AST* right = parse_factor(p);

            left = create_operator(t, left, right);
        } else { break; }
        advance_parser(p, 1);
    }

    return left;
}

AST * parse_assignment(Parser *p)
{
    Token to = parser_peek(p, -1);
    Token sign = parser_peek(p, 0);

    AST *node = malloc(sizeof(AST));
    node->type = AST_ASSIGN;
    node->assign.assignment = sign.text;
    node->assign.to = create_identifier(to);

    advance_parser(p, 1);
    node->assign.value = parse_expression(p);

    return node;
}

AST * parse_statement(Parser *p)
{
    while (1) {
        Token token = parser_peek(p, 0);

        if (token.type == TOKEN_EOF) return NULL;
        if (token.type == TOKEN_EQUALS) return parse_assignment(p);

        advance_parser(p, 1);
    }
}

AST * parse(TokenList list)
{
    Parser p = { list, 0 };

    AST* ast = parse_statement(&p);

    Token t = parser_peek(&p, 0);
    if (t.type != TOKEN_EOF) {
        printf("Erro: token inesperado após expressão\n");
        printf("%s", t.text);
        exit(1);
    }

    return ast;
}
