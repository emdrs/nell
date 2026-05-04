#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef AST * (*ParseFunction)(Parser *);

void print_indent(int level) { for (int i = 0; i < level; i++) printf("  "); }

void show_ast(AST* node, int indent)
{
    if (node == NULL) return;

    print_indent(indent);

    switch (node->type) {
        case AST_VAR_DEF: {
            printf("VAR_DEF(%s:%s)\n", node->var_def.name, node->var_def.type);
            break;
        }
        case AST_CONST_DEF: {
            printf("CONST_DEF(%s:%s)\n", node->const_def.name, node->const_def.type);
            show_ast(node->const_def.value, indent + 1);
            break;
        }
        case AST_NUMBER: {
            printf("NUMBER(%s)\n", node->number.value);
            break;
        }
        case AST_ASSIGN: {
            printf("ASSIGN(%s)\n", node->assign.type);
            show_ast(node->assign.left, indent + 1);
            show_ast(node->assign.right, indent + 1);
            break;
        }
        case AST_NAME: {
            printf("NAME(%s)\n", node->name);
            break;
        }
        case AST_EXPRESSION: {
            printf("EXPRESSION(%s)\n", node->expression.type);
            show_ast(node->expression.left, indent + 1);
            show_ast(node->expression.right, indent + 1);
            break;
        }
        case AST_BLOCK: {
            printf("BLOCK(%d)\n", node->block.size);
            for (int i = 0; i < node->block.size; i++)
                show_ast(node->block.statements[i], indent + 1);
            break;
        }
        case AST_UPDATE_IDENTIFIER: {
            printf("UPDATE_IDENTIFIER(%s) %s\n",
                   node->update_identifier.is_increment ? "++" : "--",
                   node->update_identifier.is_prefix ? "PRE" : "POST");
            show_ast(node->update_identifier.target, indent + 1);
            break;
        }
        case AST_COMMAND: {
            printf("COMMAND\n");
            show_ast(node->command, indent + 1);

            break;
        }
        case AST_IF: {
            printf("IF\n");
            show_ast(node->if_statement.expression, indent + 1);
            show_ast(node->if_statement.if_block, indent + 1);

            if (node->if_statement.else_block == NULL) return;

            printf("ELSE\n");
            show_ast(node->if_statement.else_block, indent + 1);
            break;
        }
        case AST_WHILE: {
            printf("WHILE\n");
            show_ast(node->while_statement.expression, indent + 1);
            show_ast(node->while_statement.block, indent + 1);
            break;
        }
        case AST_FOR: {
            printf("FOR\n");
            show_ast(node->for_statement.start, indent + 1);
            show_ast(node->for_statement.end, indent + 1);
            show_ast(node->for_statement.block, indent + 1);
            break;
        }
        case AST_FUNC_DEF: {
            printf("FUNC_DEF(%s -> %s)\n", node->func_def.name,
                    node->func_def.return_type);
            print_indent(indent + 1);
            printf("PARAMS:\n");
            for (int i = 0; i < node->func_def.size; i++)
                show_ast(node->func_def.params[i], indent + 2);
            show_ast(node->func_def.block, indent + 1);
            break;
        }
        case AST_RETURN: {
            printf("RETURN\n");
            show_ast(node->return_statement, indent + 1);
            break;
        }
        case AST_FUNC_DEF_PARAM: {
            printf("[%s: %s]\n", node->func_def_param.name, node->func_def_param.type);
            break;
        }
        case AST_FUNC_EXEC: {
            printf("FUNC_EXEC(%s)\n", node->func_exec.name);
            for (int i = 0; i < node->func_exec.params->size; i++) {
                AST *param = (AST *)array_list_get(node->func_exec.params, i);
                printf("");
                show_ast(param, indent + 1);
            }
            break;
        }
        case AST_STRING: {
            printf("STRING(%s)\n", node->str);
            break;
        }
        case AST_SWITCH: {
            printf("SWITCH(%s)\n", node->switch_statement.value->name);
            show_ast(node->switch_statement.block, indent + 1);
            break;
        }
        case AST_CASE: {
            if (node->case_statement.is_default) {
                printf("DEFAULT\n");
            } else  {
                printf("CASE\n");
                show_ast(node->case_statement.value, indent + 1);
            }
            show_ast(node->case_statement.block, indent + 1);
            break;
        }
        case AST_BREAK: {
            printf("BREAK\n");
            break;
        }
    }
}

