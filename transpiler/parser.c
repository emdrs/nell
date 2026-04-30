#include "parser.h"
#include "lexer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


AST * create_ast_node(ASTType type)
{
    AST *node = (AST *) malloc(sizeof(AST));
    node->type = type;
    return node;
}

char * get_token_source_line(Parser *p, Token token)
{
    int pos = 0;
    int line = 1;
    char ch;
    while (1) {
        ch = p->source[pos];
        if (ch == '\n') {
            line++;
            pos++;
            continue;
        }

        if (line == token.line) {
            int start = pos;
            while(p->source[pos++] != '\n');
            return strndup(p->source + start, (pos - start) + 1);
        }
        pos++;
    }
}

void print_indent(int level) { for (int i = 0; i < level; i++) printf("  "); }

void show_ast(AST* node, int indent)
{
    if (!node) return;

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
        case AST_IDENTIFIER: {
            printf("IDENTIFIER(%s)\n", node->identifier);
            break;
        }
        case AST_OPERATOR: {
            printf("OPERATOR(%s)\n", node->expression.type);
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
            printf("FUNC_EXEC(%s)", node->func_exec.name);
            for (int i = 0; i < node->func_exec.size; i++)
                show_ast(node->func_exec.params[i], indent + 1);
            break;
        }
    }
}

void parser_advance(Parser *p, int amount) {
    p->pos += amount;

    if (p->pos < p->list.size) return;

    printf("Trying to advance to a token after EOF\n");
    exit(1);
}

Token parser_peek(Parser *p, int offset) {
    if (p->pos + offset < p->list.size) return p->list.data[p->pos + offset];

    printf("Trying to peek a token after EOF\n");
    exit(1);
}

void report_error(Parser *p, Token token, char *error_msg)
{
    printf("%s:%d:%d error: %s\n", p->file, token.line, token.column, error_msg);
    printf("%4d | %s\n", token.line, get_token_source_line(p, token));
    printf("Got: %s\n", token.text);
}

void match(Parser* p, TokenType type, char* error_msg) {
    Token token = parser_peek(p, 0);
    if (token.type != type) {
        report_error(p, token, error_msg);
        exit(1);
    }
}

int is_var_def_explicit(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_IDENTIFIER) return 0;

    return 1;
}

int is_var_def_implicit(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_ASSIGN) return 0;

    return 1;
}

int is_var_def(Parser *p) { return is_var_def_explicit(p) || is_var_def_implicit(p); }

AST * parse_var_def(Parser *p)
{
    AST *node = create_ast_node(AST_VAR_DEF);
    node->var_def.initialized = 0;
    node->var_def.name = parser_peek(p, 0).text;

    if (is_var_def_implicit(p)) {
        node->var_def.type = strdup("int");
        parser_advance(p, 2);
    } else {
        node->var_def.type = parser_peek(p, 2).text;
        parser_advance(p, 3);
    }

    return node;
}

int is_number(Token token)
{
    return token.type == TOKEN_INT || token.type == TOKEN_FLOAT;
}

int is_identifier_updater(Token token)
{
    return token.type == TOKEN_INCREMENT || token.type == TOKEN_DECREMENT;
}

int is_identifier_update(Parser *p)
{
    Token token1 = parser_peek(p, 0);
    Token token2 = parser_peek(p, 1);

    return (is_identifier_updater(token1) && token2.type == TOKEN_IDENTIFIER) ||
           (is_identifier_updater(token2) && token1.type == TOKEN_IDENTIFIER);
}

int is_func_exec(Parser *p);
int is_factor(Parser *p, int offset)
{
    Token token = parser_peek(p, offset);
    return (token.type == TOKEN_IDENTIFIER ||
            is_number(token)               ||
            is_identifier_update(p)        ||
            is_func_exec(p));
}

AST * parse_number(Parser *p)
{
    AST *node = create_ast_node(AST_NUMBER);
    Token number = parser_peek(p, 0);
    node->number.type = strdup(number.type == TOKEN_INT ? "int" : "float");
    node->number.value = number.text;
    parser_advance(p, 1);

    return node;
}

