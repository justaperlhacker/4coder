/* date = October 12th 2026

   cases_perl.h - Test data for the Perl hand-written lexer.

   Focus areas:
     - Sigil resolution per perldata: $ @ % & * are sigils when they prefix
       a name, a block (${...}), a package chain (Foo::Bar) or a $-special.
     - Ambiguous characters: % * & flip to operators when the previous
       semantic token ended a term (identifier, number, close bracket...).
     - Reference operator \ never ends a term, so \$x / \%h / &sub after it
       still read as variables.
     - Pod blocks, shebang, and single-line '#' comments. */

#ifndef FCODER_FLEURY_LEXER_TEST_CASES_PERL_H
#define FCODER_FLEURY_LEXER_TEST_CASES_PERL_H

#include "lexer_test_framework.h"

//~ ------------------------------------------------------------------------
//~ Keywords
//~ ------------------------------------------------------------------------

static TokExpect perl_keywords_basic[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$x"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Keyword, "sub"),
};

static TokExpect perl_keywords_all[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Keyword, "our"),
    EXPECT(TokenBaseKind_Keyword, "local"),
    EXPECT(TokenBaseKind_Keyword, "sub"),
    EXPECT(TokenBaseKind_Keyword, "use"),
    EXPECT(TokenBaseKind_Keyword, "no"),
    EXPECT(TokenBaseKind_Keyword, "package"),
    EXPECT(TokenBaseKind_Keyword, "require"),
    EXPECT(TokenBaseKind_Keyword, "if"),
    EXPECT(TokenBaseKind_Keyword, "elsif"),
    EXPECT(TokenBaseKind_Keyword, "else"),
    EXPECT(TokenBaseKind_Keyword, "unless"),
    EXPECT(TokenBaseKind_Keyword, "while"),
    EXPECT(TokenBaseKind_Keyword, "until"),
    EXPECT(TokenBaseKind_Keyword, "for"),
    EXPECT(TokenBaseKind_Keyword, "foreach"),
    EXPECT(TokenBaseKind_Keyword, "do"),
    EXPECT(TokenBaseKind_Keyword, "last"),
    EXPECT(TokenBaseKind_Keyword, "next"),
    EXPECT(TokenBaseKind_Keyword, "redo"),
    EXPECT(TokenBaseKind_Keyword, "return"),
    EXPECT(TokenBaseKind_Keyword, "given"),
    EXPECT(TokenBaseKind_Keyword, "when"),
    EXPECT(TokenBaseKind_Keyword, "default"),
    EXPECT(TokenBaseKind_Keyword, "print"),
    EXPECT(TokenBaseKind_Keyword, "printf"),
    EXPECT(TokenBaseKind_Keyword, "say"),
    EXPECT(TokenBaseKind_Keyword, "sort"),
    EXPECT(TokenBaseKind_Keyword, "map"),
    EXPECT(TokenBaseKind_Keyword, "grep"),
    EXPECT(TokenBaseKind_Keyword, "defined"),
    EXPECT(TokenBaseKind_Keyword, "undef"),
    EXPECT(TokenBaseKind_Keyword, "eval"),
    EXPECT(TokenBaseKind_Keyword, "BEGIN"),
    EXPECT(TokenBaseKind_Keyword, "END"),
    EXPECT(TokenBaseKind_Keyword, "CHECK"),
    EXPECT(TokenBaseKind_Keyword, "INIT"),
    EXPECT(TokenBaseKind_Keyword, "UNITCHECK"),
    EXPECT(TokenBaseKind_Keyword, "DESTROY"),
    EXPECT(TokenBaseKind_Keyword, "AUTOLOAD"),
    EXPECT(TokenBaseKind_Keyword, "die"),
    EXPECT(TokenBaseKind_Keyword, "warn"),
    EXPECT(TokenBaseKind_Keyword, "exit"),
    EXPECT(TokenBaseKind_Keyword, "and"),
    EXPECT(TokenBaseKind_Keyword, "or"),
    EXPECT(TokenBaseKind_Keyword, "not"),
    EXPECT(TokenBaseKind_Keyword, "eq"),
    EXPECT(TokenBaseKind_Keyword, "ne"),
    EXPECT(TokenBaseKind_Keyword, "lt"),
    EXPECT(TokenBaseKind_Keyword, "gt"),
    EXPECT(TokenBaseKind_Keyword, "le"),
    EXPECT(TokenBaseKind_Keyword, "ge"),
    EXPECT(TokenBaseKind_Keyword, "cmp"),
};

