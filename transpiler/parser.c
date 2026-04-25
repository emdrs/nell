#include "parser.h"
#include "lexer.h"
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
    if (parser_peek(p, 2).type != TOKEN_IDENTIFIER) return 0;

    return 1;
}

AST * parse_var_def(Parser *p)
{
    AST *node = create_ast_node(AST_VAR_DEF);
    node->var_def.name = parser_peek(p, 0).text;
    node->var_def.type = parser_peek(p, 2).text;

    parser_advance(p, 3);

    return node;
}

AST * parse_number(Parser *p)
{
    AST *node = create_ast_node(AST_NUMBER);
    node->number = atoi(parser_peek(p, 0).text);
    parser_advance(p, 1);

    return node;
}
AST * parse(TokenList list)
{
    Parser p = { list, 0 };

    AST *ast;
    if (is_var_def(&p)) {
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
