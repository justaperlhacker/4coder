/* date = October 12th 2026

   cases_python.h - Test data for the Python hand-written lexer.

   Adding a new case:
     1. Define a static TokExpect array listing what you expect to see.
     2. Add a CASE(...) row to py_cases[] referring to it.

   Whitespace and EOF are ignored during comparison so arrays can be terse. */

#ifndef FCODER_FLEURY_LEXER_TEST_CASES_PYTHON_H
#define FCODER_FLEURY_LEXER_TEST_CASES_PYTHON_H

#include "lexer_test_framework.h"

//~ ------------------------------------------------------------------------
//~ Keywords
//~ ------------------------------------------------------------------------

static TokExpect py_keywords_def[] = {
    EXPECT(TokenBaseKind_Keyword, "def"),
    EXPECT(TokenBaseKind_Identifier, "greet"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "name"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_StatementClose, ":"),
};

static TokExpect py_keywords_class[] = {
    EXPECT(TokenBaseKind_Keyword, "class"),
    EXPECT(TokenBaseKind_Identifier, "Foo"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "object"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_StatementClose, ":"),
};

static TokExpect py_keywords_flow[] = {
    EXPECT(TokenBaseKind_Keyword, "if"),
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_Keyword, "in"),
    EXPECT(TokenBaseKind_Identifier, "y"),
    EXPECT(TokenBaseKind_StatementClose, ":"),
    EXPECT(TokenBaseKind_Keyword, "return"),
    EXPECT(TokenBaseKind_Keyword, "True"),
};

static TokExpect py_keywords_all[] = {
    EXPECT(TokenBaseKind_Keyword, "False"),
    EXPECT(TokenBaseKind_Keyword, "None"),
    EXPECT(TokenBaseKind_Keyword, "True"),
    EXPECT(TokenBaseKind_Keyword, "and"),
    EXPECT(TokenBaseKind_Keyword, "as"),
    EXPECT(TokenBaseKind_Keyword, "assert"),
    EXPECT(TokenBaseKind_Keyword, "async"),
    EXPECT(TokenBaseKind_Keyword, "await"),
    EXPECT(TokenBaseKind_Keyword, "break"),
    EXPECT(TokenBaseKind_Keyword, "class"),
    EXPECT(TokenBaseKind_Keyword, "continue"),
    EXPECT(TokenBaseKind_Keyword, "def"),
    EXPECT(TokenBaseKind_Keyword, "del"),
    EXPECT(TokenBaseKind_Keyword, "elif"),
    EXPECT(TokenBaseKind_Keyword, "else"),
    EXPECT(TokenBaseKind_Keyword, "except"),
    EXPECT(TokenBaseKind_Keyword, "finally"),
    EXPECT(TokenBaseKind_Keyword, "for"),
    EXPECT(TokenBaseKind_Keyword, "from"),
    EXPECT(TokenBaseKind_Keyword, "global"),
    EXPECT(TokenBaseKind_Keyword, "if"),
    EXPECT(TokenBaseKind_Keyword, "import"),
    EXPECT(TokenBaseKind_Keyword, "in"),
    EXPECT(TokenBaseKind_Keyword, "is"),
    EXPECT(TokenBaseKind_Keyword, "lambda"),
    EXPECT(TokenBaseKind_Keyword, "nonlocal"),
    EXPECT(TokenBaseKind_Keyword, "not"),
    EXPECT(TokenBaseKind_Keyword, "or"),
    EXPECT(TokenBaseKind_Keyword, "pass"),
    EXPECT(TokenBaseKind_Keyword, "raise"),
    EXPECT(TokenBaseKind_Keyword, "return"),
    EXPECT(TokenBaseKind_Keyword, "try"),
    EXPECT(TokenBaseKind_Keyword, "while"),
    EXPECT(TokenBaseKind_Keyword, "with"),
    EXPECT(TokenBaseKind_Keyword, "yield"),
};