int is_var_def(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_IDENTIFIER) return 0;
    Token *token;
    if ((token = parser_peek(p, 1))->type != TOKEN_IDENTIFIER) {
        parser_set_error(p, 1.0f/2.0f, "Name is needed in var definition", token, 0);
        return 0;
    }

    return 1;
}

AST * parse_var_def(Parser *p)
{
    if(!is_var_def(p)) return NULL;

    AST *node = create_ast_node(AST_VAR_DEF);
    node->var_def.initialized = 0;
    node->var_def.type = parser_peek(p, 0)->text;
    node->var_def.name = parser_peek(p, 1)->text;
    parser_advance(p, 2);

    return node;
}

int is_identifier_updater(Token *token)
{
    return token->type == TOKEN_INCREMENT || token->type == TOKEN_DECREMENT;
}

int is_identifier_update(Parser *p, int offset)
{
    Token *token1 = parser_peek(p, offset);
    Token *token2 = parser_peek(p, offset + 1);

    if (is_identifier_updater(token1) && token2->type != TOKEN_IDENTIFIER) {
        parser_report_error(p, token2,"Name needed after/before updater.");
        exit(1);
    } else if (is_identifier_updater(token2)) {
        parser_match(p, TOKEN_IDENTIFIER, "Name needed after/before updater.");
    }

    return (is_identifier_updater(token1) && is_name(token2)) ||
           (is_identifier_updater(token2) && is_name(token1));
}

AST * parse_identifier_update(Parser *p)
{
    if (!is_identifier_update(p, 0)) return NULL;

    AST *update = create_ast_node(AST_UPDATE_IDENTIFIER);

    int is_prefix = is_identifier_updater(parser_peek(p, 0));
    update->update_identifier.is_prefix = is_prefix;

    if (is_prefix) parser_advance(p, 1); // -- or ++

    update->update_identifier.target = parse_name(p);
    update->update_identifier.is_increment =
        parser_peek(p, is_prefix)->type == TOKEN_INCREMENT;

    if (!is_prefix) parser_advance(p, 1); // -- or ++

    return update;
}

int is_factor(Parser *p, int offset)
{
    Token *token = parser_peek(p, offset);

    if(token->type == TOKEN_LPAREN) return is_factor(p, offset + 1);

    return (is_name(token)          ||
            is_number(token)        ||
            is_string(token)        ||
            is_identifier_update(p, offset) ||
            is_func_exec(p));
}

AST * parse_factor(Parser *p)
{
    Token *token = parser_peek(p, 0);

    if (is_identifier_update(p, 0)) return parse_identifier_update(p);
    if (is_number(token))           return parse_number(p);
    if (is_func_exec(p))            return parse_func_exec(p);
    if (is_string(token))           return parse_string(p);
    if (is_name(token))             return parse_name(p);

    parser_set_error_and_abort(p, 0, "Invalid factor", token);
    exit(1);
}

int is_func_exec(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1)->type != TOKEN_LPAREN) return 0;

    return 1;
}

// first needed to prevent exec(a,)
void parse_func_exec_params(Parser *p, ArrayList *param_list, int first)
{
    Token *token = parser_peek(p, 0);
    parser_advance(p, 1); // first ( or ,

    if(!is_expression(p) && p->error_info.progress == 0)
        parser_set_error_and_abort(p, 1, "Expression needed in function execution",
                parser_peek(p, 0));

    array_list_add(param_list, parse_expression(p));

    token = parser_peek(p, 0);
    if (token->type == TOKEN_RPAREN) {
        parser_advance(p, 1); // )
        return;
    }

    parser_match(p, TOKEN_COMMA, "',' needed to split execution parameters");

    parse_func_exec_params(p, param_list, 0);
}

AST * parse_func_exec(Parser *p)
{
    if (!is_func_exec(p)) return NULL;

    AST *node = create_ast_node(AST_FUNC_EXEC);
    node->func_exec.params = array_list_create(sizeof(AST), 1);
    node->func_exec.name = parser_peek(p, 0)->text;
    parser_advance(p, 1); // Identifier

    parse_func_exec_params(p, node->func_exec.params, 1);

    return node;
}

int is_expression(Parser *p) { return is_factor(p, 0); }

int is_operator(Token *token)
{
    if (token->type != TOKEN_PLUS           &&
        token->type != TOKEN_MINUS          &&
        token->type != TOKEN_STAR           &&
        token->type != TOKEN_SLASH          &&
        token->type != TOKEN_GREATER        &&
        token->type != TOKEN_GREATER_EQUALS &&
        token->type != TOKEN_LESS           &&
        token->type != TOKEN_LESS_EQUALS    &&
        token->type != TOKEN_EQUALS         &&
        token->type != TOKEN_AND            &&
        token->type != TOKEN_OR) return 0;

    return 1;
}

