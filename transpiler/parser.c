#include "lexer.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void print_indent(int level) { for (int i = 0; i < level; i++) printf("  "); }

void show_ast_node(ASTNode *node, int indent)
{
    if (node == NULL) return;

    print_indent(indent);

    char *old;

    switch (node->type) {
        case AST_NUMBER: {
            printf("NUMBER(%s)\n", node->token->text);
            break;
        }
        case AST_NAME: {
            printf("NAME(%s)\n", node->token->text);
            break;
        }
        case AST_STRING: {
            printf("STRING(%s)\n", node->token->text);
            break;
        }
        case AST_TYPE: {
            char *name = strdup(node->token->text);
            for (int i = 0; i < node->pointer_level; i++) {
                old = name;
                asprintf(&name, "%s%c", name, '*');
                free(old);
            }
            printf("TYPE(%s)\n", name);
            free(name);
            break;
        }
        case AST_EXPRESSION: {
            if (node->token != NULL) {
                printf("EXPRESSION(%s)\n", node->token->text);
                show_ast_node(node->left, indent + 1);
                show_ast_node(node->right, indent + 1);
                break;
            }

            printf("EXPRESSION\n");
            show_ast_node(node->left, indent + 1);
            break;
        }
        case AST_VAR_DEF: {
            printf("VAR_DEF(%s)\n", node->token->text);
            show_ast_node(node->left, indent+1);
            show_ast_node(node->right, indent+1);
            break;
        }
        case AST_CONST_DEF: {
            printf("CONST_DEF(%s)\n", node->token->text);
            show_ast_node(node->left, indent+1);
            show_ast_node(node->right, indent+1);
            break;
        }
        case AST_FUNC_DEF_PARAM: {
            printf("PARAM\n");
            show_ast_node(node->left, indent+1);
            print_indent(indent + 1);
            printf("NAME(%s)\n", node->token->text);
            break;
        }
        case AST_FUNC_DEF: {
            printf("FUNC_DEF(%s)\n", node->token->text);
            show_ast_node(node->left, indent + 1);
            print_indent(indent+1);
            printf("PARAMS:\n");
            for (int i = 0; i < node->children->size; i++)
                show_ast_node(array_list_get(node->children, i), indent+2);
            show_ast_node(node->right, indent + 1);
            break;
        }
        case AST_ASSIGNMENT: {
            printf("ASSIGNMENT\n");
            show_ast_node(node->left, indent + 1);
            show_ast_node(node->right, indent + 1);
            break;
        }
        case AST_COMMAND: {
            printf("COMMAND\n");
            show_ast_node(node->left, indent + 1);
            break;
        }
        case AST_IF: {
            printf("IF\n");
            show_ast_node(node->left, indent + 1);
            show_ast_node(node->right, indent + 1);
            break;
        }
        case AST_ELSE: {
            printf("ELSE\n");
            show_ast_node(node->right, indent + 1);
            break;
        }
        case AST_WHILE: {
            printf("WHILE\n");
            show_ast_node(node->left, indent + 1);
            show_ast_node(node->right, indent + 1);
            break;
        }
        case AST_FOR: {
            printf("FOR\n");
            for (int i = 0; i < 3; i++)
                show_ast_node(array_list_get(node->children, i), indent + 1);
            break;
        }
        case AST_BLOCK: {
            printf("BLOCK\n");
            for (int i = 0; i < node->children->size; i++)
                show_ast_node(array_list_get(node->children, i), indent+1);
            break;
        }
        case AST_BREAK: {
            printf("BREAK\n");
            break;
        }
        case AST_RETURN: {
            printf("RETURN\n");
            show_ast_node(node->right, indent+1);
            break;
        }
        case AST_FUNC_EXEC_PARAM: {
            printf("PARAM\n");
            show_ast_node(node->right, indent + 1);
            break;
        }
        case AST_FUNC_EXEC: {
            printf("FUNC_EXEC\n");
            print_indent(indent+1);
            printf("PARAMS:\n");
            for (int i = 0; i < node->children->size; i++)
                show_ast_node(array_list_get(node->children, i), indent+2);
            break;
        }

        case AST_EMPTY: {
            printf("EMPTY\n");
            break;
        }
        default: {
            printf("NOT IMPLEMENTED SHOW AST\n");
        }
    }
}

// name, struct name, type (*name)(type, type)
int is_type(Parser *p, int offset) { return is_name(parser_peek(p, offset)); }

ASTNode * parse_type(Parser *p)
{
    if(!is_type(p, 0)) return NULL;

    ASTNode *node = create_ast_node(AST_TYPE);
    node->token = parser_peek(p, 0);
    parser_advance(p, 1); // name

    while (parser_peek(p, 0)->type == TOKEN_STAR) {
        node->pointer_level++;
        parser_advance(p, 1); // *
    }

    return node;
}

