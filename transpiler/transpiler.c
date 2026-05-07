#include "sema.h"
#include "lexer.h"
#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char * read_all_file(char *path)
{
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

void write_file(char *path, char *content)
{
    FILE *f = fopen(path, "w");

    if (!f) {
        printf("Error opening file\n");
        return;
    }

    fprintf(f, "%s", content);

    fclose(f);
}

void compile_code(char *code_path)
{
    char *command;
    asprintf(&command, "gcc %s", code_path);
    system(command);
    free(command);
}

char * generate_code(ASTNode *ast)
{
    if(ast == NULL) return strdup("");
    char *result = NULL;

    switch (ast->type) { }

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("./transpiler file.nell");
        return 1;
    }

    char *source = read_all_file(argv[1]);

    if (source == NULL) return 1;

    ArrayList *tokens = tokenize(source);

    token_list_show(tokens);

    printf("\n");

    ASTNode *ast = parse(tokens, source, argv[1]);
    show_ast_node(ast, 0);

    sema_analize(argv[1], source, ast);
    
    // char *code = generate_code(ast);
    //
    // printf("\n");
    //
    // printf("source: %s\n", source);
    //
    // printf("Ccode:  %s\n", code);
    //
    // write_file("out.c", code);

    return 0;
}

/*
 * Switch case
 * Strings
 * Char
 * Include
 * Struct
 * Enum
 * Arrays
 * Typedef
 * Break
 */

#define LANGB_IMPLEMENTATION
#include "langb.h"