AST * parse_identifier(Parser *p)
{
    AST *node = create_ast_node(AST_IDENTIFIER);
    node->identifier = parser_peek(p, 0).text;
    parser_advance(p, 1);

    return node;
}

AST * parse_identifier_update(Parser *p)
{
    AST *update = create_ast_node(AST_UPDATE_IDENTIFIER);

    int is_prefix = is_identifier_updater(parser_peek(p, 0));
    update->update_identifier.is_prefix = is_prefix;

    if (is_prefix) parser_advance(p, 1); // Consume -- or ++

    update->update_identifier.target = parse_identifier(p);
    update->update_identifier.is_increment =
        parser_peek(p, is_prefix).type == TOKEN_INCREMENT;

    if (!is_prefix) parser_advance(p, 1); // Consume -- or ++

    return update;
}

AST * parse_expression(Parser *p);

void push_exec_param(AST *func_exec, AST *param)
{
    if (func_exec->func_exec.size >= func_exec->func_exec.capacity) {
        func_exec->func_exec.capacity *= 2;
        func_exec->func_exec.params =
            realloc(func_exec->func_exec.params,
                    sizeof(AST *) * func_exec->func_exec.capacity);
    }

    func_exec->func_exec.params[func_exec->func_exec.size++] = param;
}

int is_func_exec(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_LPAREN) return 0;

    return 1;
}

AST * parse_func_exec(Parser *p)
{
    AST *node = create_ast_node(AST_FUNC_EXEC);
    node->func_exec.params = (AST **) malloc(sizeof(AST *));
    node->func_exec.size = 0;
    node->func_exec.capacity = 1;
    node->func_exec.name = parser_peek(p, 0).text;
    parser_advance(p, 2);

    while (parser_peek(p, 0).type != TOKEN_RPAREN) {
        push_exec_param(node, parse_expression(p));
        if (parser_peek(p, 0).type == TOKEN_COMMA) parser_advance(p, 1);
    }
    parser_advance(p, 1);

    return node;
}

AST * parse_factor(Parser *p)
{
    Token token = parser_peek(p, 0);

    // Lógica para Parênteses
    if (token.type == TOKEN_LPAREN) {
        parser_advance(p, 1); // Consome o '('
        
        // Reinicia o ciclo: trata o conteúdo como uma nova expressão completa
        AST *node = parse_expression(p);
        node->expression.has_paren = 1;

        // Após a expressão, PRECISA haver um ')'
        match(p, TOKEN_RPAREN, "')' needed to close a '('b");
        parser_advance(p, 1); // Consome o ')'
        
        return node;
    }

    if (is_number(parser_peek(p, 0))) return parse_number(p);
    if (is_identifier_update(p)) return parse_identifier_update(p);
    if (is_func_exec(p)) return parse_func_exec(p);

    return parse_identifier(p);
}

int is_expression(Parser *p) { return is_factor(p, 0); }

int is_operator(Token token)
{
    if (token.type != TOKEN_PLUS           &&
        token.type != TOKEN_MINUS          &&
        token.type != TOKEN_STAR           &&
        token.type != TOKEN_SLASH          &&
        token.type != TOKEN_GREATER        &&
        token.type != TOKEN_GREATER_EQUALS &&
        token.type != TOKEN_LESS           &&
        token.type != TOKEN_LESS_EQUALS    &&
        token.type != TOKEN_EQUALS         &&
        token.type != TOKEN_AND            &&
        token.type != TOKEN_OR) return 0;

    return 1;
}

AST * parse_expression(Parser *p)
{
    AST *left = parse_factor(p);

    Token operator = parser_peek(p, 0);

    if (!is_operator(operator)) return left;
    
    parser_advance(p, 1);

    AST *op = create_ast_node(AST_OPERATOR);
    op->expression.left = left;
    op->expression.right = parse_expression(p);
    op->expression.type = operator.text;
    op->expression.has_paren = 0;

    return op;
}

