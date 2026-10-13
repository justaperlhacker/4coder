/* date = October 12th 2026

   lexer_test_stubs.h - The minimum surface of the 4coder runtime that the
   hand-written language lexers rely on, so we can compile and run them
   outside of the editor as plain C++ unit tests.

   Types and helper names match the real 4coder definitions closely enough
   that the lexer sources include with no modifications. */

#ifndef FCODER_FLEURY_LEXER_TEST_STUBS_H
#define FCODER_FLEURY_LEXER_TEST_STUBS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;
typedef int b32;

#define internal static
#define ArrayCount(a) ((int)(sizeof(a) / sizeof((a)[0])))

//~ Token kinds mirror 4coder's Token_Base_Kind enum exactly so we can share
//~ source with the editor-side lexers.
enum Token_Base_Kind{
    TokenBaseKind_EOF = 0,
    TokenBaseKind_Whitespace = 1,
    TokenBaseKind_LexError = 2,
    TokenBaseKind_Comment = 3,
    TokenBaseKind_Keyword = 4,
    TokenBaseKind_Preprocessor = 5,
    TokenBaseKind_Identifier = 6,
    TokenBaseKind_Operator = 7,
    TokenBaseKind_LiteralInteger = 8,
    TokenBaseKind_LiteralFloat = 9,
    TokenBaseKind_LiteralString = 10,
    TokenBaseKind_ScopeOpen = 11,
    TokenBaseKind_ScopeClose = 12,
    TokenBaseKind_ParentheticalOpen = 13,
    TokenBaseKind_ParentheticalClose = 14,
    TokenBaseKind_StatementClose = 15,
    TokenBaseKind_COUNT = 16,
};

struct String_Const_u8{
    u8 *str;
    u64 size;
};
#define S8Lit(s) (String_Const_u8){(u8*)(s), (u64)(sizeof(s)-1)}

struct Token{
    i64 pos;
    i64 size;
    Token_Base_Kind kind;
    u16 flags;
    i16 sub_kind;
    u16 sub_flags;
};

struct Arena{ u8 dummy; };
static Arena g_arena;

struct Token_List{ u64 count; };

//~ token_list_push is a singly-linked list; tests read via lex_test_head().
struct TokenNode{
    TokenNode *next;
    Token t;
};
static TokenNode *g_lex_test_head = 0;
static TokenNode *g_lex_test_tail = 0;

internal void
token_list_push(Arena *arena, Token_List *list, Token *t)
{
    (void)arena;
    TokenNode *n = (TokenNode*)malloc(sizeof(TokenNode));
    n->t = *t;
    n->next = 0;
    if(g_lex_test_tail) g_lex_test_tail->next = n;
    else g_lex_test_head = n;
    g_lex_test_tail = n;
    list->count += 1;
}

internal void
lex_test_reset_token_stream(void)
{
    while(g_lex_test_head){
        TokenNode *x = g_lex_test_head;
        g_lex_test_head = x->next;
        free(x);
    }
    g_lex_test_tail = 0;
}

internal TokenNode *
lex_test_token_stream(void)
{
    return g_lex_test_head;
}

//~ The handful of helpers the lexer code paths actually call.
internal b32 character_is_whitespace(u8 c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0x0b || c == 0x0c;
}
internal b32 character_is_alpha(u8 c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
internal b32 character_is_alpha_numeric(u8 c){
    return character_is_alpha(c) || (c >= '0' && c <= '9');
}
internal b32 string_match(String_Const_u8 a, String_Const_u8 b){
    if(a.size != b.size) return 0;
    for(u64 i = 0; i < a.size; i += 1) if(a.str[i] != b.str[i]) return 0;
    return 1;
}

//~ Signatures from 4coder_fleury_lang.h so the language files compile as-is.
#define F4_LANGUAGE_LEXINIT(name) void name(void *state_ptr, String_Const_u8 contents)
#define F4_LANGUAGE_LEXFULLINPUT(name) b32 name(Arena *arena, Token_List *list, void *state_ptr, u64 max)
#define F4_LANGUAGE_INDEXFILE(name) void name(void *ctx)
#define F4_LANGUAGE_POSCONTEXT(name) void* name(void *app, Arena *arena, int buffer, i64 pos)
#define F4_LANGUAGE_HIGHLIGHT(name) void name(void *app, int tl, void *arr, int ct)

//~ Highlight hook uses these — no-op stubs.
struct R_i64{ i64 first, one_past_last; };
#define fleury_color_index_comment_tag 0
#define F4_ARGBFromID(...) 0
internal R_i64 text_layout_get_visible_range(void *a, int tl){ (void)a; (void)tl; R_i64 r = {0, 0}; return r; }
internal i64 token_index_from_pos(void *a, i64 p){ (void)a; (void)p; return 0; }
internal int token_iterator_index(void *a, void *b, i64 c){ (void)a; (void)b; (void)c; return 0; }
internal Token *token_it_read(int *it){ (void)it; return 0; }
internal b32 token_it_inc_all(int *it){ (void)it; return 0; }
internal void paint_text_color(void *a, int tl, R_i64 r, int c){ (void)a; (void)tl; (void)r; (void)c; }
#define Ii64_t(a,b) (R_i64){a,b}
#define Ii64(t) (R_i64){t.pos, t.pos + t.size}

#endif // FCODER_FLEURY_LEXER_TEST_STUBS_H
