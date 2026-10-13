/* date = October 12th 2026

   cases_lua.h - Test data for the Lua hand-written lexer. */

#ifndef FCODER_FLEURY_LEXER_TEST_CASES_LUA_H
#define FCODER_FLEURY_LEXER_TEST_CASES_LUA_H

#include "lexer_test_framework.h"

//~ ------------------------------------------------------------------------
//~ Keywords
//~ ------------------------------------------------------------------------

static TokExpect lua_keywords_local_function[] = {
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_Keyword, "nil"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Keyword, "function"),
    EXPECT(TokenBaseKind_Identifier, "f"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_Keyword, "return"),
    EXPECT(TokenBaseKind_Keyword, "true"),
    EXPECT(TokenBaseKind_Keyword, "end"),
};

static TokExpect lua_keywords_if_then[] = {
    EXPECT(TokenBaseKind_Keyword, "if"),
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_Operator, "=="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Keyword, "then"),
    EXPECT(TokenBaseKind_Keyword, "elseif"),
    EXPECT(TokenBaseKind_Keyword, "else"),
    EXPECT(TokenBaseKind_Keyword, "end"),
};

static TokExpect lua_keywords_all[] = {
    EXPECT(TokenBaseKind_Keyword, "and"),
    EXPECT(TokenBaseKind_Keyword, "break"),
    EXPECT(TokenBaseKind_Keyword, "do"),
    EXPECT(TokenBaseKind_Keyword, "else"),
    EXPECT(TokenBaseKind_Keyword, "elseif"),
    EXPECT(TokenBaseKind_Keyword, "end"),
    EXPECT(TokenBaseKind_Keyword, "false"),
    EXPECT(TokenBaseKind_Keyword, "for"),
    EXPECT(TokenBaseKind_Keyword, "function"),
    EXPECT(TokenBaseKind_Keyword, "goto"),
    EXPECT(TokenBaseKind_Keyword, "if"),
    EXPECT(TokenBaseKind_Keyword, "in"),
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Keyword, "nil"),
    EXPECT(TokenBaseKind_Keyword, "not"),
    EXPECT(TokenBaseKind_Keyword, "or"),
    EXPECT(TokenBaseKind_Keyword, "repeat"),
    EXPECT(TokenBaseKind_Keyword, "return"),
    EXPECT(TokenBaseKind_Keyword, "then"),
    EXPECT(TokenBaseKind_Keyword, "true"),
    EXPECT(TokenBaseKind_Keyword, "until"),
    EXPECT(TokenBaseKind_Keyword, "while"),
};

//~ ------------------------------------------------------------------------
//~ Comments
//~ ------------------------------------------------------------------------

static TokExpect lua_line_comment[] = {
    EXPECT(TokenBaseKind_Comment, "-- just a comment"),
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "x"),
};

static TokExpect lua_shebang[] = {
    EXPECT(TokenBaseKind_Comment, "#!/usr/bin/env lua"),
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "x"),
};

static TokExpect lua_block_comment[] = {
    EXPECT(TokenBaseKind_Comment, "--[[ multi\nline\ncomment ]]"),
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "x"),
};

static TokExpect lua_block_comment_level[] = {
    EXPECT(TokenBaseKind_Comment, "--[==[ hello ]==]"),
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "y"),
};

static TokExpect lua_block_comment_with_dashes[] = {
    EXPECT(TokenBaseKind_Comment, "--[[ -- not an inner comment -- ]]"),
    EXPECT(TokenBaseKind_Identifier, "x"),
};

static TokExpect lua_double_dash_in_string[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"-- not a comment --\""),
};

//~ ------------------------------------------------------------------------
//~ Strings
//~ ------------------------------------------------------------------------

static TokExpect lua_string_double[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"hello\""),
};

static TokExpect lua_string_single[] = {
    EXPECT(TokenBaseKind_LiteralString, "'hello'"),
};

static TokExpect lua_string_escape_tab[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"a\\tb\\nc\""),
};

static TokExpect lua_string_escape_quote[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"say \\\"hi\\\"\""),
};

static TokExpect lua_long_string[] = {
    EXPECT(TokenBaseKind_LiteralString, "[[ multi\nline\n ]]"),
};

