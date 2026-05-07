#include "sema.h"
#include "langb.h"
#include "lexer.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

Symbol * sema_check_type(SemanticAnalyzer *sema, Token *token)
{
    Symbol *symbol = sema_lookup(sema, token->text);

    if (symbol == NULL) {
        char *msg = NULL;
        asprintf(&msg, "Undefined type: %s", token->text);
        sema_report_error(sema, token, msg);
        free(msg);
    }

    return symbol;
}

int compare_nodes_types(ASTNode *node1, ASTNode *node2)
{
    if (node1->resolved_type == NULL || node2->resolved_type == NULL) return 0;
    return strcmp(node1->resolved_type, node2->resolved_type) == 0;
}

int sema_analize_node(SemanticAnalyzer *sema, ASTNode *node)
{
    if (node == NULL) return 0;

    switch (node->type) {
        case AST_NUMBER: {
            node->resolved_type = node->token->type == TOKEN_INT ? "int" : "float";
            break;
        }
        case AST_TYPE: {
            if (sema_check_type(sema, node->token) == NULL) return 0;
            node->resolved_type = node->token->text;
            break;
        }
        case AST_NAME: {
            Symbol *symbol = sema_lookup(sema, node->token->text);

            if (symbol == NULL) {
                sema_report_error(sema, node->token, "Undefined symbol");
                return 0;
            }

            node->resolved_type = symbol->type_name;
            break;
        }
        case AST_BLOCK: {
            char *scope_name;
            if (node->token == NULL)
                asprintf(&scope_name, "%d", sema->anonymous_block_count++);
            else
                scope_name = node->token->text;
            sema_scope_push(sema, scope_name);
            for (int i = 0; i < node->children->size; i++)
                sema_analize_node(sema, array_list_get(node->children, i));
            sema_scope_pop(sema);
            break;
        }
        case AST_EXPRESSION: {
            sema_analize_node(sema, node->left);

            if (node->right != NULL) {
                if (!sema_analize_node(sema, node->right)) return 0;

                if (!compare_nodes_types(node->left, node->right)) {
                    sema_report_error(sema, node->left->token, "Incompatible types");
                    return 0;
                }
            }

            node->resolved_type = node->left->resolved_type;
            break;
        }
        case AST_VAR_DEF: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined type

            sema_define(sema, node->token->text, SK_VARIABLE, node->left->token->text,
                        node->pointer_level, node->token);

            if (node->right != NULL) {
                if (!sema_analize_node(sema, node->right)) return 0;

                if (!compare_nodes_types(node->left, node->right)) {
                    sema_report_error(sema, node->right->token, "Incompatible types");
                    return 0;
                }
            }

            break;
        }
        case AST_CONST_DEF: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined type

            sema_define(sema, node->token->text, SK_CONSTANT, node->left->token->text,
                        node->pointer_level, node->token);

            if (node->right != NULL) {
                if(!sema_analize_node(sema, node->right)) return 0;

                if (!compare_nodes_types(node->left, node->right)) {
                    sema_report_error(sema, node->right->token, "Incompatible types");
                    return 0;
                }
            }

            break;
        }
        case AST_FUNC_DEF_PARAM: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined return type

            sema_define(sema, node->token->text, SK_VARIABLE, node->left->token->text,
                        node->pointer_level, node->token);
            break;
        }
        case AST_FUNC_DEF: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined type

            sema_define(sema, node->token->text, SK_FUNCTION, node->token->text,
                        node->pointer_level, node->token);

            sema_analize_node(sema, node->right);

            break;
        }
    }

    return 1;
}
