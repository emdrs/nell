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

char * generate_code(AST *ast)
{
    if(ast == NULL) return NULL;
    char *result = strdup("");

    switch (ast->type) {
        case AST_VAR_DEF: {
            if(ast->var_def.initialized)
                sprintf(result, "%s %s", ast->var_def.type, ast->var_def.name);
            else
                sprintf(result, "%s %s;", ast->var_def.type, ast->var_def.name);
            break;
        }
        case AST_NUMBER: {
            sprintf(result, "%d", ast->number);
            break;
        }
        case AST_ASSIGN: {
            sprintf(result, "%s %s %s;", generate_code(ast->assign.left),
                    ast->assign.type, generate_code(ast->assign.right));
            break;
        }
        case AST_IDENTIFIER: {
            sprintf(result, "%s", ast->identifier);
            break;
        }
        case AST_OPERATOR: {
            sprintf(result, "%s %s %s", generate_code(ast->op.left), ast->op.type,
                    generate_code(ast->op.right));
            break;
        }
        case AST_BLOCK: {
            char *code = strdup("");
            for (int i = 0; i < ast->block.size; i++)
                sprintf(code, "%s %s", code, generate_code(ast->block.statements[i]));
            sprintf(result, "{%s }", code);
            break;
        }
        case AST_CONST_DEF: {
            sprintf(result, "const %s %s;", ast->const_def.type, ast->const_def.name);
            break;
        }
    }

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

    TokenList tokens = tokenize(source);

    show_token_list(tokens);

    printf("\n");

    AST *ast = parse(tokens, source);
    show_ast(ast, 0);

    char *code = generate_code(ast);

    printf("\n");

    printf("source: %s\n", source);

    printf("Ccode:  %s\n", code);

    return 0;
}
