#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int resolve(AST *ast)
{
    if (ast == NULL) {
        printf("Erro: AST nula\n");
        exit(1);
    }

    if (ast->type == AST_NUMBER) return ast->number;

    int left = resolve(ast->op.left);
    int right = resolve(ast->op.right);

    switch (ast->op.sign) {
        case '+': return left + right;
        case '-': return left - right;
        case '*': return left * right;
        case '/': return left / right;
    }

    printf("Erro: tipo desconhecido\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    TokenList tokens = tokenize(argv[1]);

    show_token_list(tokens);

    printf("\n");

    AST *ast = parse(tokens);
    show_ast(ast, 0);

    printf("\n");

    printf("Resultado: %d\n", resolve(ast));

    return 0;
}