AST * parse_expression(Parser *p)
{
    if (parser_peek(p, 0)->type == TOKEN_LPAREN) {
        parser_advance(p, 1); // (
        
        AST *node = parse_expression(p);
        if(node->type == AST_EXPRESSION)
            node->expression.has_paren = 1;

        parser_match(p, TOKEN_RPAREN, "')' needed to close a '('");
        parser_advance(p, 1); // )
        
        return node;
    }

    AST *left = parse_factor(p);

    Token *operator = parser_peek(p, 0);

    if (!is_operator(operator)) return left;
    
    parser_advance(p, 1);

    AST *op = create_ast_node(AST_EXPRESSION);
    op->expression.left = left;
    op->expression.right = parse_expression(p);
    if (op->expression.right == NULL)
        parser_set_error_and_abort(p, 2.0f/3.0f, "Factor expected after operator",
                parser_peek(p, 0));

    op->expression.type = operator->text;
    op->expression.has_paren = 0;

    return op;
}

int is_assign(Token *token)
{
    return token->type == TOKEN_ASSIGN       ||
           token->type == TOKEN_PLUS_ASSIGN  ||
           token->type == TOKEN_MINUS_ASSIGN ||
           token->type == TOKEN_STAR_ASSIGN  ||
           token->type == TOKEN_SLASH_ASSIGN;
}

int is_assignment(Parser *p)
{
    if(is_var_def(p)) {
        if (!is_assign(parser_peek(p, 2))) return 0;
        if (!is_factor(p, 3)) {
            parser_set_error(p, 3.0 / 4.0, "Assignment need a factor.",
                    parser_peek(p, 3), 0);
            return 0;
        }
        return 1;
    } else {
        if (parser_peek(p, 0)->type != TOKEN_IDENTIFIER) return 0;
        if (!is_assign(parser_peek(p, 1))) return 0;
        if (!is_factor(p, 2))
            parser_set_error_and_abort(p, 3.0f/4, "Assignment need a factor",
                    parser_peek(p, 2));
    }

    return 1;
}

AST * parse_assignment(Parser *p)
{
    if (!is_assignment(p)) return NULL;

    AST *node = create_ast_node(AST_ASSIGN);
    if (is_var_def(p)) {
        node->assign.left = parse_var_def(p);
        node->assign.left->var_def.initialized = 1;
    } else {
        node->assign.left = parse_name(p);
    }

    Token *token = parser_peek(p, 0);

    node->assign.type = token->text;
    parser_advance(p, 1);

    node->assign.right = parse_expression(p);

    return node;
}

void push_statement(AST *block, AST *statement)
{
    if (block->block.size >= block->block.capacity) {
        block->block.capacity *= 2;
        block->block.statements =
            realloc(block->block.statements, sizeof(AST *) * block->block.capacity);
    }

    block->block.statements[block->block.size++] = statement;
}

// TODO make this more specific
int is_block(Parser *p)
{
    if (p->level == 0) return 1;

    if (parser_peek(p, 0)->type != TOKEN_LBRACE) return 0;

    return 1;
}

int is_const_def(Parser *p)
{
    if (parser_peek(p, 0)->type != TOKEN_CONST) return 0;

    Token *token;
    if ((token = parser_peek(p, 1))->type != TOKEN_IDENTIFIER)
        parser_set_error_and_abort(p, 1.0f/5, "Type needed to define a const", token);

    if ((token = parser_peek(p, 2))->type != TOKEN_IDENTIFIER)
        parser_set_error_and_abort(p, 2.0f/5, "Type and name needed to define a const",
                token);

    if ((token = parser_peek(p, 3))->type != TOKEN_ASSIGN)
        parser_set_error_and_abort(p, 3.0f/5, "Assign needed to define a const", token);

    token = parser_peek(p, 4);
    if (!is_factor(p, 4))
        parser_set_error_and_abort(p, 4.0f/5, "Value needed to define a const", token);

    return 1;
}

AST * parse_const_def(Parser *p)
{
    if(!is_const_def(p)) return NULL;

    AST *node = create_ast_node(AST_CONST_DEF);
    node->const_def.type = parser_peek(p, 1)->text;
    node->const_def.name = parser_peek(p, 2)->text;
    parser_advance(p, 4); // const type name =

    node->const_def.value = parse_expression(p);

    return node;
}

