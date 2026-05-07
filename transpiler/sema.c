#include "sema.h"
#include "langb.h"
#include "lexer.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

Symbol * sema_check_node_type(SemanticAnalyzer *sema, ASTNode *node)
{
    Symbol *symbol = sema_lookup(sema, node->token->text);

    if (symbol == NULL) {
        char *msg = NULL;
        asprintf(&msg, "Undefined type: %s", node->token->text);
        sema_report_error(sema, node->token, msg);
        free(msg);
    }

    return symbol;
}

void sema_analize_node(SemanticAnalyzer *sema, ASTNode *node)
{
    if (node == NULL) return;

    switch (node->type) {
        case AST_NUMBER: {
            node->resolved_type = node->token->type == TOKEN_INT ? "int" : "float";
            break;
        }
        case AST_BLOCK: {
            for (int i = 0; i < node->children->size; i++)
                sema_analize_node(sema, array_list_get(node->children, i));
            break;
        }
        case AST_VAR_DEF: {
            if (sema_check_node_type(sema, node->left) == NULL) break;

            if (node->right != NULL) {
                sema_analize_node(sema, node->right);
                if (strcmp(node->left->token->text, node->right->resolved_type) != 0) {
                    sema_report_error(sema, node->right->token, "Incompatible types");
                }
            }

            sema_define(sema, node->token->text, SK_VARIABLE, node->left->token->text,
                        node->pointer_level, node->token);
            break;
        }
    }
}