//~ ------------------------------------------------------------------------
//~ Comments and pod
//~ ------------------------------------------------------------------------

static TokExpect perl_line_comment[] = {
    EXPECT(TokenBaseKind_Comment, "# comment"),
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$x"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
};

static TokExpect perl_shebang[] = {
    EXPECT(TokenBaseKind_Comment, "#!/usr/bin/env perl"),
    EXPECT(TokenBaseKind_Keyword, "use"),
    EXPECT(TokenBaseKind_Identifier, "strict"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
};

static TokExpect perl_pod_block[] = {
    EXPECT(TokenBaseKind_Comment,
           "=pod\n\n=head1 NAME\n\nSample\n\n=cut\n"),
    EXPECT(TokenBaseKind_Keyword, "package"),
    EXPECT(TokenBaseKind_Identifier, "main"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
};

static TokExpect perl_comment_with_specials[] = {
    EXPECT(TokenBaseKind_Comment, "# $sigil % hash * glob & sub"),
};

//~ ------------------------------------------------------------------------
//~ Strings
//~ ------------------------------------------------------------------------

static TokExpect perl_string_double[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"hello $name world\""),
};

static TokExpect perl_string_single[] = {
    EXPECT(TokenBaseKind_LiteralString, "'no $interpolation here'"),
};

static TokExpect perl_string_escape[] = {
    EXPECT(TokenBaseKind_LiteralString, "\"say \\\"hi\\\"\""),
};

static TokExpect perl_string_single_escape_quote[] = {
    EXPECT(TokenBaseKind_LiteralString, "'it\\'s'"),
};

//~ ------------------------------------------------------------------------
//~ Sigil forms
//~ ------------------------------------------------------------------------

static TokExpect perl_sigil_scalar[] = {
    EXPECT(TokenBaseKind_Identifier, "$foo"),
};

static TokExpect perl_sigil_array[] = {
    EXPECT(TokenBaseKind_Identifier, "@arr"),
};

static TokExpect perl_sigil_hash[] = {
    EXPECT(TokenBaseKind_Identifier, "%h"),
};

static TokExpect perl_sigil_sub[] = {
    EXPECT(TokenBaseKind_Identifier, "&sub"),
};

static TokExpect perl_sigil_glob[] = {
    EXPECT(TokenBaseKind_Identifier, "*GLOB"),
};

static TokExpect perl_sigil_topic_underscore[] = {
    EXPECT(TokenBaseKind_Identifier, "$_"),
};

static TokExpect perl_sigil_env[] = {
    EXPECT(TokenBaseKind_Identifier, "%ENV"),
};

//~ ------------------------------------------------------------------------
//~ $-only special variables (per perldata)
//~ ------------------------------------------------------------------------

static TokExpect perl_dollar_dollar_pid[] = {
    EXPECT(TokenBaseKind_Identifier, "$$"),
};

static TokExpect perl_dollar_dollar_deref[] = {
    //~ $$ref: the second '$' introduces a name so the first is a deref sigil.
    EXPECT(TokenBaseKind_Identifier, "$"),
    EXPECT(TokenBaseKind_Identifier, "$ref"),
};

static TokExpect perl_dollar_bang[] = {
    EXPECT(TokenBaseKind_Identifier, "$!"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
};

static TokExpect perl_dollar_record_sep[] = {
    EXPECT(TokenBaseKind_Identifier, "$/"),
};

static TokExpect perl_dollar_line_num[] = {
    EXPECT(TokenBaseKind_Identifier, "$."),
};

static TokExpect perl_match_vars[] = {
    EXPECT(TokenBaseKind_Identifier, "$1"),
    EXPECT(TokenBaseKind_Operator, "+"),
    EXPECT(TokenBaseKind_Identifier, "$2"),
};

static TokExpect perl_match_var_two_digit[] = {
    EXPECT(TokenBaseKind_Identifier, "$10"),
};

static TokExpect perl_last_index[] = {
    EXPECT(TokenBaseKind_Identifier, "$#"),
    EXPECT(TokenBaseKind_Identifier, "arr"),
};

static TokExpect perl_caret_var[] = {
    EXPECT(TokenBaseKind_Identifier, "$^V"),
};

//~ ------------------------------------------------------------------------
//~ Package chains and block derefs
//~ ------------------------------------------------------------------------

static TokExpect perl_package_chain[] = {
    EXPECT(TokenBaseKind_Identifier, "$My::Deep::var"),
};

static TokExpect perl_array_pkg[] = {
    EXPECT(TokenBaseKind_Identifier, "@Foo::Bar"),
};

static TokExpect perl_block_deref_only_sigil[] = {
    //~ ${...} — sigil emits bare '$' so '{' opens a ScopeOpen below.
    EXPECT(TokenBaseKind_Identifier, "$"),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_Identifier, "ref"),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
};

static TokExpect perl_hash_deref_arrow[] = {
    EXPECT(TokenBaseKind_Identifier, "$h"),
    EXPECT(TokenBaseKind_Operator, "->"),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_Identifier, "key"),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
};