int is_return(Parser *p)
{
    if(parser_peek(p, 0)->type != TOKEN_RETURN) return 0;

    return 1;
}

AST * parse_return(Parser *p)
{
    if(!is_return(p)) return NULL;

    if (!p->in_func)
        parser_set_error_and_abort(p, 1.0/2.0,
                "return not allowed outside function block", parser_peek(p, 0));

    AST *node = create_ast_node(AST_RETURN);
    parser_advance(p, 1);
    node->return_statement = is_expression(p) ? parse_expression(p) : NULL;

    return node;
}

int is_break(Parser *p)
{
    return parser_peek(p, 0)->type == TOKEN_BREAK;
}

AST * parse_break(Parser *p) {
    if (!is_break(p)) return NULL;

    if (!p->in_case && !p->in_repeat) 
        parser_set_error_and_abort(p, 1,
                "break not allowed outside case or repeat blocks", parser_peek(p, 0));
    
    parser_advance(p, 1);
    return create_ast_node(AST_BREAK); }

int is_default(Parser *p)
{
    return parser_peek(p, 0)->type == TOKEN_DEFAULT;
}

int is_command(Parser *p)
{
    return is_identifier_update(p, 0) ||
           is_assignment(p)        ||
           is_var_def(p)           ||
           is_const_def(p)         ||
           is_return(p)            ||
           is_func_exec(p)         ||
           is_break(p);
}

AST * parse_command(Parser *p)
{
    AST *node = NULL;
    ParseFunction functions[] = {
        parse_identifier_update,
        parse_assignment,
        parse_var_def,
        parse_const_def,
        parse_return,
        parse_break,
        parse_func_exec
    };

    for (int i = 0; node == NULL && i < sizeof(functions) / sizeof(ParseFunction); i++)
        node = functions[i](p);

    if (node == NULL) return NULL;

    parser_match(p, TOKEN_SEMICOLON, "; expected to define a command");
    parser_advance(p, 1);

    AST *command = create_ast_node(AST_COMMAND);
    command->command = node;

    return command;
}

AST * parse_if(Parser *p, int level)
{
    AST *node = create_ast_node(AST_IF);
    parser_advance(p, 1);
    node->if_statement.expression = parse_expression(p);
    node->if_statement.if_block = parse_block(p);

    return node;
}

AST * parse_case(Parser *p, int level)
{
    AST *node = create_ast_node(AST_CASE);
    node->case_statement.is_default = parser_peek(p, 0)->type == TOKEN_DEFAULT;
    parser_advance(p, 1);
    Token *token = parser_peek(p, 0);
    if (!is_expression(p))
        parser_report_error(p, token, "Expression needed on case value");
    if (node->case_statement.is_default)
        node->case_statement.value = NULL;
    else
        node->case_statement.value = parse_expression(p);
    node->case_statement.block = parse_block(p);

    return node;
}

AST * parse_switch(Parser *p, int level)
{
    AST *node = create_ast_node(AST_SWITCH);
    parser_advance(p, 1);
    Token *token = parser_peek(p, 0);
    if (token->type != TOKEN_IDENTIFIER)
        parser_report_error(p, token, "Identifier needed on switch value");
    node->switch_statement.value = parse_name(p);
    node->switch_statement.block = parse_block(p);

    return node;
}

AST * parse_while(Parser *p, int level)
{
    AST *node = create_ast_node(AST_WHILE);
    parser_advance(p, 1);
    node->while_statement.expression = parse_expression(p);
    node->while_statement.block = parse_block(p);

    return node;
}

AST * parse_for(Parser *p, int level)
{
    AST *node = create_ast_node(AST_FOR);
    parser_advance(p, 1);

    if (!is_expression(p)) {
         parser_report_error(p, parser_peek(p, 0), "Need an expression in for start range");
        exit(0);
    }
    node->for_statement.start = parse_expression(p);

    Token *token = parser_peek(p, 0);
    
    switch (token->type) {
        case TOKEN_DOUBLE_DOT:
            node->for_statement.is_start_inclusive = 1;
            node->for_statement.is_end_inclusive = 1;
            break;
        case TOKEN_GREATER_DOT:
            node->for_statement.is_start_inclusive = 0;
            node->for_statement.is_end_inclusive = 1;
            break;
        case TOKEN_DOT_LESS:
            node->for_statement.is_start_inclusive = 1;
            node->for_statement.is_end_inclusive = 0;
            break;
        case TOKEN_GREATER_LESS:
            node->for_statement.is_start_inclusive = 0;
            node->for_statement.is_end_inclusive = 0;
            break;
        default:
            parser_report_error(p, token, "Need '..|>.|.<|><' to define a for range");
    }

    parser_advance(p, 1);

    node->for_statement.end = parse_expression(p);
    node->for_statement.block = parse_block(p);

    return node;
}

