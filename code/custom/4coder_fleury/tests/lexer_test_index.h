/* date = October 12th 2026

   lexer_test_index.h - A faithful-but-minimal stand-in for the F4 custom code
   index (4coder_fleury_index.cpp) so the language IndexFile hooks can be
   compiled and unit-tested outside the editor.

   Semantics mirror the real implementation verified against source:
     - Range_i64 uses {min, one_past_last} (see 4coder_base_types.h union).
     - token_iterator_pos / token_it_read / token_it_inc_non_whitespace follow
       4coder_token.cpp exactly (read = current slot, inc skips whitespace).
     - F4_Index_RequireToken matches the token TEXT via string_substring and
       does NOT pre-skip whitespace; F4_Index_RequireTokenKind matches kind and
       does NOT pre-skip; both advance with ParseCtx_Inc (which skips ws).
     - F4_Index_ParsePattern restores the whole ctx on any parse failure.

   Captured notes land in a flat ring so tests can assert the set of symbols an
   IndexFile emitted, without pulling in the full note-tree/hashing machinery. */

#ifndef FCODER_FLEURY_LEXER_TEST_INDEX_H
#define FCODER_FLEURY_LEXER_TEST_INDEX_H

#include "lexer_test_stubs.h"

typedef int Buffer_ID_like;

//~ ---------------------------------------------------------------------------
//~ F4 index model types (subset)
//~ ---------------------------------------------------------------------------

enum F4_Index_NoteKind{
    F4_Index_NoteKind_Null,
    F4_Index_NoteKind_Scope,
    F4_Index_NoteKind_Type,
    F4_Index_NoteKind_Constant,
    F4_Index_NoteKind_Function,
    F4_Index_NoteKind_Decl,
    F4_Index_NoteKind_Macro,
    F4_Index_NoteKind_CommentTag,
    F4_Index_NoteKind_CommentToDo,
    F4_Index_NoteKind_COUNT
};

typedef u32 F4_Index_NoteFlags;
enum{
    F4_Index_NoteFlag_Prototype   = (1<<0),
    F4_Index_NoteFlag_ProductType = (1<<1),
    F4_Index_NoteFlag_SumType     = (1<<2),
};

typedef u32 F4_Index_TokenSkipFlags;
enum{
    F4_Index_TokenSkipFlag_SkipWhitespace = (1<<0),
};

struct F4_Index_Note{
    F4_Index_Note *next;
    String_Const_u8 string;
    F4_Index_NoteKind kind;
    F4_Index_NoteFlags flags;
    Range_i64 range;
};

struct F4_Index_File{
    Buffer_ID_like buffer;
    F4_Index_Note *first_note;
};

struct Token_Array{
    Token *tokens;
    i64 count;
    i64 max;
};

struct Token_Iterator_Array{
    u64 user_id;
    Token *ptr;
    Token *tokens;
    i64 count;
};

struct F4_Index_ParseCtx{
    b32 done;
    void *app;
    F4_Index_File *file;
    String_Const_u8 string;
    Token_Array tokens;
    Token_Iterator_Array it;
    F4_Index_Note *active_parent;
};

//~ ---------------------------------------------------------------------------
//~ Captured notes: reset per test, inspected by the index test runner.
//~ ---------------------------------------------------------------------------
#define LEX_TEST_MAX_NOTES 256
static F4_Index_Note g_lex_index_notes[LEX_TEST_MAX_NOTES];
static u32 g_lex_index_note_count = 0;

internal void
lex_test_reset_index(void)
{
    g_lex_index_note_count = 0;
}

//~ ---------------------------------------------------------------------------
//~ Token iterator (mirrors 4coder_token.cpp)
//~ ---------------------------------------------------------------------------

internal Token_Iterator_Array
token_iterator_pos(u64 user_id, Token_Array *tokens, i64 pos)
{
    Token_Iterator_Array it = {};
    if(tokens->tokens != 0 && tokens->count > 0)
    {
        if(pos < 0) pos = 0;
        if(pos > tokens->count - 1) pos = tokens->count - 1;
        it.user_id = user_id;
        it.tokens = tokens->tokens;
        it.count = tokens->count;
        it.ptr = tokens->tokens + pos;
    }
    return it;
}