int is_factor(Token *token) { return is_number(token) || is_name(token); }

ASTNode * parse_factor(Parser *p)
{
    ParseFunction parses[] = {
        parse_func_exec,
        parse_string,
        parse_number,
        parse_name,
    };

    return try_parses(p, parses, parses_count(parses));
}

int is_operator(Token *token)
{
    return token->type == TOKEN_MINUS ||
           token->type == TOKEN_PLUS  ||
           token->type == TOKEN_STAR  ||
           token->type == TOKEN_SLASH;
}

int is_bool_operator(Token *token)
{
    return token->type == TOKEN_GREATER        ||
           token->type == TOKEN_GREATER_EQUALS ||
           token->type == TOKEN_LESS           ||
           token->type == TOKEN_LESS_EQUALS    ||
           token->type == TOKEN_EQUALS         ||
           token->type == TOKEN_AND            ||
           token->type == TOKEN_OR;
}

ASTNode * parse_expression(Parser *p)
{
    Token *token = parser_peek(p, 0);

    ASTNode *factor = parse_factor(p);

    if (factor == NULL) {
        parser_set_error(p, 0, "Factor needed on expression", token, 0);
        return NULL;
    }

    token = parser_peek(p, 0);

    if (!is_operator(token) && !is_bool_operator(token)) return factor;
    parser_advance(p, 1); // operator

    ASTNode *expression = create_ast_node(AST_EXPRESSION);
    expression->left = factor;
    expression->token = token;
    expression->right = parse_expression(p);

    return expression;
}

int is_assign(Token *token)
{
    return token->type == TOKEN_ASSIGN       ||
           token->type == TOKEN_PLUS_ASSIGN  ||
           token->type == TOKEN_MINUS_ASSIGN ||
           token->type == TOKEN_STAR_ASSIGN  ||
           token->type == TOKEN_SLASH_ASSIGN;
}

ASTNode * parse_var_def(Parser *p)
{
    if(!is_type(p, 0)) return NULL;

    ASTNode *node = create_ast_node(AST_VAR_DEF);
    node->left = parse_type(p);

    Token *token = parser_peek(p, 0);
    if(!is_name(token)) {
        parser_set_error(p, 1.0f/4.0f, "Name needed to define a variable", token, 0);
        return NULL;
    }

    node->token = token;
    parser_advance(p, 1); // name

    token = parser_peek(p, 0);
    if (!is_assign(token)) return node;

    if (token->type != TOKEN_ASSIGN) 
        parser_set_error_and_abort(p, 2.0f/4.0f,
                "Assign operator not allowed to define a variable", token);
    
    parser_advance(p, 1); // =

    token = parser_peek(p, 0);
    node->right = parse_expression(p);
    
    if (node->right == NULL)
        parser_set_error_and_abort(p, 3.0f/4.0f,
                "Expression needed to initializa a variable", token);

    return node;
}

int is_const_def(Parser *p)
{
    if(parser_peek(p, 0)->type != TOKEN_CONST) return 0;
    return 1;
}

ASTNode * parse_const_def(Parser *p)
{
    if (!is_const_def(p)) return NULL;

    parser_advance(p, 1); // const

    if(!is_type(p, 0))
        parser_set_error_and_abort(p, 1.0f/5.0f, "Type needed to define a const", parser_peek(p, 1));

    ASTNode *node = create_ast_node(AST_CONST_DEF);
    node->left = parse_type(p);

    Token *token = parser_peek(p, 0);
    if(!is_name(token))
        parser_set_error_and_abort(p, 2.0f/5.0f, "Name needed to define a const",
                token);

    node->token = token;
    parser_advance(p, 1); // name

    token = parser_peek(p, 0);
    if (!is_assign(token)) {
        parser_set_error_and_abort(p, 3.0f/5.0f, "Assign is needed to define a const",
                token);
    };

    if (token->type != TOKEN_ASSIGN) 
        parser_set_error_and_abort(p, 3.0f/5.0f,
                "Assign operator not allowed to define a const", token);
    
    parser_advance(p, 1); // =
    
    token = parser_peek(p, 0);
    node->right = parse_factor(p);

    if (node->right == NULL)
        parser_set_error_and_abort(p, 4.0f/4.0f, "Expression needed to define a const",
                token);

    return node;
}

