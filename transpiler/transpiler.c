#include "lexer.h"
#include "parser.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
    TokenList tokens = tokenize(argv[1]);

    show_token_list(tokens);

    printf("\n");

    AST *ast = parse(tokens);
    show_ast(ast, 0);

    return 0;
}
