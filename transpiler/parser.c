#include "parser.h"
#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>


void print_indent(int level)
{
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 espaços
    }
}

void show_ast(AST* node, int indent)
{
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case AST_BLOCK:
            printf("BLOCK(%d)\n", node->block.count);
            for (int i = 0; i < node->block.count; i++) {
                show_ast(node->block.statements[i], indent + 1);
            }
            break;

        case AST_FUNC_DEF:
            printf("FUNCTION_DEFINITION(%s -> %s)\n", 
                    node->func_def.name, node->func_def.return_type);
            
            print_indent(indent + 1);
            printf("PARAMS: ");
            AST *def_param = node->func_def.params;
            while (def_param) {
                show_ast(def_param, 0);
                def_param = def_param->func_def_param.next;
            }
            printf("\n");

            show_ast(node->func_def.body, indent + 1);
            break;

        case AST_FUNC_DEF_PARAM:
            printf("[%s : %s] ", node->func_def_param.name, node->func_def_param.type);
            break;

        case AST_FUNC_EXEC:
            printf("FUNCTION_EXECUTION(%s)\n", node->func_exec.name);
            
            AST *exec_param = node->func_exec.params;
            while (exec_param) {
                show_ast(exec_param->func_exec_param.expression, indent + 1);
                exec_param = exec_param->func_exec_param.next;
            }
            break;

        case AST_FUNC_EXEC_PARAM:
            printf("PARAM(%s)\n", node->func_exec_param.name);
            show_ast(node->func_exec_param.expression, indent + 1);
            break;

        case AST_FUNC_RETURN:
            printf("FUNC_RETURN\n");
            show_ast(node->func_return, indent + 1);
            break;

        case AST_ASSIGN:
            printf("ASSIGN(%s)\n", node->assign.assignment);
            show_ast(node->assign.to, indent + 1);
            show_ast(node->assign.value, indent + 1);
            break;

        case AST_OPERATOR:
            printf("OPERATOR(%c)\n", node->op.sign);
            show_ast(node->op.left, indent + 1);
            show_ast(node->op.right, indent + 1);
            break;

        case AST_NUMBER:
            printf("NUMBER(%d)\n", node->number);
            break;

        case AST_IDENTIFIER:
            printf("IDENTIFIER(%s)\n", node->identifier);
            break;

        case AST_VAR_DEF:
            printf("VAR_DEF(%s:%s)\n", node->field.name, node->field.type);
            if (node->field.value != NULL) show_ast(node->field.value, indent + 1);
            break;

        case AST_CONST_DEF:
            printf("CONST_DEF(%s:%s)\n", node->field.name, node->field.type);
            show_ast(node->field.value, indent + 1);
            break;
    }
}

void advance_parser(Parser* p, unsigned int amount) { p->pos += amount; }

Token parser_peek(Parser* p, int offset) { return p->list.data[p->pos + offset]; }

void match(Parser* p, TokenType type, const char* error_msg) {
    Token token = parser_peek(p, 0);
    if (token.type != type) {
        fprintf(stderr, "Erro de Sintaxe: %s (Found: '%s')\n", 
                error_msg, token.text);
        exit(1);
    }
}

AST * create_number(Token token)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_NUMBER;
    node->number = atoi(token.text);
    return node;
}

AST * create_operator(Token token, AST *left, AST *right)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_OPERATOR;
    node->op.sign = token.text[0];
    node->op.left = left;
    node->op.right = right;
    return node;
}

AST * create_identifier(Token token)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_IDENTIFIER;
    node->identifier = token.text;
    return node;
}

AST * create_assignment(Token to, AST *value, Token sign)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_ASSIGN;
    node->assign.assignment = sign.text;
    node->assign.to = create_identifier(to);
    node->assign.value = value;

    return node;
}

AST * create_block(int main)
{
    AST* node = malloc(sizeof(AST));
    node->type = AST_BLOCK;
    node->block.count = 0;
    node->block.main = main;
    node->block.capacity = 1;
    node->block.statements = malloc(sizeof(AST*) * node->block.capacity);
    return node;
}

void add_to_block(AST* block, AST* stmt)
{
    if (block->block.count >= block->block.capacity) {
        block->block.capacity *= 2;
        block->block.statements = realloc(block->block.statements, sizeof(AST*) * block->block.capacity);
    }
    block->block.statements[block->block.count++] = stmt;
}

AST * create_func_exec(char *func_name, AST *params)
{
    AST* node = malloc(sizeof(AST));
    node->type = AST_FUNC_EXEC;
    node->func_exec.name= func_name;
    node->func_exec.params = params;
    
    int count = 0;
    while (params != NULL) {
        count++;
        params = params->func_exec_param.next;
    }
    node->func_exec.param_count = count;
    return node;
}