ASTNode * parse_assignment(Parser *p)
{
    ASTNode *name = parse_name(p);

    if (name == NULL) return NULL;

    Token *token = parser_peek(p, 0);
    if (!is_assign(token)) {
        parser_set_error(p, 1.0f/3.0f, "Assign needed on assignment", token, 0);
        return NULL;
    }

    parser_advance(p, 1); // assign

    ASTNode *node = create_ast_node(AST_ASSIGNMENT);
    node->token = token;

    token = parser_peek(p, 0);

    ASTNode *expression = parse_expression(p);
    if (expression == NULL)
        parser_set_error_and_abort(p, 2.0f/3.0f, "Expression needed on assignment",
                token);

    node->left = name;
    node->right = expression;

    return node;
}

ASTNode * parse_break(Parser *p)
{
    Token *token = parser_peek(p, 0);

    if (token->type != TOKEN_BREAK) return NULL;
    parser_advance(p, 1); // return

    ASTNode *node = create_ast_node(AST_BREAK);
    node->token = token;

    return node;
}

ASTNode * parse_return(Parser *p)
{
    Token *token = parser_peek(p, 0);

    if (token->type != TOKEN_RETURN) return NULL;
    parser_advance(p, 1); // return

    ASTNode *node = create_ast_node(AST_RETURN);
    node->token = token;

    Token *next = parser_peek(p, 0);           
    if (next->type != TOKEN_SEMICOLON)         
        node->right = parse_expression(p);     

    return node;
}

ASTNode * parse_command(Parser *p)
{
    ParseFunction parses[] = {
        parse_var_def,
        parse_const_def,
        parse_break,
        parse_return,
        parse_assignment,
        parse_factor,
    };

    ASTNode *n = try_parses(p, parses, parses_count(parses));

    if (n == NULL) {
        parser_report_error(p);
        exit(1);
    }

    parser_match(p, TOKEN_SEMICOLON, "';' needed to end a command");

    ASTNode *node = create_ast_node(AST_COMMAND);
    node->left = n;

    return node;
}

ASTNode * parse_func_def_param(Parser *p)
{
    ASTNode *type = parse_type(p);

    if (type == NULL)
        parser_set_error_and_abort(p, 0, "Type needed on parameter definition",
                parser_peek(p, 0));

    ASTNode *name = parse_name(p);

    if (name == NULL)
        parser_set_error_and_abort(p, 0, "Name needed on parameter definition",
                parser_peek(p, 0));

    ASTNode *node = create_ast_node(AST_FUNC_DEF_PARAM);
    node->left = type;
    node->token = name->token;

    return node;
}

ArrayList * parse_func_def_params(Parser *p)
{
    Token *token = parser_peek(p, 0);
    if (token->type != TOKEN_LPAREN) return NULL;

    parser_advance(p, 1); // (

    ArrayList *params = array_list_create(sizeof(ASTNode), 1);

    while (parser_peek(p, 0)->type != TOKEN_RPAREN) {
        array_list_add(params, parse_func_def_param(p));

        token = parser_peek(p, 0);
        if (token->type == TOKEN_COMMA) {
            parser_advance(p, 1); // ,
            continue;
        }

        if (token->type != TOKEN_RPAREN)
            parser_set_error_and_abort(p, 2.0f/3.0f,
                    "')' needed to end parameters definition", token);
    }

    parser_advance(p, 1); // )
    
    return params;
}

ASTNode * parse_func_def(Parser *p)
{
    ASTNode *type = parse_type(p);

    if (type == NULL) return NULL;

    ASTNode *name = parse_name(p);

    if (name == NULL) return NULL;

    ArrayList *params = parse_func_def_params(p);

    if (params == NULL) return NULL;

    ASTNode *node = create_ast_node(AST_FUNC_DEF);
    node->left = type;
    node->token = name->token;
    node->children = params;
    node->right = parse_block(p);

    return node;

}

ASTNode * parse_func_exec_param(Parser *p)
{
    ASTNode *node = create_ast_node(AST_FUNC_EXEC_PARAM);
    node->right = parse_expression(p);

    return node;
}

ArrayList * parse_func_exec_params(Parser *p)
{
    Token *token = parser_peek(p, 0);
    if (token->type != TOKEN_LPAREN) return NULL;

    parser_advance(p, 1); // (

    ArrayList *params = array_list_create(sizeof(ASTNode), 1);

    while (parser_peek(p, 0)->type != TOKEN_RPAREN) {
        array_list_add(params, parse_func_exec_param(p));

        token = parser_peek(p, 0);
        if (token->type == TOKEN_COMMA) {
            parser_advance(p, 1); // ,
            continue;
        }

        if (token->type != TOKEN_RPAREN)
            parser_set_error_and_abort(p, 2.0f/3.0f,
                    "')' needed to end parameters execution", token);
    }

    parser_advance(p, 1); // )
    
    return params;
}

