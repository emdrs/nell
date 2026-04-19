#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>


char * read_all_file(char *path)
{
    FILE *f;
    f = fopen(path, "r");

    fseek(f, 0, SEEK_END); 
    long size = ftell(f);
    fseek(f, 0, SEEK_SET); 

    char *content = (char*) malloc(size);
    fread(content, 1, size, f);

    return content;
}

int main(int argc, char *argv[])
{
    printf("--- Debug de Argumentos ---\n");
    printf("Total de args (argc): %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    printf("---------------------------\n");

    if (argc < 2) {
        printf("./transpiler file.nell");
        return 1;
    }

    char * source = read_all_file(argv[1]);

    TokenList tokens = tokenize(source);

    show_token_list(tokens);

    printf("\n");

    AST *ast = parse(tokens);
    show_ast(ast, 0);

    return 0;
}