int is_assign(Token token)
{
    return token.type == TOKEN_ASSIGN       ||
           token.type == TOKEN_PLUS_ASSIGN  ||
           token.type == TOKEN_MINUS_ASSIGN ||
           token.type == TOKEN_STAR_ASSIGN  ||
           token.type == TOKEN_SLASH_ASSIGN;
}

int is_assignment(Parser *p)
{
    if(is_var_def(p)) {
        int offset = is_var_def_implicit(p) ? 3 : 4;
        if (parser_peek(p, offset-1).type == TOKEN_SEMICOLON) return 0;
        if (is_factor(p, offset)) return 1;
    } else {
        if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
        if (!is_assign(parser_peek(p, 1)) && !is_factor(p, 2)) return 0;
    }

    return 1;
}

AST * parse_assignment(Parser *p)
{
    AST *node = create_ast_node(AST_ASSIGN);
    if (is_var_def(p)) {
        node->assign.left = parse_var_def(p);
        node->assign.left->var_def.initialized = 1;
    } else {
        node->assign.left = parse_identifier(p);
    }

    Token token = parser_peek(p, 0);

    node->assign.type = token.text;
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
int is_block(Parser *p, int level)
{
    if (level == 0) return 1;

    if (parser_peek(p, 0).type != TOKEN_LBRACE) return 0;

    return 1;
}

int is_const_def(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_DOUBLE_COLON) return 0;
    if (!is_factor(p, 2)) return 0;

    return 1;
}

AST * parse_const_def(Parser *p)
{
    AST *node = create_ast_node(AST_CONST_DEF);
    node->const_def.name = parser_peek(p, 0).text;
    parser_advance(p, 2);

    node->const_def.value = parse_expression(p);
    if (node->const_def.value->type == AST_NUMBER)
        node->const_def.type = node->const_def.value->number.type;
    else
        node->const_def.type = strdup("int");

    return node;
}

int is_return(Parser *p)
{
    if(parser_peek(p, 0).type != TOKEN_RETURN) return 0;

    return 1;
}

int is_command(Parser *p)
{
    return is_identifier_update(p) ||
           is_assignment(p)        ||
           is_var_def(p)           ||
           is_const_def(p)         ||
           is_return(p);
}

AST * parse_return_expression(Parser *p)
{
    AST *node = create_ast_node(AST_RETURN);
    parser_advance(p, 1);
    node->return_statement = is_expression(p) ? parse_expression(p) : NULL;

    return node;
}

AST * parse_command(Parser *p)
{
    AST *node = create_ast_node(AST_COMMAND);

    if (is_identifier_update(p)) {
        node->command = parse_identifier_update(p);
        match(p, TOKEN_SEMICOLON, "; expected to define a identifier updater");
        parser_advance(p, 1);
    } else if (is_const_def(p)) {
        node->command = parse_const_def(p);
        match(p, TOKEN_SEMICOLON, "; expected to define a const");
        parser_advance(p, 1);
    } else if (is_assignment(p)) {
        node->command = parse_assignment(p);
        match(p, TOKEN_SEMICOLON, "; expected to define a assignment");
        parser_advance(p, 1);
    } else if (is_var_def(p)) {
        node->command = parse_var_def(p);
        match(p, TOKEN_SEMICOLON, "; expected to define a variable");
        parser_advance(p, 1);
    } else if (is_return(p)) {
        node->command = parse_return_expression(p);
        match(p, TOKEN_SEMICOLON, "; expected to define a return");
        parser_advance(p, 1);
    }

    return node;
}

AST * parse_block(Parser *p, int level);

AST * parse_if(Parser *p, int level)
{
    AST *node = create_ast_node(AST_IF);
    parser_advance(p, 1);
    node->if_statement.expression = parse_expression(p);
    node->if_statement.if_block = parse_block(p, level);

    return node;
}

AST * parse_while(Parser *p, int level)
{
    AST *node = create_ast_node(AST_WHILE);
    parser_advance(p, 1);
    node->while_statement.expression = parse_expression(p);
    node->while_statement.block = parse_block(p, level);

    return node;
}

