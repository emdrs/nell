#include "lexer.h"
#include "parser.h"
#include <_stdio.h>
#include <stdio.h>
#include <stdlib.h>


char * read_all_file(char *path) {
    FILE *f = fopen(path, "rb");

    if (!f) {
        printf("Error opening file\n");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = (char*) malloc(size + 1);
    if (!content) return NULL;

    size_t bytes_read = fread(content, 1, size, f);
    
    content[bytes_read] = '\0'; 

    fclose(f);
    return content;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("./transpiler file.nell");
        return 1;
    }

    char * source = read_all_file(argv[1]);

    if (source == NULL) return 1;

    TokenList tokens = tokenize(source);

    show_token_list(tokens);

    printf("\n");

    AST *ast = parse(tokens);
    show_ast(ast, 0);

    return 0;
}
