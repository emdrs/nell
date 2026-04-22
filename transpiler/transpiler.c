#include "lexer.h"
#include "parser.h"
#include <string.h>
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

char * generate_code(AST *ast)
{
    char *result = strdup("");
    char *temp = strdup("");
    char *params = strdup("");

    switch (ast->type) {
        case AST_BLOCK:
            for (int i = 0; i < ast->block.count; i++) {
                 sprintf(temp, "%s%s", temp, generate_code(ast->block.statements[i]));
            }
            sprintf(result, ast->block.main ? "%s" : "{ %s }", temp);
            break;
        case AST_FUNC_DEF:
            for (AST *p = ast->func_def.params; p != NULL; p = p->func_def_param.next) {
                char *param_code = generate_code(p);
                if (strcmp(params, "") == 0) sprintf(params, "%s", param_code);
                else sprintf(params, "%s, %s", params, param_code);
            }

            sprintf(result, "%s %s (%s) %s", ast->func_def.return_type,
                    ast->func_def.name, params, generate_code(ast->func_def.body));
            break;

        case AST_FUNC_DEF_PARAM:
            sprintf(result, "%s %s", ast->func_def_param.type, ast->func_def_param.name);
            break;
        case AST_FUNC_RETURN:
            sprintf(result, "return %s;", generate_code(ast->func_return));
            break;
        case AST_OPERATOR:
            sprintf(result, "%s %c %s", generate_code(ast->op.left), ast->op.sign, generate_code(ast->op.right));
            break;
        case AST_IDENTIFIER:
            sprintf(result, "%s", ast->identifier);
            break;
        case AST_NUMBER:
            sprintf(result, "%d", ast->number);
            break;
    }

    return result;
}

int main(int argc, char *argv[])
{
    // if (argc < 2) {
    //     printf("./transpiler file.nell");
    //     return 1;
    // }
    //
    // char *source = read_all_file(argv[1]);
    //
    // if (source == NULL) return 1;
    //
    // TokenList tokens = tokenize(source);
    //
    // show_token_list(tokens);
    //
    // printf("\n");
    //
    // AST *ast = parse(tokens);
    // show_ast(ast, 0);

    char *source = "soma :: (a: int, b: int) -> int { return a + b; }";

    TokenList tokens = tokenize(source);

    show_token_list(tokens);

    printf("\n");

    AST *ast = parse(tokens);
    show_ast(ast, 0);

    char *code = generate_code(ast);

    printf("\n");

    printf("source: %s\n", source);

    printf("Ccode:  %s\n", code);

    return 0;
}
