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

void match(Parser* p, TokenType type, const char* error_msg) {
    Token token = parser_peek(p, 0);
    if (token.type != type) {
        printf("%s:%d:%d error: %s\n", p->file, token.line, token.column, error_msg);
        printf("%4d | %s\n", token.line, get_token_source_line(p, token));
        printf("Got: %s\n", token.text);
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

int is_factor(Parser *p, int offset)
{
    Token token = parser_peek(p, offset);
    return (token.type == TOKEN_IDENTIFIER ||
            is_number(token)               ||
            is_identifier_update(p));
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

AST * parse_factor(Parser *p)
{
    if (is_number(parser_peek(p, 0))) return parse_number(p);
    if (is_identifier_update(p)) return parse_identifier_update(p);

    return parse_identifier(p);
}

inline int is_expression(Parser *p) { return is_factor(p, 0); }

int is_operator(Token token)
{
    if (token.type != TOKEN_PLUS  &&
        token.type != TOKEN_MINUS &&
        token.type != TOKEN_STAR  &&
        token.type != TOKEN_SLASH &&
        token.type != TOKEN_GREATER &&
        token.type != TOKEN_GREATER_EQUALS &&
        token.type != TOKEN_LESS &&
        token.type != TOKEN_LESS_EQUALS &&
        token.type != TOKEN_EQUALS) return 0;

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

int is_command(Parser *p)
{
    return is_identifier_update(p) ||
           is_assignment(p)        ||
           is_var_def(p)           ||
           is_const_def(p);
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
    }

    return node;
}

AST * parse_block(Parser *p, int level);

AST * parse_statement(Parser *p, int level)
{
    AST *node;

    if (is_command(p)) {
        node = parse_command(p);
    } else if (is_block(p, level + 1)) {
        node = parse_block(p, level + 1);
        match(p, TOKEN_RBRACE, "} expected to define a block");
        parser_advance(p, 1);
    } else {
        printf("Syntax error\n");
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