static TokExpect py_not_keyword_print[] = {
    //~ "print", "type" and "list" are builtins — should stay Identifier.
    EXPECT(TokenBaseKind_Identifier, "print"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_Identifier, "type"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "y"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
};

//~ ------------------------------------------------------------------------
//~ Comments
//~ ------------------------------------------------------------------------

static TokExpect py_line_comment[] = {
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Comment, "# a comment"),
};

static TokExpect py_shebang[] = {
    EXPECT(TokenBaseKind_Comment, "#!/usr/bin/env python3"),
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
};

//~ ------------------------------------------------------------------------
//~ Strings
//~ ------------------------------------------------------------------------

static TokExpect py_str_single[] = {
    EXPECT(TokenBaseKind_LiteralString, "'hello'"),
};

static TokExpect py_str_double[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"hello\""),
};

static TokExpect py_str_escape[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"a\\\"b\""),
};

static TokExpect py_str_raw[] = {
    EXPECT(TokenBaseKind_LiteralString, "r\"back\\\\slash\""),
};

static TokExpect py_str_bytes[] = {
    EXPECT(TokenBaseKind_LiteralString, "b'\\x00'"),
};

static TokExpect py_str_unicode[] = {
    EXPECT(TokenBaseKind_LiteralString, "u\"caf\\xc3\\xa9\""),
};

static TokExpect py_str_f[] = {
    EXPECT(TokenBaseKind_Identifier, "x"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Identifier, "s"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralString, "f\"hi {x} !\""),
};

static TokExpect py_str_rf[] = {
    EXPECT(TokenBaseKind_LiteralString, "rf\"a{b}\\c\""),
};

static TokExpect py_str_fr[] = {
    EXPECT(TokenBaseKind_LiteralString, "Fr\"hi\\x{y}\""),
};

static TokExpect py_str_triple_doc[] = {
    EXPECT(TokenBaseKind_Keyword, "class"),
    EXPECT(TokenBaseKind_Identifier, "C"),
    EXPECT(TokenBaseKind_StatementClose, ":"),
    EXPECT(TokenBaseKind_LiteralString, "\"\"\"docstring\"\"\""),
};

static TokExpect py_str_triple_multiline[] = {
    EXPECT(TokenBaseKind_LiteralString,
           "\"\"\"\nline one\nline \"quoted\" two\n\"\"\""),
};

static TokExpect py_str_triple_single[] = {
    EXPECT(TokenBaseKind_LiteralString, "'''it\\'s fine'''"),
};

static TokExpect py_str_undertermined[] = {
    //~ Unterminated string at end-of-line stops at the newline, not at EOF.
    EXPECT(TokenBaseKind_Identifier, "s"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralString, "\"oops"),
    EXPECT(TokenBaseKind_Identifier, "x"),
};

//~ ------------------------------------------------------------------------
//~ Numbers
//~ ------------------------------------------------------------------------

static TokExpect py_numbers_ints[] = {
    EXPECT(TokenBaseKind_LiteralInteger, "42"),
    EXPECT(TokenBaseKind_LiteralInteger, "0"),
    EXPECT(TokenBaseKind_LiteralInteger, "1_000_000"),
    EXPECT(TokenBaseKind_LiteralInteger, "0b1010"),
    EXPECT(TokenBaseKind_LiteralInteger, "0B1010"),
    EXPECT(TokenBaseKind_LiteralInteger, "0o777"),
    EXPECT(TokenBaseKind_LiteralInteger, "0O17"),
    EXPECT(TokenBaseKind_LiteralInteger, "0xDEADBEEF"),
    EXPECT(TokenBaseKind_LiteralInteger, "0Xdeadbeef"),
};

