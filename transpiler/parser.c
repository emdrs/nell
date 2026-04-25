#include "parser.h"
#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>


void print_indent(int level) { for (int i = 0; i < level; i++) printf("  "); }

void show_ast(AST* node, int indent)
{
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
    }
}

void parser_seek(Parser *p, int offset) {
    p->pos += offset;

    if (p->pos < p->list.size) return;

    printf("Trying to get a token after EOF\n");
    exit(1);
}

Token parser_peek(Parser *p, int offset) { return p->list.data[p->pos + offset]; }

void match(Parser* p, TokenType type, const char* error_msg) {
    Token token = parser_peek(p, 0);
    if (token.type != type) {
        fprintf(stderr, "Erro de Sintaxe: %s (Found: '%s')\n", 
                error_msg, token.text);
        exit(1);
    }
}

AST * parse(TokenList list)
{
    Parser p = { list, 0, false };

    AST *ast = parse_block(&p, 1);

    Token t = parser_peek(&p, 0);
    if (t.type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t.text);
        exit(1);
    }

    return ast;
}