internal Token *
token_it_read(Token_Iterator_Array *it)
{
    return it->tokens != 0 ? it->ptr : 0;
}

internal b32
token_it_inc_all(Token_Iterator_Array *it)
{
    b32 result = false;
    if(it->tokens != 0)
    {
        if(it->ptr < it->tokens + it->count - 1)
        {
            it->ptr += 1;
            result = true;
        }
    }
    return result;
}

internal b32
token_it_inc_non_whitespace(Token_Iterator_Array *it)
{
    b32 result = false;
repeat:
    if(token_it_inc_all(it))
    {
        Token *token = token_it_read(it);
        if(token != 0 && token->kind == TokenBaseKind_Whitespace)
        {
            goto repeat;
        }
        result = true;
    }
    return result;
}

//~ ---------------------------------------------------------------------------
//~ Index parse API (mirrors 4coder_fleury_index.cpp)
//~ ---------------------------------------------------------------------------

internal String_Const_u8
F4_Index_StringFromRange(F4_Index_ParseCtx *ctx, Range_i64 range)
{
    return string_substring(ctx->string, range);
}

internal String_Const_u8
F4_Index_StringFromToken(F4_Index_ParseCtx *ctx, Token *token)
{
    return F4_Index_StringFromRange(ctx, Ii64(token));
}

internal b32
F4_Index_ParseCtx_Inc(F4_Index_ParseCtx *ctx, F4_Index_TokenSkipFlags flags)
{
    if(flags & F4_Index_TokenSkipFlag_SkipWhitespace)
    {
        ctx->done = !token_it_inc_non_whitespace(&ctx->it);
    }
    else
    {
        ctx->done = !token_it_inc_all(&ctx->it);
    }
    return ctx->done;
}

internal F4_Index_Note *
F4_Index_LookupNote(String_Const_u8 string)
{
    for(u32 i = 0; i < g_lex_index_note_count; i += 1)
    {
        if(string_match(g_lex_index_notes[i].string, string))
        {
            return &g_lex_index_notes[i];
        }
    }
    return 0;
}

internal F4_Index_Note *
F4_Index_LookupNoteP(String_Const_u8 string, F4_Index_Note *parent)
{
    (void)parent;
    return F4_Index_LookupNote(string);
}

internal F4_Index_Note *
F4_Index_MakeNote(F4_Index_ParseCtx *ctx, Range_i64 name_range,
                  F4_Index_NoteKind note_kind, F4_Index_NoteFlags note_flags)
{
    String_Const_u8 name = string_substring(ctx->string, name_range);
    if(g_lex_index_note_count >= LEX_TEST_MAX_NOTES)
    {
        return 0;
    }
    F4_Index_Note *note = &g_lex_index_notes[g_lex_index_note_count++];
    note->next = 0;
    note->string = name;
    note->kind = note_kind;
    note->flags = note_flags;
    note->range = name_range;
    return note;
}

internal b32
F4_Index_RequireToken(F4_Index_ParseCtx *ctx, String_Const_u8 string, F4_Index_TokenSkipFlags flags)
{
    b32 result = 0;
    Token *token = token_it_read(&ctx->it);
    if(token)
    {
        String_Const_u8 token_string = string_substring(ctx->string, Ii64p(token->pos, token->pos + token->size));
        if(string_match(string, token_string))
        {
            result = 1;
        }
    }
    else
    {
        ctx->done = 1;
    }
    if(result)
    {
        F4_Index_ParseCtx_Inc(ctx, flags);
    }
    return result;
}

internal b32
F4_Index_RequireTokenKind(F4_Index_ParseCtx *ctx, Token_Base_Kind kind, Token **token_out, F4_Index_TokenSkipFlags flags)
{
    b32 result = 0;
    Token *token = token_it_read(&ctx->it);
    if(token)
    {
        if(token->kind == kind)
        {
            result = 1;
            if(token_out)
            {
                *token_out = token;
            }
        }
    }
    else
    {
        ctx->done = 1;
    }
    if(result)
    {
        F4_Index_ParseCtx_Inc(ctx, flags);
    }
    return result;
}