ASTNode * parse_func_exec(Parser *p)
{
    ASTNode *func_name = parse_name(p);

    if (func_name == NULL) return NULL;

    ArrayList *params = parse_func_exec_params(p);

    if (params == NULL) return NULL;

    ASTNode *node = create_ast_node(AST_FUNC_EXEC);
    node->token = func_name->token;
    node->children = params;

    return node;
}

ASTNode * parse_if(Parser *p);

ASTNode * parse_else(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_ELSE) return NULL;
    parser_advance(p, 1); // else

    ASTNode *node = create_ast_node(AST_ELSE);

    if (parser_peek(p, 0)->type == TOKEN_IF) {
        node->right = parse_if(p);
        return node;
    }

    node->right = parse_block(p);

    return node;
}

ASTNode * parse_if(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_IF) return NULL;
    parser_advance(p, 1); // if

    parser_match(p, TOKEN_LPAREN, "'(' needed in if condition");

    ASTNode *node = create_ast_node(AST_IF);
    node->left = parse_expression(p);
    if (node->left == NULL) {
        parser_set_error_and_abort(p, 2.0/5.0, "if needs a condition", parser_peek(p, 0));
    }

    parser_match(p, TOKEN_RPAREN, "')' needed in if condition");

    node->right = parse_block(p);

    return node;
}

ASTNode * parse_while(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_WHILE) return NULL;
    parser_advance(p, 1); // while

    parser_match(p, TOKEN_LPAREN, "'(' needed in while condition");

    ASTNode *node = create_ast_node(AST_WHILE);
    node->left = parse_expression(p);
    if (node->left == NULL)
        parser_set_error_and_abort(p, 2.0/5.0, "while needs a condition", parser_peek(p, 0));

    parser_match(p, TOKEN_RPAREN, "')' needed in while condition");

    node->right = parse_block(p);

    return node;
}

ASTNode * parse_for(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_FOR) return NULL;
    parser_advance(p, 1); // for

    parser_match(p, TOKEN_LPAREN, "'(' needed in for");

    ASTNode *node = create_ast_node(AST_FOR);
    node->children = array_list_create(sizeof(ASTNode), 3);

    ParseFunction parses[] = {
        parse_var_def,
        parse_const_def,
        parse_assignment,
        parse_factor,
    };

    for (int i = 0; i < 3; i++) {
        if (parser_peek(p, 0)->type == TOKEN_SEMICOLON) {
            array_list_add(node->children, create_ast_node(AST_EMPTY));
            parser_advance(p, 1); // semicolon
            continue;
        }

        if (i == 1) {
            array_list_add(node->children, parse_expression(p));
        } else {
            ASTNode *n = try_parses(p, parses, parses_count(parses));

            if (n == NULL) n = create_ast_node(AST_EMPTY);

            array_list_add(node->children, n);
        }

        if (i < 2)
            parser_match(p, TOKEN_SEMICOLON,
                    i == 0 ? "condition needed in for" : "expression expected in for");
    }

    parser_match(p, TOKEN_RPAREN, "')' needed in for");

    node->right = parse_block(p);

    return node;
}

ASTNode * parse_statement(Parser *p)
{
    ParseFunction parses[] = {
        parse_func_def,
        parse_if,
        parse_else,
        parse_while,
        parse_for,
        parse_command
    };

    ASTNode *node = try_parses(p, parses, parses_count(parses));

    if (node == NULL) {
        parser_report_error(p);
        exit(1);
    }

    return node;
}

ASTNode * parse_block(Parser *p)
{
    ParseFunction parses[] = {
        parse_statement
    };

    int is_root = p->level == 0;
    if (!is_root) parser_match(p, TOKEN_LBRACE, "'{' needed start a block");

    ASTNode *block = create_ast_node(AST_BLOCK);
    block->children = array_list_create(sizeof(ASTNode), 1);

    p->level++;

    while (parser_peek(p, 0)->type != (is_root ? TOKEN_EOF : TOKEN_RBRACE)) {
        ASTNode *node = try_parses(p, parses, parses_count(parses));

        if (node == NULL) {
            parser_report_error(p);
            exit(1);
        }

        array_list_add(block->children, node);
    }

    if (!is_root) parser_match(p, TOKEN_RBRACE, "'}' needed end a block");

    p->level--;

    return block;
}

ASTNode * parse(ArrayList *list, char *source, char *file_name)
{
    Parser p = {0}; 
    p.list = list;
    p.source = source;
    p.file_name = file_name;
    p.error_info.progress = -1;

    ASTNode *ast = parse_block(&p);

    Token *t = parser_peek(&p, 0);
    if (t->type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t->text);
        exit(1);
    }

    return ast;
}

