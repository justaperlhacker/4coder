/* date = October 12th 2026

   lexer_test_framework.h - The reusable machinery for driving a language
   lexer with a table of snippets and asserting on the resulting token stream.

   Author model:
     1. Write a table of TokExpect arrays: one per test case, listing the
        semantic tokens (kind + exact bytes) you expect to see in order.
     2. Write a table of Case structs that pair a name, a source string and
        an expectations array.
     3. Bundle them into a Suite describing how to run the lexer, and call
        run_suite() from main.

   Whitespace and EOF tokens produced by the lexer are ignored during
   comparison, so authors only need to enumerate the interesting tokens.

   Adding a new language is just a new "cases_<lang>.h" that follows the same
   pattern; the framework doesn't need to change. */

#ifndef FCODER_FLEURY_LEXER_TEST_FRAMEWORK_H
#define FCODER_FLEURY_LEXER_TEST_FRAMEWORK_H

#include "lexer_test_stubs.h"

typedef void (*LexInitFn)(void *, String_Const_u8);
typedef b32 (*LexFullInputFn)(Arena *, Token_List *, void *, u64);
typedef void (*IndexFileFn)(F4_Index_ParseCtx *);

struct TokExpect{
    Token_Base_Kind kind;
    const char *text;
};

struct Case{
    const char *name;
    const char *src;
    TokExpect *expects;
    int expect_count;
};

//~ An expected indexed symbol: name text + the note kind the IndexFile should
//~ file it under. Order does not matter; the runner compares sets.
struct IndexExpect{
    F4_Index_NoteKind kind;
    const char *name;
};

struct IndexCase{
    const char *name;
    const char *src;
    IndexExpect *expects;
    int expect_count;
};

struct Suite{
    const char *language;
    LexInitFn LexInit;
    LexFullInputFn LexFullInput;
    size_t state_size;
    Case *cases;
    int case_count;
    IndexFileFn IndexFile;
    IndexCase *index_cases;
    int index_case_count;
};

//~ Shorthand for entries inside an IndexExpect array.
#define IEXPECT(kind, name) { (kind), (name) }

//~ Shorthand for a case that asserts the set of indexed symbols.
#define ICASE(cname, csrc, syms) \
    { (cname), (csrc), (syms), ArrayCount(syms) }

//~ Shorthand for the entries inside a TokExpect array.
#define EXPECT(kind, text) { (kind), (text) }

//~ Shorthand for a case that expects one or more tokens.
#define CASE(cname, csrc, toks) \
    { (cname), (csrc), (toks), ArrayCount(toks) }

//~ Shorthand for a case that should produce zero semantic tokens (empty
//~ file, pure whitespace, etc.).
#define CASE0(cname, csrc) \
    { (cname), (csrc), 0, 0 }

static int g_lex_pass = 0;
static int g_lex_fail = 0;
static int g_lex_verbose = 0;

//~ Names for the token kinds — used in failure output.
internal const char *
lex_kind_name(Token_Base_Kind k)
{
    switch(k)
    {
        case TokenBaseKind_EOF: return "EOF";
        case TokenBaseKind_Whitespace: return "Whitespace";
        case TokenBaseKind_LexError: return "LexError";
        case TokenBaseKind_Comment: return "Comment";
        case TokenBaseKind_Keyword: return "Keyword";
        case TokenBaseKind_Preprocessor: return "Preprocessor";
        case TokenBaseKind_Identifier: return "Identifier";
        case TokenBaseKind_Operator: return "Operator";
        case TokenBaseKind_LiteralInteger: return "LiteralInteger";
        case TokenBaseKind_LiteralFloat: return "LiteralFloat";
        case TokenBaseKind_LiteralString: return "LiteralString";
        case TokenBaseKind_ScopeOpen: return "ScopeOpen";
        case TokenBaseKind_ScopeClose: return "ScopeClose";
        case TokenBaseKind_ParentheticalOpen: return "ParentheticalOpen";
        case TokenBaseKind_ParentheticalClose: return "ParentheticalClose";
        case TokenBaseKind_StatementClose: return "StatementClose";
        default: return "???";
    }
}