int is_func_def(Parser *p)
{
    if (!is_name(parser_peek(p, 0))) return 0;
    if (!is_name(parser_peek(p, 1))) return 0;
    if (parser_peek(p, 2)->type != TOKEN_LPAREN) return 0;

    return 1;
}

void push_param(AST *func_def, AST *param)
{
    if (func_def->func_def.size >= func_def->func_def.capacity) {
        func_def->func_def.capacity *= 2;
        func_def->func_def.params =
            realloc(func_def->func_def.params,
                    sizeof(AST *) * func_def->func_def.capacity);
    }

    func_def->func_def.params[func_def->func_def.size++] = param;
}

AST * parse_func_def_param(Parser *p)
{
    AST *node = create_ast_node(AST_FUNC_DEF_PARAM);
    node->func_def_param.type = parser_peek(p, 0)->text;
    node->func_def_param.name = parser_peek(p, 1)->text;
    parser_advance(p, 2);

    return node;
}

AST * parse_func_def(Parser *p, int level)
{
    AST *node = create_ast_node(AST_FUNC_DEF);
    node->func_def.params = (AST **) malloc(sizeof(AST *));
    node->func_def.size = 0;
    node->func_def.capacity = 1;
    node->func_def.return_type = parser_peek(p, 0)->text;
    node->func_def.name = parser_peek(p, 1)->text;
    parser_advance(p, 3);

    while (parser_peek(p, 0)->type != TOKEN_RPAREN) {
        push_param(node, parse_func_def_param(p));
        if (parser_peek(p, 0)->type == TOKEN_COMMA) parser_advance(p, 1);
    }

    parser_advance(p, 1);
    node->func_def.block = parse_block(p);

    return node;
}


AST * parse_statement(Parser *p, int level)
{
    AST *node = NULL;
    ParseFunction functions[] = {
        parse_command
    };

    for (int i = 0; node == NULL && i < sizeof(functions) / sizeof(ParseFunction); i++)
        node = functions[i](p);

    if (node == NULL) {
        parser_show_error(p);
        exit(1);
    }

    return node;

    if (is_func_def(p)) {
        node = parse_func_def(p,  level + 1);
        parser_advance(p, 1);
    } else if (is_command(p)) {
        node = parse_command(p);
    } else if (is_block(p)) {
        node = parse_block(p);
        parser_match(p, TOKEN_RBRACE, "} expected to define a block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0)->type == TOKEN_IF) {
        node = parse_if(p,  level + 1);
        parser_match(p, TOKEN_RBRACE, "} expected to define a if block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0)->type == TOKEN_SWITCH) {
        node = parse_switch(p,  level + 1);
        parser_match(p, TOKEN_RBRACE, "} expected to define a switch block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0)->type == TOKEN_CASE || is_default(p)) {
        node = parse_case(p,  level + 1);
        parser_match(p, TOKEN_RBRACE, "} expected to define a case block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0)->type == TOKEN_WHILE) {
        node = parse_while(p,  level + 1);
        parser_match(p, TOKEN_RBRACE, "} expected to define a while block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0)->type == TOKEN_FOR) {
        node = parse_for(p,  level + 1);
        parser_match(p, TOKEN_RBRACE, "} expected to define a for block");
        parser_advance(p, 1);
    } else {
        parser_show_error(p);
        exit(1);
    }

    return node;
}

AST * parse_block(Parser *p)
{
    int level = p->level;
    AST *block = create_ast_node(AST_BLOCK);
    block->block.statements = (AST **) malloc(sizeof(AST *));
    block->block.size = 0;
    block->block.capacity = 1;
    block->block.level = level;

    if (level > 0) parser_advance(p, 1);
    p->level++;

    while (parser_peek(p, 0)->type != ((level > 0) ? TOKEN_RBRACE : TOKEN_EOF)) {
        p->error_info.progress = -1;
        push_statement(block, parse_statement(p, level));
    }
    p->level--;

    return block;
}

AST * parse(ArrayList *list, char *source, char *file)
{
    Parser p = { list, 0, source, file, 0, 0, 0, 0 };

    p.error_info.progress = -1;

    AST *ast = parse_block(&p);

    Token *t = parser_peek(&p, 0);
    if (t->type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t->text);
        exit(1);
    }

    return ast;
}

