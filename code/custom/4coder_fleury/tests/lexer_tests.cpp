/* date = October 12th 2026

   lexer_tests.cpp - Entry point that stitches together the reusable test
   framework and the per-language cases. Everything lives in one translation
   unit so the language .cpp files can use their internal static helpers just
   like they do inside the editor's single-TU build.

   To add a new language under test:

     1. Add the hand-written language file under 4coder_fleury/ (see the
        Python / Lua / Perl lexers for the pattern).
     2. Include it in this file right next to the others so its lexer state
        and hooks are visible.
     3. Create a matching "cases_<lang>.h" in tests/ that defines a
        static Case array and a static Suite, then #include it below and
        add a run_suite() call in main().

   Build:  ./run.sh            (or: g++ -std=c++11 -w lexer_tests.cpp -o ...)
   Run:    ./lexer_tests       (or: ./run.sh -v for full token dumps) */

#include "lexer_test_stubs.h"
#include "lexer_test_index.h"

//~ The lexers themselves — same sources the editor compiles into
//~ custom_4coder.so via 4coder_fleury.cpp.
#include "../4coder_fleury_lang_python.cpp"
#include "../4coder_fleury_lang_lua.cpp"
#include "../4coder_fleury_lang_perl.cpp"

#include "lexer_test_framework.h"

#include "cases_python.h"
#include "cases_lua.h"
#include "cases_perl.h"

int
main(int argc, char **argv)
{
    for(int i = 1; i < argc; i += 1)
    {
        if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
        {
            g_lex_verbose = 1;
        }
        else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("usage: lexer_tests [-v|--verbose] [-h|--help]\n");
            printf("  -v   print PASS lines and full token dump for failures\n");
            return 0;
        }
    }

    Suite *suites[] = {
        &py_suite,
        &lua_suite,
        &perl_suite,
    };
    for(int i = 0; i < ArrayCount(suites); i += 1)
    {
        run_suite(suites[i]);
    }

    printf("\n----- %d pass, %d fail -----\n", g_lex_pass, g_lex_fail);
    return g_lex_fail == 0 ? 0 : 1;
}
