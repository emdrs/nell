#include "parser.h"
#include "lexer.h"
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

    switch (node->type) {
        case AST_BLOCK:
            printf("BLOCK(%d)\n", node->block.count);
            for (int i = 0; i < node->block.count; i++) {
                show_ast(node->block.statements[i], indent + 1);
            }
            break;

        case AST_FUNC_DEF:
            printf("FUNCTION_DEFINITION(%s -> %s)\n", 
                    node->func_def.name, node->func_def.return_type);
            
            print_indent(indent + 1);
            printf("PARAMS: ");
            Parameter *p = node->func_def.params;
            while (p) {
                printf("[%s : %s] ", p->name, p->type);
                p = p->next;
            }
            printf("\n");

            show_ast(node->func_def.body, indent + 1);
            break;

        case AST_ASSIGN:
            printf("ASSIGN(%s)\n", node->assign.assignment);
            show_ast(node->assign.value, indent + 1);
            break;

        case AST_OPERATOR:
            printf("OPERATOR(%c)\n", node->op.sign);
            show_ast(node->op.left, indent + 1);
            show_ast(node->op.right, indent + 1);
            break;

        case AST_NUMBER:
            printf("NUMBER(%d)\n", node->number);
            break;

        case AST_VARIABLE:
            printf("VARIABLE(%s)\n", node->identifier);
            break;

        default:
            printf("UNKNOWN_NODE_TYPE\n");
            break;
    }
}

void advance_parser(Parser* p, unsigned int amount) { p->pos += amount; }

Token parser_peek(Parser* p, int offset) { return p->list.data[p->pos + offset]; }

void match(Parser* p, TokenType type, const char* error_msg) {
    Token token = parser_peek(p, 0);
    if (token.type == type) {
        advance_parser(p, 1);
    } else {
        fprintf(stderr, "Erro de Sintaxe: %s (Encontrado: %s)\n", 
                error_msg, token.text);
        exit(1);
    }
}

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

AST * create_assignment(Token to, AST *value, Token sign)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_ASSIGN;
    node->assign.assignment = sign.text;
    node->assign.to = create_identifier(to);
    node->assign.value = value;

    return node;
}

AST* parse_factor(Parser* p) {
    Token token = parser_peek(p, 0);

    advance_parser(p, 1);

    if (token.type == TOKEN_NUMBER) return create_number(token);
    if (token.type == TOKEN_IDENTIFIER) return create_identifier(token);

    fprintf(stderr, "Erro: esperado número ou variável, mas encontrou '%s'\n", token.text);
    exit(1);
}

AST * parse_expression(Parser *p)
{
    AST* left = parse_factor(p);

    while (1) {
        Token t = parser_peek(p, 0);

        if (t.type == TOKEN_PLUS || t.type == TOKEN_MINUS) {
            advance_parser(p, 1);

            AST* right = parse_factor(p);

            left = create_operator(t, left, right);
        } else { break; }
    }
    return left;
}

AST * parse_assignment(Parser *p)
{
    Token to   = parser_peek(p, 0);
    Token sign = parser_peek(p, 1);

    AST *node = malloc(sizeof(AST));
    node->type = AST_ASSIGN;
    node->assign.assignment = sign.text;
    node->assign.to = create_identifier(to);

    advance_parser(p, 2); // Consume identifier and equals
    node->assign.value = parse_expression(p);

    return node;
}

AST* parse_statement(Parser* p) {
    Token t1 = parser_peek(p, 0);
    Token t2 = parser_peek(p, 1);

    if (t1.type == TOKEN_IDENTIFIER && t2.type == TOKEN_EQUALS)
        return parse_assignment(p);

    return parse_expression(p);
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