void create_exec_param(AST **params, char *name, AST *expression)
{
    AST *parameter = (AST*) malloc(sizeof(AST));

    parameter->type = AST_FUNC_EXEC_PARAM;
    parameter->func_exec_param.expression = expression;
    parameter->func_exec_param.next = NULL;

    if (*params == NULL) {
        *params = parameter;
        return;
    }

    AST *p;
    for (p = *params;p->func_exec_param.next != NULL;p = p->func_exec_param.next);
    p->func_exec_param.next = parameter;
}

AST * parse_expression(Parser *p);

AST * parse_func_exec(Parser* p)
{
    Token func_name = parser_peek(p, 0);
    advance_parser(p, 2); // identifier, lparen

    AST *params = NULL;
    if (parser_peek(p, 0).type != TOKEN_RPAREN) {
        while (1) {
            create_exec_param(&params, parser_peek(p, 0).text, parse_expression(p));
            if (parser_peek(p, 0).type != TOKEN_COMMA) break;

            advance_parser(p, 1);
        }

    }

    match(p, TOKEN_RPAREN, ") needed to end a function execution");
    advance_parser(p, 1);

    return create_func_exec(func_name.text, params);
}

AST * parse_factor(Parser* p)
{
    Token token = parser_peek(p, 0);

    if (token.type == TOKEN_IDENTIFIER && parser_peek(p, 1).type == TOKEN_LPAREN)
        return parse_func_exec(p);

    advance_parser(p, 1);
    if (token.type == TOKEN_NUMBER) return create_number(token);
    if (token.type == TOKEN_IDENTIFIER) return create_identifier(token);

    fprintf(stderr, "Erro: esperado número ou variável, mas encontrou '%s'\n",
            token.text);
    exit(1);
}

AST * parse_expression(Parser *p)
{
    AST* left = parse_factor(p);

    while (1) {
        Token t = parser_peek(p, 0);

        if (t.type == TOKEN_PLUS  ||
            t.type == TOKEN_MINUS ||
            t.type == TOKEN_STAR  ||
            t.type == TOKEN_SLASH) {
            advance_parser(p, 1);

            AST* right = parse_factor(p);

            left = create_operator(t, left, right);
        } else { break; }
    }
    return left;
}

AST * parse_assignment(Parser *p)
{
    Token to   = parser_peek(p, 0);
    Token sign = parser_peek(p, 1);

    AST *node = malloc(sizeof(AST));
    node->type = AST_ASSIGN;
    node->assign.assignment = sign.text;
    node->assign.to = create_identifier(to);

    advance_parser(p, 2); // Consume identifier and equals
    node->assign.value = parse_expression(p);

    return node;
}

AST * parse_var_def(Parser *p, int explicit_type)
{
    AST *node = malloc(sizeof(AST));
    node->type = AST_VAR_DEF;
    node->field.name = parser_peek(p, 0).text;

    Token token = parser_peek(p, 2);
    if (token.type == TOKEN_IDENTIFIER && parser_peek(p, 3).type == TOKEN_SEMICOLON) {
        advance_parser(p, 3);
        node->field.type = token.text;
        return node;

    }

    node->field.type = (explicit_type) ? token.text : NULL;
    
    // Consume identifier, colon, type(not explicit) and equals
    advance_parser(p, 3 + explicit_type);
    node->field.value = parse_expression(p);

    return node;
}

AST * parse_const_def(Parser *p)
{
    Token name = parser_peek(p, 0);

    AST *node = malloc(sizeof(AST));
    node->type = AST_CONST_DEF;
    node->field.name = name.text;
    node->field.type = NULL;

    advance_parser(p, 2); // Consume identifier and double colon
    node->field.value = parse_expression(p);

    return node;
}

AST * parse_block(Parser *p, int main);

void create_param(AST **params, char *name, char *type)
{
    AST *parameter = (AST*) malloc(sizeof(AST));

    parameter->type = AST_FUNC_DEF_PARAM;
    parameter->func_def_param.name = name;
    parameter->func_def_param.type = type;

    if (*params == NULL) {
        *params = parameter;
        return;
    }

    AST *p;
    for (p = *params;p->func_def_param.next != NULL;p = p->func_def_param.next);
    p->func_def_param.next = parameter;
}

int is_func_params(Parser *p, int offset)
{
    if (parser_peek(p, offset).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, offset + 1).type != TOKEN_COLON) return 0;
    if (parser_peek(p, offset + 2).type != TOKEN_IDENTIFIER) return 0;

    return 1;
}