static TokExpect lua_long_string_level[] = {
    EXPECT(TokenBaseKind_LiteralString, "[==[ with ]] inside ]==]"),
};

static TokExpect lua_long_string_deep_level[] = {
    EXPECT(TokenBaseKind_LiteralString, "[===[ still ]] inside ]===]"),
};

//~ ------------------------------------------------------------------------
//~ Numbers
//~ ------------------------------------------------------------------------

static TokExpect lua_numbers[] = {
    EXPECT(TokenBaseKind_LiteralInteger, "42"),
    EXPECT(TokenBaseKind_LiteralFloat, "3.14"),
    EXPECT(TokenBaseKind_LiteralFloat, "1e10"),
    EXPECT(TokenBaseKind_LiteralFloat, "1E10"),
    EXPECT(TokenBaseKind_LiteralFloat, "2.5e-3"),
    EXPECT(TokenBaseKind_LiteralFloat, ".5"),
    EXPECT(TokenBaseKind_LiteralFloat, "1."),
    EXPECT(TokenBaseKind_LiteralInteger, "0xff"),
    EXPECT(TokenBaseKind_LiteralInteger, "0XdeadBEEF"),
    EXPECT(TokenBaseKind_LiteralFloat, "0x1p4"),
    EXPECT(TokenBaseKind_LiteralFloat, "0x1.8p3"),
};

//~ ------------------------------------------------------------------------
//~ Operators / punctuation
//~ ------------------------------------------------------------------------

static TokExpect lua_concat[] = {
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "s"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralString, "\"a\""),
    EXPECT(TokenBaseKind_Operator, ".."),
    EXPECT(TokenBaseKind_LiteralString, "\"b\""),
};

static TokExpect lua_vararg[] = {
    EXPECT(TokenBaseKind_Keyword, "function"),
    EXPECT(TokenBaseKind_Identifier, "f"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_StatementClose, ","),
    EXPECT(TokenBaseKind_Operator, "..."),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_Keyword, "end"),
};

static TokExpect lua_length_operator[] = {
    //~ '#' is the length operator in Lua — NOT a comment marker.
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Identifier, "n"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_Operator, "#"),
    EXPECT(TokenBaseKind_Identifier, "t"),
};

static TokExpect lua_method_call[] = {
    EXPECT(TokenBaseKind_Identifier, "obj"),
    EXPECT(TokenBaseKind_Operator, ":"),
    EXPECT(TokenBaseKind_Identifier, "method"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
};

static TokExpect lua_goto_label[] = {
    EXPECT(TokenBaseKind_Operator, "::"),
    EXPECT(TokenBaseKind_Identifier, "loop"),
    EXPECT(TokenBaseKind_Operator, "::"),
};

static TokExpect lua_table_literal[] = {
    EXPECT(TokenBaseKind_Identifier, "t"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_StatementClose, ","),
    EXPECT(TokenBaseKind_LiteralString, "\"two\""),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
};

static TokExpect lua_compare_ops[] = {
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "=="),
    EXPECT(TokenBaseKind_Identifier, "b"),
    EXPECT(TokenBaseKind_Operator, "~="),
    EXPECT(TokenBaseKind_Identifier, "c"),
    EXPECT(TokenBaseKind_Operator, "<="),
    EXPECT(TokenBaseKind_Identifier, "d"),
    EXPECT(TokenBaseKind_Operator, ">="),
    EXPECT(TokenBaseKind_Identifier, "e"),
    EXPECT(TokenBaseKind_Operator, "<"),
    EXPECT(TokenBaseKind_Identifier, "f"),
    EXPECT(TokenBaseKind_Operator, ">"),
    EXPECT(TokenBaseKind_Identifier, "g"),
};

