#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/types.h"

#define MAX_FIELDS  256
#define MAX_STRUCTS 128
#define MAX_NAME    128
#define MAX_GEN     256

GEN(array)
typedef struct Field {
    char name[MAX_NAME];
    char type[MAX_NAME];
    char gen[MAX_GEN]; // field-level GEN(...)
} Field;

GEN(array)
typedef struct StructDef {
    char file[MAX_NAME];
    char name[MAX_NAME];
    char gen[MAX_GEN]; // struct-level GEN(...)

    Field fields[MAX_FIELDS];
    int   field_count;
} StructDef;

static StructDef structs[MAX_STRUCTS];
static int       struct_count = 0;
static cstr      src;
static cstr      filename;

// --- Tokenizer ---
typedef enum TokenKind {
    TOK_EOF,
    TOK_IDENT,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_SEMI,
    TOK_STAR
} TokenKind;

typedef struct Token {
    TokenKind kind;
    char      text[256];
} Token;

static void skip_ws(void) {
    for (;;) {
        if (isspace((unsigned char)*src))
            src++;
        else if (*src == '/' && src[1] == '/') {
            while (*src && *src != '\n') src++;
        } else if (*src == '/' && src[1] == '*') {
            src += 2;
            while (*src && !(*src == '*' && src[1] == '/')) src++;
            if (*src) src += 2;
        } else
            break;
    }
}

static struct Token next_token(void) {
    skip_ws();
    Token tok = {TOK_EOF, {0}};
    if (!*src) return tok;

    if (isalpha((unsigned char)*src) || *src == '_') {
        int n = 0;
        while (isalnum((unsigned char)*src) || *src == '_') tok.text[n++] = *src++;
        tok.text[n] = 0;
        tok.kind    = TOK_IDENT;
        return tok;
    }
    switch (*src) {
    case '{':
        tok.kind    = TOK_LBRACE;
        tok.text[0] = *src++;
        break;
    case '}':
        tok.kind    = TOK_RBRACE;
        tok.text[0] = *src++;
        break;
    case '(':
        tok.kind    = TOK_LPAREN;
        tok.text[0] = *src++;
        break;
    case ')':
        tok.kind    = TOK_RPAREN;
        tok.text[0] = *src++;
        break;
    case ';':
        tok.kind    = TOK_SEMI;
        tok.text[0] = *src++;
        break;
    case '*':
        tok.kind    = TOK_STAR;
        tok.text[0] = *src++;
        break;
    default:
        tok.kind    = TOK_IDENT;
        tok.text[0] = *src++;
        tok.text[1] = 0;
        break;
    }
    return tok;
}

static Token lookahead;
static int   has_lookahead = 0;
static Token peek_token(void) {
    if (!has_lookahead) {
        lookahead     = next_token();
        has_lookahead = 1;
    }
    return lookahead;
}
static Token consume_token(void) {
    if (has_lookahead) {
        has_lookahead = 0;
        return lookahead;
    }
    return next_token();
}

// --- GEN buffers ---
static char last_struct_gen[MAX_GEN];
static char last_field_gen[MAX_GEN];

static void parse_gen(void) {
    consume_token(); // consume GEN
    Token t = consume_token();
    if (t.kind != TOK_LPAREN) return;

    char buf[MAX_GEN];
    int  n = 0, depth = 1;
    while (*src && depth > 0 && n < MAX_GEN - 1) {
        if (*src == '(')
            depth++;
        else if (*src == ')')
            depth--;
        if (depth > 0) buf[n++] = *src;
        src++;
    }
    buf[n] = 0;

    // decide if struct-level or field-level
    if (last_struct_gen[0] == 0 && peek_token().kind == TOK_IDENT &&
        strcmp(peek_token().text, "typedef") == 0) {
        SDL_strlcpy(last_struct_gen, buf, MAX_GEN);
    } else {
        SDL_strlcpy(last_field_gen, buf, MAX_GEN);
    }
}

static void parse_struct(void) {
    consume_token(); // typedef
    if (!(peek_token().kind == TOK_IDENT && strcmp(peek_token().text, "struct") == 0)) return;
    consume_token(); // struct

    struct Token nameTok = consume_token();
    if (nameTok.kind != TOK_IDENT) return;

    struct StructDef def = {0};
    SDL_strlcpy(def.file, filename, MAX_NAME);
    SDL_strlcpy(def.name, nameTok.text, MAX_NAME);
    if (last_struct_gen[0]) {
        SDL_strlcpy(def.gen, last_struct_gen, MAX_GEN);
        last_struct_gen[0] = 0;
    }

    struct Token lb = consume_token();
    if (lb.kind != TOK_LBRACE) return;

    while (1) {
        struct Token t = peek_token();
        if (t.kind == TOK_RBRACE || t.kind == TOK_EOF) break;

        if (t.kind == TOK_IDENT && strcmp(t.text, "GEN") == 0) {
            parse_gen();
            continue;
        }

        struct Token typeTok = consume_token();
        if (typeTok.kind != TOK_IDENT) continue;

        int is_ptr = 0;
        if (peek_token().kind == TOK_STAR) {
            consume_token();
            is_ptr = 1;
        }

        struct Token nameTok2 = consume_token();
        if (nameTok2.kind != TOK_IDENT) continue;

        while (peek_token().kind != TOK_SEMI && peek_token().kind != TOK_EOF) consume_token();
        if (peek_token().kind == TOK_SEMI) consume_token();

        struct Field f = {0};
        snprintf(f.type, MAX_NAME, "%s%s", typeTok.text, is_ptr ? "*" : "");
        SDL_strlcpy(f.name, nameTok2.text, MAX_NAME);
        if (last_field_gen[0]) {
            SDL_strlcpy(f.gen, last_field_gen, MAX_GEN);
            last_field_gen[0] = 0;
        }

        def.fields[def.field_count++] = f;
    }

    if (peek_token().kind == TOK_RBRACE) consume_token();
    while (peek_token().kind != TOK_SEMI && peek_token().kind != TOK_EOF) consume_token();
    if (peek_token().kind == TOK_SEMI) consume_token();

    structs[struct_count++] = def;
}

static void parse(void) {
    while (1) {
        struct Token t = peek_token();
        if (t.kind == TOK_EOF) break;
        if (t.kind == TOK_IDENT && strcmp(t.text, "GEN") == 0) {
            parse_gen();
            continue;
        }
        if (t.kind == TOK_IDENT && strcmp(t.text, "typedef") == 0) {
            parse_struct();
            continue;
        }
        consume_token(); // ignore other code
    }
}