internal void
lex_dump_stream(const char *src)
{
    printf("    full stream:\n");
    TokenNode *n = lex_test_token_stream();
    int idx = 0;
    while(n){
        printf("      %2d: %-18s '%.*s'\n",
               idx, lex_kind_name(n->t.kind),
               (int)n->t.size, src + n->t.pos);
        n = n->next;
        idx += 1;
    }
}

//~ Runs one test case against the given suite and reports pass/fail.
internal void
run_case(Suite *s, Case *c)
{
    lex_test_reset_token_stream();

    String_Const_u8 text = { (u8*)c->src, (u64)strlen(c->src) };
    void *state = malloc(s->state_size);
    memset(state, 0, s->state_size);
    s->LexInit(state, text);
    Token_List list = {};
    b32 reached_eof = s->LexFullInput(&g_arena, &list, state, (u64)-1);
    free(state);

    TokenNode *it = lex_test_token_stream();
    while(it && it->t.kind == TokenBaseKind_Whitespace) it = it->next;

    int exp_i = 0;
    b32 pass = 1;
    const char *reason = "";

    if(c->expect_count == 0)
    {
        if(it && it->t.kind != TokenBaseKind_EOF)
        {
            pass = 0;
            reason = "unexpected token in empty-expect case";
        }
    }
    else
    {
        for(exp_i = 0; exp_i < c->expect_count; exp_i += 1)
        {
            TokExpect e = c->expects[exp_i];
            if(!it || it->t.kind == TokenBaseKind_EOF)
            {
                pass = 0;
                reason = "missing token";
                break;
            }
            size_t want_len = strlen(e.text);
            if((u64)it->t.size != want_len ||
               memcmp(c->src + it->t.pos, e.text, want_len) != 0)
            {
                pass = 0;
                reason = "text mismatch";
                break;
            }
            if(it->t.kind != e.kind)
            {
                pass = 0;
                reason = "kind mismatch";
                break;
            }
            it = it->next;
            while(it && it->t.kind == TokenBaseKind_Whitespace) it = it->next;
        }
        if(pass && it && it->t.kind != TokenBaseKind_EOF)
        {
            pass = 0;
            reason = "unexpected extra token";
        }
    }

    if(pass && !reached_eof)
    {
        pass = 0;
        reason = "lexer did not report EOF";
    }

    if(pass)
    {
        g_lex_pass += 1;
        if(g_lex_verbose) printf("  PASS %s.%s\n", s->language, c->name);
    }
    else
    {
        g_lex_fail += 1;
        printf("  FAIL %s.%s: %s\n", s->language, c->name, reason);
        printf("    src: \"%s\"\n", c->src);
        if(c->expect_count > 0)
        {
            if(exp_i < c->expect_count)
            {
                printf("    expected[%d]: %-18s '%s'\n", exp_i,
                       lex_kind_name(c->expects[exp_i].kind),
                       c->expects[exp_i].text);
            }
            else
            {
                printf("    expected[%d]: <none — but got>\n", exp_i);
            }
            if(it && it->t.kind != TokenBaseKind_EOF)
            {
                printf("    actual  [%d]: %-18s '%.*s' @%lld+%.0lld\n", exp_i,
                       lex_kind_name(it->t.kind),
                       (int)it->t.size, c->src + it->t.pos,
                       (long long)it->t.pos, (long long)it->t.size);
            }
            else
            {
                printf("    actual  [%d]: <end of stream>\n", exp_i);
            }
        }
        if(g_lex_verbose) lex_dump_stream(c->src);
    }
}