AST * parse_for(Parser *p, int level)
{
    AST *node = create_ast_node(AST_FOR);
    parser_advance(p, 1);

    if (!is_expression(p)) {
        report_error(p, parser_peek(p, 0), "Need an expression in for start range");
        exit(0);
    }
    node->for_statement.start = parse_expression(p);

    Token token = parser_peek(p, 0);
    
    switch (token.type) {
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
            report_error(p, token, "Need '..|>.|.<|><' to define a for range");
    }

    parser_advance(p, 1);

    node->for_statement.end = parse_expression(p);
    node->for_statement.block = parse_block(p, level+1);

    return node;
}

int is_func_def(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_DOUBLE_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_LPAREN) return 0;

    // With parameters
    if (parser_peek(p, 3).type == TOKEN_IDENTIFIER &&
        parser_peek(p, 4).type == TOKEN_COLON) return 1;

    if (parser_peek(p, 3).type != TOKEN_RPAREN) return 0;
    if (parser_peek(p, 4).type != TOKEN_ARROW) return 0;
    if (parser_peek(p, 5).type != TOKEN_IDENTIFIER) return 0;

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
    node->func_def_param.name = parser_peek(p, 0).text;
    node->func_def_param.type = parser_peek(p, 2).text;
    parser_advance(p, 3);

    return node;
}

AST * parse_func_def(Parser *p, int level)
{
    AST *node = create_ast_node(AST_FUNC_DEF);
    node->func_def.params = (AST **) malloc(sizeof(AST *));
    node->func_def.size = 0;
    node->func_def.capacity = 1;
    node->func_def.name = parser_peek(p, 0).text;
    parser_advance(p, 3);

    while (parser_peek(p, 0).type != TOKEN_RPAREN) {
        push_param(node, parse_func_def_param(p));
        if (parser_peek(p, 0).type == TOKEN_COMMA) parser_advance(p, 1);
    }
    parser_advance(p, 2);
    node->func_def.return_type = parser_peek(p, 0).text;
    parser_advance(p, 1);

    node->func_def.block = parse_block(p, level+1);

    return node;
}

AST * parse_statement(Parser *p, int level)
{
    AST *node;

    if (is_func_def(p)) {
        node = parse_func_def(p,  level + 1);
        parser_advance(p, 1);
    } else if (is_command(p)) {
        node = parse_command(p);
    } else if (is_block(p, level + 1)) {
        node = parse_block(p, level + 1);
        match(p, TOKEN_RBRACE, "} expected to define a block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0).type == TOKEN_IF) {
        node = parse_if(p,  level + 1);
        match(p, TOKEN_RBRACE, "} expected to define a if block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0).type == TOKEN_WHILE) {
        node = parse_while(p,  level + 1);
        match(p, TOKEN_RBRACE, "} expected to define a while block");
        parser_advance(p, 1);
    } else if (parser_peek(p, 0).type == TOKEN_FOR) {
        node = parse_for(p,  level + 1);
        match(p, TOKEN_RBRACE, "} expected to define a for block");
        parser_advance(p, 1);
    } else {
        printf("Syntax error\n");
        printf("Token: %s\n", parser_peek(p, 0).text);
        printf("Line: %s\n", get_token_source_line(p, parser_peek(p, 0)));
        exit(1);
    }

    return node;
}

AST * parse_block(Parser *p, int level)
{
    AST *block = create_ast_node(AST_BLOCK);
    block->block.statements = (AST **) malloc(sizeof(AST *));
    block->block.size = 0;
    block->block.capacity = 1;
    block->block.level = level;
    
    if (level > 0) parser_advance(p, 1);

    while (parser_peek(p, 0).type != ((level > 0) ? TOKEN_RBRACE : TOKEN_EOF))
        push_statement(block, parse_statement(p, level));

    return block;
}

AST * parse(TokenList list, char *source, char *file)
{
    Parser p = { list, 0, source, file };

    AST *ast = parse_block(&p, 0);

    Token t = parser_peek(&p, 0);
    if (t.type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t.text);
        exit(1);
    }

    return ast;
}