void parse_func_params(Parser *p, AST *node)
{
    AST *params = NULL;
    int count = 0;

    if (is_func_params(p, 0)) {
        while (1) {
            count++;
            create_param(&params, parser_peek(p, 0).text, parser_peek(p, 2).text);
            advance_parser(p, 3);

            if (parser_peek(p, 0).type != TOKEN_COMMA) break;
            advance_parser(p, 1);
        }
    }

    node->func_def.params = params;
    node->func_def.param_count = count;
}

AST * parse_func_def(Parser *p)
{
    Token name = parser_peek(p, 0);

    AST *node = malloc(sizeof(AST));
    node->type = AST_FUNC_DEF;

    node->func_def.name = name.text;
    advance_parser(p, 3); // Identifier, Double colon, Lparen
    parse_func_params(p, node);

    advance_parser(p, 2); // Rparen, Arrow
    Token return_type = parser_peek(p, 0);
    node->func_def.return_type = return_type.text;

    advance_parser(p, 2); // Return type, Lbrace
    node->func_def.body = parse_block(p, 0);

    return node;
}

int is_var_def(Parser *p, int *explicit_type)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_COLON) return 0;

    *explicit_type = parser_peek(p, 2).type == TOKEN_IDENTIFIER;

    if (*explicit_type && parser_peek(p, 3).type == TOKEN_SEMICOLON) return 1;

    if (parser_peek(p, 2 + (*explicit_type)).type != TOKEN_EQUALS) return 0;

    if (parser_peek(p, 3 + (*explicit_type)).type != TOKEN_IDENTIFIER &&
        parser_peek(p, 3 + (*explicit_type)).type != TOKEN_NUMBER) return 0;

    return 1;
}

int is_const_def(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_DOUBLE_COLON) return 0;

    if (parser_peek(p, 2).type != TOKEN_IDENTIFIER &&
        parser_peek(p, 2).type != TOKEN_NUMBER) return 0;

    return 1;
}

int is_assignment(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_EQUALS) return 0;

    if (parser_peek(p, 2).type != TOKEN_IDENTIFIER &&
        parser_peek(p, 2).type != TOKEN_NUMBER) return 0;

    return 1;
}

int is_func_def(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_DOUBLE_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_LPAREN) return 0;

    if (is_func_params(p, 3)) return 1;

    if (parser_peek(p, 3).type != TOKEN_RPAREN) return 0;
    if (parser_peek(p, 4).type != TOKEN_ARROW) return 0;
    if (parser_peek(p, 5).type != TOKEN_IDENTIFIER) return 0;

    if (parser_peek(p, 6).type != TOKEN_LBRACE) return 0;

    return 1;
}

AST * parse_return(Parser *p) {
    AST* node = malloc(sizeof(AST));
    node->type = AST_FUNC_RETURN;

    advance_parser(p, 1);

    node->func_return = parse_expression(p);
    return node;
}

AST * parse_statement(Parser* p)
{
    Token t = parser_peek(p, 0);

    AST *node;
    int explicit_type;
    if (is_var_def(p, &explicit_type)) {
        node = parse_var_def(p, explicit_type);
        match(p, TOKEN_SEMICOLON, "; needed to end a command");
        advance_parser(p, 1);
    } else if (is_const_def(p)) {
        node = parse_const_def(p);
        match(p, TOKEN_SEMICOLON, "; needed to end a command");
        advance_parser(p, 1);
    } else if (is_assignment(p)) {
        node = parse_assignment(p);
        match(p, TOKEN_SEMICOLON, "; needed to end a command");
        advance_parser(p, 1);
    } else if (is_func_def(p)) {
        node = parse_func_def(p);
    } else if (parser_peek(p, 0).type == TOKEN_RETURN) {
        node = parse_return(p);
        match(p, TOKEN_SEMICOLON, "; needed to end a command");
        advance_parser(p, 1);
    } else {
        printf("Unexpected token: '%s'\n", t.text); 
        exit(1);
    }

    return node;
}

int is_block_end(Parser *p, int is_main)
{
    return parser_peek(p, 0).type == ((is_main) ? TOKEN_EOF : TOKEN_RBRACE);
}

AST * parse_block(Parser* p, int main)
{
    AST* block = create_block(main);

    while (!is_block_end(p, main))
        add_to_block(block, parse_statement(p));

    if(main == 0) advance_parser(p, 1); // Consume }

    return block;
}

AST * parse(TokenList list)
{
    Parser p = { list, 0, false };

    AST* ast = parse_block(&p, 1);

    Token t = parser_peek(&p, 0);
    if (t.type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t.text);
        exit(1);
    }

    return ast;
}