static TokExpect perl_brace_subscript[] = {
    EXPECT(TokenBaseKind_Identifier, "$h"),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_LiteralString, "\"key\""),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
};

//~ ------------------------------------------------------------------------
//~ Ambiguous % * & — sigil vs binary operator
//~ ------------------------------------------------------------------------

static TokExpect perl_modulo_vs_hash[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$m"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "10"),
    EXPECT(TokenBaseKind_Operator, "%"),
    EXPECT(TokenBaseKind_LiteralInteger, "3"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "%h"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
};

static TokExpect perl_bitwise_vs_sub[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$b"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "6"),
    EXPECT(TokenBaseKind_Operator, "&"),
    EXPECT(TokenBaseKind_LiteralInteger, "3"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Identifier, "&foo"),
};

static TokExpect perl_mult_vs_glob[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$p"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "2"),
    EXPECT(TokenBaseKind_Operator, "*"),
    EXPECT(TokenBaseKind_LiteralInteger, "3"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Identifier, "*STDOUT"),
};

static TokExpect perl_power_operator[] = {
    //~ Two stars after a number are a single exponent operator token.
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$p"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_LiteralInteger, "2"),
    EXPECT(TokenBaseKind_Operator, "**"),
    EXPECT(TokenBaseKind_LiteralInteger, "8"),
};

static TokExpect perl_deref_after_ref[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$r" ),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_Operator, "\\" ),
    EXPECT(TokenBaseKind_Identifier, "$x"),
};

//~ ------------------------------------------------------------------------
//~ Reference operator and its interaction with sigils
//~ ------------------------------------------------------------------------

static TokExpect perl_ref_scalar[] = {
    EXPECT(TokenBaseKind_Operator, "\\"),
    EXPECT(TokenBaseKind_Identifier, "$x"),
};

static TokExpect perl_ref_sub[] = {
    EXPECT(TokenBaseKind_Operator, "\\"),
    EXPECT(TokenBaseKind_Identifier, "&greet"),
};

static TokExpect perl_ref_hash[] = {
    EXPECT(TokenBaseKind_Operator, "\\"),
    EXPECT(TokenBaseKind_Identifier, "%My::Greeter::hash"),
};

static TokExpect perl_ref_array[] = {
    EXPECT(TokenBaseKind_Operator, "\\"),
    EXPECT(TokenBaseKind_Identifier, "@list"),
};

static TokExpect perl_ref_glob[] = {
    EXPECT(TokenBaseKind_Operator, "\\"),
    EXPECT(TokenBaseKind_Identifier, "*STDOUT"),
};

//~ ------------------------------------------------------------------------
//~ Arrows and package separator
//~ ------------------------------------------------------------------------

static TokExpect perl_arrow_method[] = {
    EXPECT(TokenBaseKind_Identifier, "$obj"),
    EXPECT(TokenBaseKind_Operator, "->"),
    EXPECT(TokenBaseKind_Identifier, "greet"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
};

static TokExpect perl_fat_comma[] = {
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "%h"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "a"),
    EXPECT(TokenBaseKind_Operator, "=>"),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
};

static TokExpect perl_pkg_separator[] = {
    EXPECT(TokenBaseKind_Identifier, "My"),
    EXPECT(TokenBaseKind_Operator, "::"),
    EXPECT(TokenBaseKind_Identifier, "Greeter"),
    EXPECT(TokenBaseKind_Operator, "->"),
    EXPECT(TokenBaseKind_Identifier, "new"),
};

//~ ------------------------------------------------------------------------
//~ Numbers, ranges
//~ ------------------------------------------------------------------------

static TokExpect perl_numbers[] = {
    EXPECT(TokenBaseKind_LiteralInteger, "42"),
    EXPECT(TokenBaseKind_LiteralInteger, "0xFF_FF"),
    EXPECT(TokenBaseKind_LiteralInteger, "0b1010_1011"),
    EXPECT(TokenBaseKind_LiteralFloat, "3.14"),
    EXPECT(TokenBaseKind_LiteralFloat, "1e10"),
    EXPECT(TokenBaseKind_LiteralFloat, "2.5e-3"),
    EXPECT(TokenBaseKind_LiteralFloat, ".5"),
};

static TokExpect perl_range[] = {
    EXPECT(TokenBaseKind_Keyword, "for"),
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_Identifier, "$i"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Operator, ".."),
    EXPECT(TokenBaseKind_LiteralInteger, "5"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
};

static TokExpect perl_range_no_spaces[] = {
    //~ 1..5 — the range operator must not be swallowed as a decimal point.
    EXPECT(TokenBaseKind_LiteralInteger, "1"),
    EXPECT(TokenBaseKind_Operator, ".."),
    EXPECT(TokenBaseKind_LiteralInteger, "5"),
};

//~ ------------------------------------------------------------------------
//~ Small integration case: constructor + method call.
//~ ------------------------------------------------------------------------

static TokExpect perl_integration_constructor[] = {
    EXPECT(TokenBaseKind_Keyword, "package"),
    EXPECT(TokenBaseKind_Identifier, "My"),
    EXPECT(TokenBaseKind_Operator, "::"),
    EXPECT(TokenBaseKind_Identifier, "Greeter"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_Keyword, "sub"),
    EXPECT(TokenBaseKind_Identifier, "new"),
    EXPECT(TokenBaseKind_ScopeOpen, "{"),
    EXPECT(TokenBaseKind_Keyword, "my"),
    EXPECT(TokenBaseKind_ParentheticalOpen, "("),
    EXPECT(TokenBaseKind_Identifier, "$class"),
    EXPECT(TokenBaseKind_StatementClose, ","),
    EXPECT(TokenBaseKind_Identifier, "%args"),
    EXPECT(TokenBaseKind_ParentheticalClose, ")"),
    EXPECT(TokenBaseKind_Operator, "="),
    EXPECT(TokenBaseKind_Identifier, "@_"),
    EXPECT(TokenBaseKind_StatementClose, ";"),
    EXPECT(TokenBaseKind_ScopeClose, "}"),
};

//~ ------------------------------------------------------------------------
//~ Case table
//~ ------------------------------------------------------------------------

static Case perl_cases[] = {
    CASE("keywords_basic",                 "my $x = ; sub\n", perl_keywords_basic),
    CASE("keywords_all",                   "my our local sub use no package require if elsif else unless while until for foreach do last next redo return given when default print printf say sort map grep defined undef eval BEGIN END CHECK INIT UNITCHECK DESTROY AUTOLOAD die warn exit and or not eq ne lt gt le ge cmp\n", perl_keywords_all),
    CASE("line_comment",                   "# comment\nmy $x = 1\n", perl_line_comment),
    CASE("shebang",                        "#!/usr/bin/env perl\nuse strict;\n", perl_shebang),
    CASE("pod_block",                      "=pod\n\n=head1 NAME\n\nSample\n\n=cut\npackage main;\n", perl_pod_block),
    CASE("comment_with_specials",          "# $sigil % hash * glob & sub\n", perl_comment_with_specials),
    CASE("string_double",                  "\"hello $name world\"\n", perl_string_double),
    CASE("string_single",                  "'no $interpolation here'\n", perl_string_single),
    CASE("string_escape",                  "\"say \\\"hi\\\"\"\n", perl_string_escape),
    CASE("string_single_escape_quote",     "'it\\'s'\n", perl_string_single_escape_quote),
    CASE("sigil_scalar",                   "$foo\n", perl_sigil_scalar),
    CASE("sigil_array",                    "@arr\n", perl_sigil_array),
    CASE("sigil_hash",                     "%h\n", perl_sigil_hash),
    CASE("sigil_sub",                      "&sub\n", perl_sigil_sub),
    CASE("sigil_glob",                     "*GLOB\n", perl_sigil_glob),
    CASE("sigil_topic_underscore",         "$_\n", perl_sigil_topic_underscore),
    CASE("sigil_env",                      "%ENV\n", perl_sigil_env),
    CASE("dollar_dollar_pid",              "$$\n", perl_dollar_dollar_pid),
    CASE("dollar_dollar_deref_chain",      "$$ref\n", perl_dollar_dollar_deref),
    CASE("dollar_bang",                    "$!;\n", perl_dollar_bang),
    CASE("dollar_record_sep",              "$/\n", perl_dollar_record_sep),
    CASE("dollar_line_number",             "$.\n", perl_dollar_line_num),
    CASE("match_vars",                     "$1 + $2\n", perl_match_vars),
    CASE("match_var_two_digit",            "$10\n", perl_match_var_two_digit),
    CASE("last_index_dollar_hash",         "$#arr\n", perl_last_index),
    CASE("caret_var",                      "$^V\n", perl_caret_var),
    CASE("package_chain",                  "$My::Deep::var\n", perl_package_chain),
    CASE("array_pkg",                      "@Foo::Bar\n", perl_array_pkg),
    CASE("block_deref",                    "${ref}\n", perl_block_deref_only_sigil),
    CASE("hash_deref_arrow",               "$h->{key}\n", perl_hash_deref_arrow),
    CASE("brace_subscript",                "$h{\"key\"}\n", perl_brace_subscript),
    CASE("modulo_vs_hash",                 "my $m = 10 % 3; my %h = ();\n", perl_modulo_vs_hash),
    CASE("bitwise_vs_sub",                 "my $b = 6 & 3; &foo\n", perl_bitwise_vs_sub),
    CASE("mult_vs_glob",                   "my $p = 2 * 3; *STDOUT\n", perl_mult_vs_glob),
    CASE("power_operator",                 "my $p = 2 ** 8\n", perl_power_operator),
    CASE("deref_after_ref",                "my $r = \\$x\n", perl_deref_after_ref),
    CASE("ref_scalar",                     "\\$x\n", perl_ref_scalar),
    CASE("ref_sub",                        "\\&greet\n", perl_ref_sub),
    CASE("ref_hash",                       "\\%My::Greeter::hash\n", perl_ref_hash),
    CASE("ref_array",                      "\\@list\n", perl_ref_array),
    CASE("ref_glob",                       "\\*STDOUT\n", perl_ref_glob),
    CASE("arrow_method",                   "$obj->greet()\n", perl_arrow_method),
    CASE("fat_comma",                      "my %h = (a => 1)\n", perl_fat_comma),
    CASE("package_separator",              "My::Greeter->new\n", perl_pkg_separator),
    CASE("numbers",                        "42 0xFF_FF 0b1010_1011 3.14 1e10 2.5e-3 .5\n", perl_numbers),
    CASE("range_operator",                 "for my $i (1 .. 5)\n", perl_range),
    CASE("range_no_spaces",                "1..5\n", perl_range_no_spaces),
    CASE("integration_constructor",        "package My::Greeter; sub new { my ($class, %args) = @_; }\n", perl_integration_constructor),
    CASE0("empty_file",                    ""),
    CASE0("whitespace_only",               "  \n\t\n"),
};

static Suite perl_suite = {
    "perl",
    (LexInitFn)F4_Perl_LexInit,
    (LexFullInputFn)F4_Perl_LexFullInput,
    sizeof(F4_Perl_LexerState),
    perl_cases, ArrayCount(perl_cases),
};

#endif // FCODER_FLEURY_LEXER_TEST_CASES_PERL_H
