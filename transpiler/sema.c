#include "sema.h"
#include "langb.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
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

Symbol * sema_lookup_member(SymbolTable *scope, char *name)
{
    if (scope == NULL) return NULL;
    unsigned int h = hash(name);
    for (Symbol *s = scope->buckets[h]; s != NULL; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
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
        case AST_STRING: {
            break;
        }
        case AST_IDENTIFIER: {
            Symbol *symbol = sema_lookup(sema, node->token->text);

            if (symbol == NULL) {
                sema_report_error(sema, node->token, "Undefined symbol");
                return 0;
            }

            node->resolved_type = symbol->type_name;
            break;
        }
        case AST_STATEMENT: {
            sema_analize_node(sema, node->left);
            break;
        }
        case AST_IF: {
            sema_analize_node(sema, node->left);
            sema_analize_node(sema, node->right);
            break;
        }
        case AST_ELSE: {
            sema_analize_node(sema, node->right);
            break;
        }
        case AST_WHILE: {
            sema->loop_depth++;
            sema_analize_node(sema, node->left);
            sema_analize_node(sema, node->right);
            sema->loop_depth--;
            break;
        }
        case AST_FOR: {
            sema->loop_depth++;
            for (int i = 0; i < 3; i++)
                sema_analize_node(sema, array_list_get(node->children, i));
            sema_analize_node(sema, node->right);
            sema->loop_depth--;
            break;
        }
        case AST_FIELD: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined type

            sema_define(sema, node->right->token->text, SK_VARIABLE,
                        node->left->token->text, node->pointer_level,
                        node->right->token);
            break;
        }
        case AST_STRUCT: {
            char *scope_name;
            asprintf(&scope_name, "%s", node->left->token->text);
            Symbol *struct_symbol = sema_define(sema, scope_name, SK_STRUCT,
                                                scope_name, 0, node->left->token);
            sema_scope_push(sema, scope_name);
            if (struct_symbol != NULL)
                struct_symbol->nested_scope = sema->current_scope;
            for (int i = 0; i < node->children->size; i++)
                sema_analize_node(sema, array_list_get(node->children, i));
            sema_scope_pop(sema);
            free(scope_name);
            break;
        }
        case AST_MEMBER: {
            if (!sema_analize_node(sema, node->left)) return 0;
            Symbol *struct_sym = sema_lookup(sema, node->left->resolved_type);

            if (struct_sym == NULL || struct_sym->kind != SK_STRUCT) {
                sema_report_error(sema, node->right->token,
                        "Member access on non-struct value");
                return 0;
            }

            Symbol *field = sema_lookup_member(struct_sym->nested_scope,
                    node->right->token->text);

            if (field == NULL) {
                char *msg = NULL;
                asprintf(&msg, "Undefined member '%s' in %s",
                        node->right->token->text, struct_sym->name);
                sema_report_error(sema, node->right->token, msg);
                free(msg);
                return 0;
            }

            node->resolved_type = field->type_name;
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
        case AST_VARIABLE: {
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
        case AST_ASSIGNMENT: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined type
            if(!sema_analize_node(sema, node->right)) return 0; // Undefined type

            if (!compare_nodes_types(node->left, node->right)) {
                sema_report_error(sema, node->token,
                        "Incompatible types on assignment");
                return 0;
            }

            break;
        }
        case AST_CONSTANT: {
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
        case AST_PARAMETER: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined return type

            sema_define(sema, node->token->text, SK_VARIABLE, node->left->token->text,
                        node->pointer_level, node->token);
            break;
        }
        case AST_FUNCTION: {
            if(!sema_analize_node(sema, node->left)) return 0; // Undefined type

            sema_define(sema, node->token->text, SK_FUNCTION, node->left->resolved_type,
                        node->pointer_level, node->token);


            sema_scope_push(sema, node->token->text);
            for (int i = 0; i < node->children->size; i++)
                sema_analize_node(sema, array_list_get(node->children, i));

            sema->current_return_type = node->left->resolved_type;
            sema_analize_node(sema, node->right);
            sema->current_return_type = NULL;

            sema_scope_pop(sema);

            break;
        }
        case AST_BREAK: {
            if (sema->loop_depth == 0) {
                sema_report_error(sema, node->token, "break outside loop and switch");
                return 0;
            }
            break;
        }
        case AST_CONTINUE: {
            if (sema->loop_depth == 0) {
                sema_report_error(sema, node->token, "continue outside loop");
                return 0;
            }
            break;
        }
        case AST_RETURN: {
            char *return_type = sema->current_return_type;

            if (return_type == NULL) {
                sema_report_error(sema, node->token, "return outside function");
                return 0;
            }

            int is_void = strcmp(return_type, "void") == 0;

            if(is_void && node->right != NULL) {
                sema_report_error(sema, node->right->token,
                        "Void function cannot return expression");
                return 0;
            }

            if(!is_void && node->right == NULL) {
                sema_report_error(sema, node->token,
                        "Non void function needs return expression");
                return 0;
            }

            if (node->right != NULL) {
                if (!sema_analize_node(sema, node->right)) return 0;
                if (strcmp(return_type, node->right->resolved_type) != 0) {
                    sema_report_error(sema, node->token, "Incompatible return type");
                    return 0;
                }
            }
            break;
        }
        case AST_ARGUMENT: {
            if(!sema_analize_node(sema, node->right)) return 0; // Undefined return type
            break;
        }
        case AST_CALL: {
            Symbol *symbol = sema_lookup(sema, node->token->text);

            if (symbol == NULL) {
                sema_report_error(sema, node->token, "Undefined symbol");
                return 0;
            }

            node->resolved_type = symbol->type_name;

            for (int i = 0; i < node->children->size; i++)
                sema_analize_node(sema, array_list_get(node->children, i));

            break;
        }

        case AST_EMPTY: {
            break;
        }
        default: {
            printf("NOT IMPLEMENTED SEMA AST.\n");
            show_ast_node(node, 0);
        }
    }

    return 1;
}