static TokExpect lua_arith_ops[] = {
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Operator, "+"),
    EXPECT(TokenBaseKind_LiteralInteger, "2"),
    EXPECT(TokenBaseKind_Operator, "-"),
    EXPECT(TokenBaseKind_LiteralInteger, "3"),
    EXPECT(TokenBaseKind_Operator, "*"),
    EXPECT(TokenBaseKind_LiteralInteger, "4"),
    EXPECT(TokenBaseKind_Operator, "/"),
    EXPECT(TokenBaseKind_LiteralInteger, "5"),
    EXPECT(TokenBaseKind_Operator, "//"),
    EXPECT(TokenBaseKind_LiteralInteger, "6"),
    EXPECT(TokenBaseKind_Operator, "%"),
    EXPECT(TokenBaseKind_LiteralInteger, "7"),
    EXPECT(TokenBaseKind_Operator, "^"),
    EXPECT(TokenBaseKind_LiteralInteger, "8"),
};

static TokExpect lua_brackets[] = {
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "["),
    EXPECT(TokenBaseKind_ParentheticalClose, "]"),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
    EXPECT(TokenBaseKind_StatementClose, ","),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Operator, "."),
};

//~ ------------------------------------------------------------------------
//~ Case table
//~ ------------------------------------------------------------------------

static Case lua_cases[] = {
    CASE("keywords_local_function", "local x = nil; function f() return true end\n", lua_keywords_local_function),
    CASE("keywords_if_then_end",    "if x == 1 then elseif else end\n", lua_keywords_if_then),
    CASE("keywords_all",            "and break do else elseif end false for function goto if in local nil not or repeat return then true until while\n", lua_keywords_all),
    CASE("line_comment",            "-- just a comment\nlocal x\n", lua_line_comment),
    CASE("shebang",                 "#!/usr/bin/env lua\nlocal x\n", lua_shebang),
    CASE("block_comment",           "--[[ multi\nline\ncomment ]]\nlocal x\n", lua_block_comment),
    CASE("block_comment_level",     "--[==[ hello ]==]\nlocal y\n", lua_block_comment_level),
    CASE("block_comment_dashes",    "--[[ -- not an inner comment -- ]]\nx\n", lua_block_comment_with_dashes),
    CASE("double_dash_in_string",   "\"-- not a comment --\"\n", lua_double_dash_in_string),
    CASE("string_double",           "\"hello\"\n", lua_string_double),
    CASE("string_single",           "'hello'\n", lua_string_single),
    CASE("string_escape_tab",       "\"a\\tb\\nc\"\n", lua_string_escape_tab),
    CASE("string_escape_quote",     "\"say \\\"hi\\\"\"\n", lua_string_escape_quote),
    CASE("long_string",             "[[ multi\nline\n ]]\n", lua_long_string),
    CASE("long_string_level",       "[==[ with ]] inside ]==]\n", lua_long_string_level),
    CASE("long_string_deep",        "[===[ still ]] inside ]===]\n", lua_long_string_deep_level),
    CASE("numbers",                 "42 3.14 1e10 1E10 2.5e-3 .5 1. 0xff 0XdeadBEEF 0x1p4 0x1.8p3\n", lua_numbers),
    CASE("concat_operator",         "local s = \"a\" .. \"b\"\n", lua_concat),
    CASE("vararg_dots",             "function f(a, ...) end\n", lua_vararg),
    CASE("length_operator_hash",    "local n = #t\n", lua_length_operator),
    CASE("method_call_colon",       "obj:method()\n", lua_method_call),
    CASE("goto_label",              "::loop::\n", lua_goto_label),
    CASE("table_literal",           "t = { a = 1, \"two\" }\n", lua_table_literal),
    CASE("compare_operators",       "a == b ~= c <= d >= e < f > g\n", lua_compare_ops),
    CASE("arith_operators",         "1 + 2 - 3 * 4 / 5 // 6 % 7 ^ 8\n", lua_arith_ops),
    CASE("brackets_punctuation",    "()[]{},;.\n", lua_brackets),
    CASE0("empty_file",             ""),
    CASE0("whitespace_only",        "  \n\t\n"),
};

static Suite lua_suite = {
    "lua",
    (LexInitFn)F4_Lua_LexInit,
    (LexFullInputFn)F4_Lua_LexFullInput,
    sizeof(F4_Lua_LexerState),
    lua_cases, ArrayCount(lua_cases),
};

#endif // FCODER_FLEURY_LEXER_TEST_CASES_LUA_H