internal const char *
index_kind_name(F4_Index_NoteKind k)
{
    switch(k)
    {
        case F4_Index_NoteKind_Type: return "Type";
        case F4_Index_NoteKind_Constant: return "Constant";
        case F4_Index_NoteKind_Function: return "Function";
        case F4_Index_NoteKind_Decl: return "Decl";
        case F4_Index_NoteKind_Macro: return "Macro";
        case F4_Index_NoteKind_CommentTag: return "CommentTag";
        case F4_Index_NoteKind_CommentToDo: return "CommentToDo";
        default: return "Other";
    }
}

//~ Snapshot the linked token stream into a contiguous Token_Array that the
//~ index iterator can walk.
internal Token *
lex_test_flatten_tokens(Arena *scratch, i64 *out_count)
{
    i64 n = 0;
    for(TokenNode *it = lex_test_token_stream(); it; it = it->next) n += 1;
    Token *arr = (Token*)malloc(sizeof(Token) * (n > 0 ? n : 1));
    i64 i = 0;
    for(TokenNode *it = lex_test_token_stream(); it; it = it->next, i += 1)
    {
        arr[i] = it->t;
    }
    *out_count = n;
    return arr;
}

internal void
run_index_case(Suite *s, IndexCase *c)
{
    lex_test_reset_token_stream();
    lex_test_reset_index();

    String_Const_u8 text = { (u8*)c->src, (u64)strlen(c->src) };
    void *state = malloc(s->state_size);
    memset(state, 0, s->state_size);
    s->LexInit(state, text);
    Token_List list = {};
    s->LexFullInput(&g_arena, &list, state, (u64)-1);
    free(state);

    i64 count = 0;
    Token *toks = lex_test_flatten_tokens(&g_arena, &count);

    F4_Index_File file = {};
    F4_Index_ParseCtx ctx = {};
    ctx.done = 0;
    ctx.file = &file;
    ctx.string = text;
    ctx.tokens.tokens = toks;
    ctx.tokens.count = count;
    ctx.tokens.max = count;
    ctx.it = token_iterator_pos(0, &ctx.tokens, 0);

    s->IndexFile(&ctx);

    //~ Compare captured notes to the expected set (order independent).
    int matched = 0;
    for(u32 i = 0; i < g_lex_index_note_count; i += 1)
    {
        F4_Index_Note *note = &g_lex_index_notes[i];
        b32 hit = 0;
        for(int e = 0; e < c->expect_count; e += 1)
        {
            String_Const_u8 want = { (u8*)c->expects[e].name, (u64)strlen(c->expects[e].name) };
            if(c->expects[e].kind == note->kind && string_match(want, note->string))
            {
                hit = 1;
                break;
            }
        }
        if(hit) matched += 1;
    }
    b32 pass = (matched == c->expect_count) && (g_lex_index_note_count == (u32)c->expect_count);

    if(pass)
    {
        g_lex_pass += 1;
        if(g_lex_verbose) printf("  PASS %s.index.%s\n", s->language, c->name);
    }
    else
    {
        g_lex_fail += 1;
        printf("  FAIL %s.index.%s: expected %d symbols, got %d (%d matched)\n",
               s->language, c->name, c->expect_count, (int)g_lex_index_note_count, matched);
        printf("    expected:");
        for(int e = 0; e < c->expect_count; e += 1)
            printf(" [%s:%s]", index_kind_name(c->expects[e].kind), c->expects[e].name);
        printf("\n    got:     ");
        for(u32 i = 0; i < g_lex_index_note_count; i += 1)
            printf(" [%s:%.*s]", index_kind_name(g_lex_index_notes[i].kind),
                   (int)g_lex_index_notes[i].string.size, g_lex_index_notes[i].string.str);
        printf("\n");
    }

    free(toks);
}

internal void
run_suite(Suite *s)
{
    printf("== %s ==\n", s->language);
    for(int i = 0; i < s->case_count; i += 1)
    {
        run_case(s, &s->cases[i]);
    }
    for(int i = 0; i < s->index_case_count; i += 1)
    {
        run_index_case(s, &s->index_cases[i]);
    }
}

#endif // FCODER_FLEURY_LEXER_TEST_FRAMEWORK_H
