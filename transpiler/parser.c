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
            printf("OPERATOR(%s)\n", node->op.type);
            show_ast(node->op.left, indent + 1);
            show_ast(node->op.right, indent + 1);
            break;
        }
        case AST_BLOCK: {
            printf("BLOCK(%d)\n", node->block.size);
            for (int i = 0; i < node->block.size; i++)
                show_ast(node->block.statements[i], indent + 1);
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
        fprintf(stderr, "Erro de Sintaxe: %s (Found: '%s')\n", 
                error_msg, token.text);
        exit(1);
    }
}

int is_var_def(Parser *p)
{
    if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
    if (parser_peek(p, 1).type != TOKEN_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_IDENTIFIER &&
        parser_peek(p, 2).type != TOKEN_ASSIGN /*Implicit type*/) return 0;

    return 1;
}

AST * parse_var_def(Parser *p)
{
    AST *node = create_ast_node(AST_VAR_DEF);
    node->var_def.initialized = 0;
    node->var_def.name = parser_peek(p, 0).text;

    int implicit_type = parser_peek(p, 2).type == TOKEN_ASSIGN;
    node->var_def.type = implicit_type ? strdup("int") : parser_peek(p, 2).text;

    parser_advance(p, 3 - implicit_type);

    return node;
}


int is_number(Token token)
{
    return token.type == TOKEN_INT || token.type == TOKEN_FLOAT;
}

int is_factor(Token token)
{
    return (token.type == TOKEN_IDENTIFIER || is_number(token));
}

int is_assign(Parser *p)
{
    if(is_var_def(p)) {
        int offset = 3;
        int implicit_type = parser_peek(p, 0 + offset-1).type == TOKEN_ASSIGN;
        if (!implicit_type && parser_peek(p, 0 + offset).type != TOKEN_ASSIGN)
            return 0;

        // If implicit_type, var_def consume 2 tokens (name and ':')
        offset -= implicit_type; 

        if (!is_factor(parser_peek(p, 1 + offset))) return 0;
    } else {
        if (parser_peek(p, 0).type != TOKEN_IDENTIFIER) return 0;
        if (parser_peek(p, 1).type != TOKEN_ASSIGN) return 0;
        if (!is_factor(parser_peek(p, 2))) return 0;
    }

    return 1;
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

AST * parse_factor(Parser *p)
{
    if (is_number(parser_peek(p, 0))) return parse_number(p);

    return parse_identifier(p);
}

inline int is_expression(Parser *p) { return is_factor(parser_peek(p, 0)); }

int is_operator(Token token)
{
    if (token.type != TOKEN_PLUS  &&
        token.type != TOKEN_MINUS &&
        token.type != TOKEN_STAR  &&
        token.type != TOKEN_SLASH) return 0;

    return 1;
}

AST * parse_expression(Parser *p)
{
    AST *left = parse_factor(p);

    Token operator = parser_peek(p, 0);

    if (!is_operator(operator)) return left;
    
    parser_advance(p, 1);

    AST *op = create_ast_node(AST_OPERATOR);
    op->op.left = left;
    op->op.right = parse_expression(p);
    op->op.type = operator.text;

    return op;
}

AST * parse_assign(Parser *p)
{
    AST *node = create_ast_node(AST_ASSIGN);
    if (is_var_def(p)) {
        node->assign.left = parse_var_def(p);
        node->assign.left->var_def.initialized = 1;
    } else {
        node->assign.left = parse_identifier(p);
    }
    node->assign.type = parser_peek(p, 0).text;
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

char * get_token_source_line(Parser *p, Token token)
{
    int pos = 0;
    int line = 1;
    char ch;
    while (1) {
        ch = p->source[pos];
        if (ch == '\n') {
            line++;
            continue;
        }

        if (line == token.line) {
            int start = pos;
            while(p->source[++pos] != '\n');
            return strndup(p->source + start, (pos-1 - start) + 1);
        }
        pos++;
    }
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
    if (parser_peek(p, 1).type != TOKEN_COLON) return 0;
    if (parser_peek(p, 2).type != TOKEN_COLON) return 0;
    if (!is_factor(parser_peek(p, 3))) return 0;

    return 1;
}

AST * parse_const_def(Parser *p)
{
    AST *node = create_ast_node(AST_CONST_DEF);
    node->const_def.name = parser_peek(p, 0).text;
    parser_advance(p, 3);

    node->const_def.value = parse_expression(p);
    node->const_def.type = node->const_def.value->number.type;

    return node;
}

AST * parse_block(Parser *p, int level)
{
    AST *block = create_ast_node(AST_BLOCK);
    block->block.statements = (AST **) malloc(sizeof(AST *));
    block->block.size = 0;
    block->block.capacity = 1;
    
    if (level > 0) parser_advance(p, 1);

    AST *ast;

    while (parser_peek(p, 0).type != ((level > 0) ? TOKEN_RBRACE : TOKEN_EOF)) {
        if (is_assign(p)) {
            ast = parse_assign(p);
            match(p, TOKEN_SEMICOLON, "; expected to define a assignment");
            parser_advance(p, 1);
        } else if (is_var_def(p)) {
            ast = parse_var_def(p);
            match(p, TOKEN_SEMICOLON, "; expected to define a variable");
            parser_advance(p, 1);
        } else if (is_const_def(p)) {
            ast = parse_const_def(p);
            match(p, TOKEN_SEMICOLON, "; expected to define a const");
            parser_advance(p, 1);
        } else if (is_block(p, level + 1)) {
            ast = parse_block(p, level + 1);
            match(p, TOKEN_RBRACE, "} expected to define a block");
            parser_advance(p, 1);
        } else {
            printf("Syntax error\n");
            printf("Line: %s\n", get_token_source_line(p, parser_peek(p, 0)));
            exit(1);
        }

        push_statement(block, ast);
    }

    return block;
}

AST * parse(TokenList list, char *source)
{
    Parser p = { list, 0, source };

    AST *ast = parse_block(&p, 0);

    Token t = parser_peek(&p, 0);
    if (t.type != TOKEN_EOF) {
        printf("Erro: unexpected token at end\n");
        printf("Got: '%s'", t.text);
        exit(1);
    }

    return ast;
}