static TokExpect py_numbers_floats[] = {
    EXPECT(TokenBaseKind_LiteralFloat, "3.14"),
    EXPECT(TokenBaseKind_LiteralFloat, "1e10"),
    EXPECT(TokenBaseKind_LiteralFloat, "1E10"),
    EXPECT(TokenBaseKind_LiteralFloat, "2.5e-3"),
    EXPECT(TokenBaseKind_LiteralFloat, ".5"),
    EXPECT(TokenBaseKind_LiteralFloat, "1."),
};

static TokExpect py_numbers_imaginary[] = {
    EXPECT(TokenBaseKind_LiteralInteger, "3j"),
    EXPECT(TokenBaseKind_LiteralFloat, "1.5J"),
};

//~ ------------------------------------------------------------------------
//~ Operators / punctuation
//~ ------------------------------------------------------------------------

static TokExpect py_ops_arith[] = {
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "+"),
    EXPECT(TokenBaseKind_Identifier, "b"),
    EXPECT(TokenBaseKind_Operator, "-"),
    EXPECT(TokenBaseKind_Identifier, "c"),
    EXPECT(TokenBaseKind_Operator, "*"),
    EXPECT(TokenBaseKind_Identifier, "d"),
    EXPECT(TokenBaseKind_Operator, "**"),
    EXPECT(TokenBaseKind_Identifier, "e"),
    EXPECT(TokenBaseKind_Operator, "//"),
    EXPECT(TokenBaseKind_Identifier, "f"),
    EXPECT(TokenBaseKind_Operator, "%"),
    EXPECT(TokenBaseKind_Identifier, "g"),
};

static TokExpect py_ops_compare[] = {
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "=="),
    EXPECT(TokenBaseKind_Identifier, "b"),
    EXPECT(TokenBaseKind_Operator, "!="),
    EXPECT(TokenBaseKind_Identifier, "c"),
    EXPECT(TokenBaseKind_Operator, ">="),
    EXPECT(TokenBaseKind_Identifier, "d"),
    EXPECT(TokenBaseKind_Operator, "<="),
    EXPECT(TokenBaseKind_Identifier, "e"),
};

static TokExpect py_ops_walrus[] = {
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "y"),
    EXPECT(TokenBaseKind_Operator, ":="),
    EXPECT(TokenBaseKind_LiteralInteger, "5"),
    EXPECT(TokenBaseKind_Operator, ">"),
    EXPECT(TokenBaseKind_LiteralInteger, "0"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
};

static TokExpect py_ops_bitwise[] = {
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "&"),
    EXPECT(TokenBaseKind_Identifier, "b"),
    EXPECT(TokenBaseKind_Operator, "|"),
    EXPECT(TokenBaseKind_Identifier, "c"),
    EXPECT(TokenBaseKind_Operator, "^"),
    EXPECT(TokenBaseKind_Identifier, "d"),
    EXPECT(TokenBaseKind_Operator, "<<"),
    EXPECT(TokenBaseKind_Identifier, "e"),
    EXPECT(TokenBaseKind_Operator, ">>"),
    EXPECT(TokenBaseKind_Identifier, "f"),
    EXPECT(TokenBaseKind_Operator, "~"),
    EXPECT(TokenBaseKind_Identifier, "g"),
};

static TokExpect py_punc[] = {
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "["),
    EXPECT(TokenBaseKind_ParentheticalClose, "]"),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
    EXPECT(TokenBaseKind_StatementClose, ","),
    EXPECT(TokenBaseKind_StatementClose, ":"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Operator, "."),
};

static TokExpect py_decorated[] = {
    EXPECT(TokenBaseKind_Operator, "@"),
    EXPECT(TokenBaseKind_Identifier, "staticmethod"),
    EXPECT(TokenBaseKind_Keyword, "def"),
    EXPECT(TokenBaseKind_Identifier, "foo"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_StatementClose, ":"),
};

//~ ------------------------------------------------------------------------
//~ Special cases
//~ ------------------------------------------------------------------------

static TokExpect py_continuation[] = {
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Operator, "+"),
    EXPECT(TokenBaseKind_LiteralInteger, "2"),
};