internal b32
F4_Index_PeekToken(F4_Index_ParseCtx *ctx, String_Const_u8 string)
{
    b32 result = 0;
    Token *token = token_it_read(&ctx->it);
    if(token)
    {
        String_Const_u8 token_string = string_substring(ctx->string, Ii64p(token->pos, token->pos + token->size));
        if(string_match(string, token_string))
        {
            result = 1;
        }
    }
    else
    {
        ctx->done = 1;
    }
    return result;
}

internal void
F4_Index_ParseComment(F4_Index_ParseCtx *ctx, Token *token)
{
    String_Const_u8 string = F4_Index_StringFromToken(ctx, token);
    for(u64 i = 0; i < string.size; i += 1)
    {
        if(string.str[i] == '@')
        {
            Range_i64 range = Ii64(token);
            range.min += (i64)i;
            F4_Index_MakeNote(ctx, range, F4_Index_NoteKind_CommentTag, 0);
            break;
        }
        else if(i + 4 < string.size && string_match(S8Lit("TODO"), string_substring(string, Ii64p((i64)i, (i64)(i+4)))))
        {
            Range_i64 range = Ii64(token);
            range.min += (i64)i;
            F4_Index_MakeNote(ctx, range, F4_Index_NoteKind_CommentToDo, 0);
        }
    }
}

//~ Variadic pattern matcher supporting the %t / %k / %n specifiers our language
//~ hooks actually use. Restores ctx on failure like the real implementation.
#include <stdarg.h>
internal b32
F4_Index_ParsePattern(F4_Index_ParseCtx *ctx, char *fmt, ...)
{
    b32 parsed = 1;
    F4_Index_ParseCtx ctx_restore = *ctx;
    F4_Index_TokenSkipFlags flags = F4_Index_TokenSkipFlag_SkipWhitespace;

    va_list args;
    va_start(args, fmt);
    for(int i = 0; fmt[i];)
    {
        if(fmt[i] == '%')
        {
            switch(fmt[i+1])
            {
                case 't':
                {
                    char *cstring = va_arg(args, char *);
                    u64 len = 0; while(cstring[len]) len += 1;
                    String_Const_u8 s = { (u8*)cstring, len };
                    parsed = parsed && F4_Index_RequireToken(ctx, s, flags);
                }break;
                case 'k':
                {
                    Token_Base_Kind kind = (Token_Base_Kind)va_arg(args, int);
                    Token **out = va_arg(args, Token **);
                    parsed = parsed && F4_Index_RequireTokenKind(ctx, kind, out, flags);
                }break;
                case 'n':
                {
                    F4_Index_NoteKind want = (F4_Index_NoteKind)va_arg(args, int);
                    F4_Index_Note **out_note = va_arg(args, F4_Index_Note **);
                    Token *token = 0;
                    parsed = parsed && F4_Index_RequireTokenKind(ctx, TokenBaseKind_Identifier, &token, flags);
                    parsed = parsed && !!token;
                    if(parsed)
                    {
                        String_Const_u8 ts = F4_Index_StringFromToken(ctx, token);
                        F4_Index_Note *note = F4_Index_LookupNote(ts);
                        b32 kind_match = 0;
                        for(F4_Index_Note *n = note; n; n = n->next)
                        {
                            if(n->kind == want) { kind_match = 1; note = n; break; }
                        }
                        if(note && kind_match) { if(out_note) *out_note = note; }
                        else { parsed = 0; }
                    }
                }break;
                default: break;
            }
            i += 2;
        }
        else
        {
            i += 1;
        }
    }
    va_end(args);

    if(parsed == 0)
    {
        *ctx = ctx_restore;
    }
    return parsed;
}

//~ The hook macro, now that the ctx type exists.
#define F4_LANGUAGE_INDEXFILE(name) void name(F4_Index_ParseCtx *ctx)

#endif // FCODER_FLEURY_LEXER_TEST_INDEX_H
