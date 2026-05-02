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
    if(ast == NULL) return strdup("");
    char *result = NULL;

    switch (ast->type) {
        case AST_VAR_DEF: {
            asprintf(&result, "%s %s", ast->var_def.type, ast->var_def.name);
            break;
        }
        case AST_CONST_DEF: {
            asprintf(&result, "const %s %s = %s", ast->const_def.type,
                    ast->const_def.name, generate_code(ast->const_def.value));
            break;
        }
        case AST_NUMBER: {
            asprintf(&result, "%s", ast->number.value);
            break;
        }
        case AST_ASSIGN: {
            char *left_code = generate_code(ast->assign.left);
            char *right_code = generate_code(ast->assign.right);

            asprintf(&result, "%s %s %s", left_code, ast->assign.type, right_code);

            free(left_code);
            free(right_code);
            break;
        }
        case AST_NAME: {
            asprintf(&result, "%s", ast->name);
            break;
        }
        case AST_OPERATOR: {
            char *left_code = generate_code(ast->expression.left);
            char *right_code = generate_code(ast->expression.right);

            if (ast->expression.has_paren) {
                asprintf(&result, "(%s %s %s)", left_code, ast->expression.type,
                        right_code);
            } else {
                asprintf(&result, "%s %s %s", left_code, ast->expression.type,
                        right_code);
            }

            free(left_code);
            free(right_code);
            break;
        }
        case AST_BLOCK: {
            char *code = NULL;
            for (int i = 0; i < ast->block.size; i++) {
                char *statement_code = generate_code(ast->block.statements[i]);

                if (code) asprintf(&code, "%s %s", code, statement_code);
                else      asprintf(&code, "%s", statement_code);

                free(statement_code);
            }
            if (!code) code = "";
            if (ast->block.level == 0) asprintf(&result, "%s", code);
            else                       asprintf(&result, "{ %s }", code);
            break;
        }
        case AST_UPDATE_IDENTIFIER: {
            char *updater = ast->update_identifier.is_increment ? "++" : "--";
            char *code = generate_code(ast->update_identifier.target);
            if (ast->update_identifier.is_prefix)
                asprintf(&result, "%s%s", updater, code);
            else
                asprintf(&result, "%s%s", code, updater);
            break;
        }
        case AST_COMMAND: {
            char *code = generate_code(ast->command);
            asprintf(&result, "%s;", code);
            break;
        }
        case AST_IF: {
            char *expression = generate_code(ast->if_statement.expression);
            char *block = generate_code(ast->if_statement.if_block);
            asprintf(&result, "if (%s) %s", expression, block);

            free(expression);
            free(block);
            break;
        }
        case AST_WHILE: {
            char *expression = generate_code(ast->while_statement.expression);
            char *block = generate_code(ast->while_statement.block);
            asprintf(&result, "while (%s) %s", expression, block);

            free(expression);
            free(block);
            break;
        }
        case AST_FOR: {
            char *start = generate_code(ast->for_statement.start);
            char *end = generate_code(ast->for_statement.end);
            char *block = generate_code(ast->for_statement.block);
            int start_increment = !ast->for_statement.is_start_inclusive;
            int end_increment = ast->for_statement.is_end_inclusive;
            if (start_increment) {
                asprintf(&start, "%s + 1", start);
            }
            if (end_increment) {
                asprintf(&end, "%s + 1", end);
            }
            asprintf(&result, "for (int it = %s; it < %s; it++) %s", start, end,
                    block);

            free(start);
            free(end);
            free(block);
            break;
        }
        case AST_FUNC_DEF: {
            char *block = generate_code(ast->func_def.block);
            char *params = strdup("");
            for (int i = 0; i < ast->func_def.size; i++) {
                if (*params == '\0') 
                    asprintf(&params, "%s", generate_code(ast->func_def.params[i]));
                else
                    asprintf(&params, "%s, %s", params,
                            generate_code(ast->func_def.params[i]));
            }

            asprintf(&result, "%s %s(%s) %s", ast->func_def.return_type,
                    ast->func_def.name, params, block);
            free(block);
            break;
        }
        case AST_RETURN: {
            if (ast->return_statement == NULL) return "return";

            char *expression = generate_code(ast->return_statement);
            asprintf(&result, "return %s", expression);
            free(expression);
            break;
        }
        case AST_FUNC_DEF_PARAM: {
            asprintf(&result, "%s %s", ast->func_def_param.type,
                    ast->func_def_param.name);
            break;
        }
        case AST_FUNC_EXEC: {
            char *params = strdup("");
            for (int i = 0; i < ast->func_exec.params->size; i++) {
                char *old_params = params;
                char *param_code =
                    generate_code((AST *) array_list_get(ast->func_exec.params, i));
                if (*params == '\0') 
                    asprintf(&params, "%s", param_code);
                else
                    asprintf(&params, "%s, %s", params, param_code);
                free(old_params);
                free(param_code);
            }
            asprintf(&result, "%s(%s)", ast->func_exec.name, params);
            free(params);
            break;
        }
        case AST_STRING: {
            asprintf(&result, "\"%s\"", ast->str);
            break;
        }
        case AST_SWITCH: {
            char *identifier = generate_code(ast->switch_statement.value);
            char *block = generate_code(ast->switch_statement.block);
            asprintf(&result, "switch (%s) %s", identifier, block);

            free(identifier);
            free(block);
            break;
        }
        case AST_CASE: {
            char *block = generate_code(ast->case_statement.block);
            if (ast->case_statement.is_default) {
                asprintf(&result, "default: %s", block);
            } else {
                char *expression = generate_code(ast->case_statement.value);
                asprintf(&result, "case %s: %s", expression, block);
                free(expression);
            }
            free(block);
            break;
        }
        case AST_BREAK:
            asprintf(&result, "break");
            break;
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

    ArrayList *tokens = tokenize(source);

    token_list_show(tokens);

    printf("\n");

    AST *ast = parse(tokens, source, argv[1]);
    show_ast(ast, 0);

    char *code = generate_code(ast);

    printf("\n");

    printf("source: %s\n", source);

    printf("Ccode:  %s\n", code);

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