static TokExpect py_identifier_unicode[] = {
    EXPECT(TokenBaseKind_Identifier, "caf\xc3\xa9"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
};

//~ ------------------------------------------------------------------------
//~ Case table
//~ ------------------------------------------------------------------------

static Case py_cases[] = {
    CASE("keywords_def",        "def greet(name):\n", py_keywords_def),
    CASE("keywords_class",      "class Foo(object):\n", py_keywords_class),
    CASE("keywords_flow",       "if x in y:\n    return True\n", py_keywords_flow),
    CASE("keywords_all",        "False None True and as assert async await break class continue def del elif else except finally for from global if import in is lambda nonlocal not or pass raise return try while with yield\n", py_keywords_all),
    CASE("not_keyword_print",   "print(x) type(y)\n", py_not_keyword_print),
    CASE("line_comment",        "x = 1  # a comment\n", py_line_comment),
    CASE("shebang",             "#!/usr/bin/env python3\nx = 1\n", py_shebang),
    CASE("str_single",          "'hello'\n", py_str_single),
    CASE("str_double",          "\"hello\"\n", py_str_double),
    CASE("str_escape",          "\"a\\\"b\"\n", py_str_escape),
    CASE("str_raw",             "r\"back\\\\slash\"\n", py_str_raw),
    CASE("str_bytes",           "b'\\x00'\n", py_str_bytes),
    CASE("str_unicode",         "u\"caf\\xc3\\xa9\"\n", py_str_unicode),
    CASE("str_fstring",         "x = 1\ns = f\"hi {x} !\"\n", py_str_f),
    CASE("str_rf_prefix",       "rf\"a{b}\\c\"\n", py_str_rf),
    CASE("str_fr_prefix",       "Fr\"hi\\x{y}\"\n", py_str_fr),
    CASE("str_triple_doc",      "class C:\n    \"\"\"docstring\"\"\"\n", py_str_triple_doc),
    CASE("str_triple_multiline","\"\"\"\nline one\nline \"quoted\" two\n\"\"\"\n", py_str_triple_multiline),
    CASE("str_triple_single",   "'''it\\'s fine'''\n", py_str_triple_single),
    CASE("str_unterminated",    "s = \"oops\nx\n", py_str_undertermined),
    CASE("numbers_ints",        "42 0 1_000_000 0b1010 0B1010 0o777 0O17 0xDEADBEEF 0Xdeadbeef\n", py_numbers_ints),
    CASE("numbers_floats",      "3.14 1e10 1E10 2.5e-3 .5 1.\n", py_numbers_floats),
    CASE("numbers_imaginary",   "3j 1.5J\n", py_numbers_imaginary),
    CASE("operators_arith",     "a + b - c * d ** e // f % g\n", py_ops_arith),
    CASE("operators_compare",   "a == b != c >= d <= e\n", py_ops_compare),
    CASE("operators_walrus",    "(y := 5 > 0)\n", py_ops_walrus),
    CASE("operators_bitwise",   "a & b | c ^ d << e >> f ~ g\n", py_ops_bitwise),
    CASE("punctuation",         "()[]{},:;.\n", py_punc),
    CASE("decorator",           "@staticmethod\ndef foo():\n", py_decorated),
    CASE("line_continuation",   "a = 1 + \\\n    2\n", py_continuation),
    CASE("identifier_unicode",  "caf\xc3\xa9 = 1\n", py_identifier_unicode),
    CASE0("empty_file",         ""),
    CASE0("whitespace_only",    "  \n\t\n"),
};

static Suite py_suite = {
    "python",
    (LexInitFn)F4_Py_LexInit,
    (LexFullInputFn)F4_Py_LexFullInput,
    sizeof(F4_Py_LexerState),
    py_cases, ArrayCount(py_cases),
};

#endif // FCODER_FLEURY_LEXER_TEST_CASES_PYTHON_H
