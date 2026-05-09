#include "sema.h"
#include "lexer.h"
#include "parser.h"
#include <_stdio.h>
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

char * generate_code(ASTNode *node, int level)
{
    if(node == NULL) return NULL;

    char *result = NULL;
    char *old = NULL;


    switch (node->type) {
        case AST_NUMBER: {
            asprintf(&result, "%s", node->token->text);
            break;
        }
        case AST_TYPE: {
            asprintf(&result, "%s", node->token->text);
            break;
        }
        case AST_NAME: {
            asprintf(&result, "%s", node->token->text);
            break;
        }
        case AST_VAR_DEF: {
            char *type = generate_code(node->left, level);
            char *value = generate_code(node->right, level);
            if (value == NULL)
                asprintf(&result, "%s %s;", type, node->token->text);
            else
                asprintf(&result, "%s %s = %s;", type, node->token->text, value);

            free(type);
            free(value);
            break;
        }
        case AST_CONST_DEF: {
            char *type = generate_code(node->left, level);
            char *value = generate_code(node->right, level);

            asprintf(&result, "const %s %s = %s;", type, node->token->text, value);

            free(type);
            free(value);
            break;
        }
        case AST_BLOCK: {
            for (int i = 0; i < node->children->size; i++) {
                char *statement =
                    generate_code(array_list_get(node->children, i), level+1);
                if (result == NULL)
                    asprintf(&result, "%s", statement);
                else {
                    old = result;
                    asprintf(&result, "%s %s", result, statement);
                    free(old);
                }

                free(statement);
            }
            if (level > 0) {
                old = result;
                asprintf(&result, "{ %s }", result);
                free(old);
            }
            break;
        }
        case AST_EXPRESSION: {
            result = generate_code(node->left, level);
            if (node->token != NULL) {
                char *right = generate_code(node->right, level);
                asprintf(&result, "%s %s %s", result, node->token->text, right);
                free(right);
            }
            break;
        }
        case AST_ASSIGNMENT: {
            char *left = generate_code(node->left, level);
            char *right = generate_code(node->right, level);

            asprintf(&result, "%s %s %s;", left, node->token->text, right);
            free(left);
            free(right);
            break;
        }
        case AST_FUNC_DEF_PARAM: {
            char *type = generate_code(node->left, level);
            asprintf(&result, "%s %s", type, node->token->text);
            free(type);
            break;
        }
        case AST_FUNC_DEF: {
            char *type = generate_code(node->left, level);
            char *params = NULL;
            for (int i = 0; i < node->children->size; i++) {
                char *param = generate_code(array_list_get(node->children, i), level);
                old = params;
                if (params == NULL)
                    asprintf(&params, "%s", param);
                else
                    asprintf(&params, "%s, %s", params, param);
                free(param);
                free(old);
            }

            char *block = generate_code(node->right, level);

            asprintf(&result, "%s %s(%s) %s", type, node->token->text,
                    params == NULL ? "" : params, block);
            free(block);

            break;
        }
    }

    return result;
}

int verbose = 0;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("./transpiler file.nell");
        return 1;
    }

    for (int i = 1 ; i < argc; i++) {
        if(strcmp(argv[i], "--verbose") == 0) verbose = 1;
    }

    char *source = read_all_file(argv[1]);

    if (source == NULL) return 1;

    ArrayList *tokens = tokenize(source);

    if (verbose) {
        token_list_show(tokens);
        printf("\n");
    }

    ASTNode *ast = parse(tokens, source, argv[1]);

    if (verbose) {
        show_ast_node(ast, 0);
        printf("\n");
    }

    sema_analize(argv[1], source, ast);
    
    char *code = generate_code(ast, 0);

    printf("\n");

    if (verbose) {
        printf("source:\n%s\n", source);
        printf("Ccode:\n%s\n", code);
    }

    write_file("out.c", code);

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
