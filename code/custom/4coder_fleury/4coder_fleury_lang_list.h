/* date = January 29th 2021 9:37 pm */

#ifndef FCODER_FLEURY_LANG_LIST_H
#define FCODER_FLEURY_LANG_LIST_H

// NOTE(rjf): Include language files here.
#include "generated/4coder_fleury_lexer_jai.h"
#include "generated/4coder_fleury_lexer_jai.cpp"
#include "4coder_fleury_lang_cpp.cpp"
#include "4coder_fleury_lang_jai.cpp"
#include "4coder_fleury_lang_metadesk.cpp"
#include "4coder_fleury_lang_python.cpp"
#include "4coder_fleury_lang_lua.cpp"
#include "4coder_fleury_lang_perl.cpp"

// NOTE(rjf): @f4_register_languages Register languages.
function void
F4_RegisterLanguages(void)
{
    // NOTE(rjf): C/C++
    {
        String_Const_u8 extensions[] =
        {
            S8Lit("cpp"), S8Lit("cc"), S8Lit("c"), S8Lit("cxx"),
            S8Lit("C"), S8Lit("h"), S8Lit("hpp"),
        };
        for(int i = 0; i < ArrayCount(extensions); i += 1)
        {
            F4_RegisterLanguage(extensions[i],
                                F4_CPP_IndexFile,
                                lex_full_input_cpp_init,
                                lex_full_input_cpp_breaks,
                                F4_CPP_PosContext,
                                F4_CPP_Highlight,
                                Lex_State_Cpp);
        }
    }
    
    // NOTE(rjf): Jai
    {
        F4_RegisterLanguage(S8Lit("jai"),
                            F4_Jai_IndexFile,
                            lex_full_input_jai_init,
                            lex_full_input_jai_breaks,
                            F4_Jai_PosContext,
                            F4_Jai_Highlight,
                            Lex_State_Jai);
    }
    
    // NOTE(rjf): Metadesk
    {
        String_Const_u8 extensions[] =
        {
            // TODO(rjf): Maybe find a config-driven way to specify these? "mc" was sort of
            // introduced ad-hoc...
            S8Lit("md"), S8Lit("mc"), S8Lit("metacode"), S8Lit("meta"), S8Lit("metadesk"),
        };
        for(int i = 0; i < ArrayCount(extensions); i += 1)
        {
            F4_RegisterLanguage(extensions[i],
                                F4_MD_IndexFile,
                                lex_full_input_cpp_init,
                                lex_full_input_cpp_breaks,
                                F4_MD_PosContext,
                                F4_MD_Highlight,
                                Lex_State_Cpp);
        }
    }
    
    // NOTE: Python
    {
        String_Const_u8 extensions[] =
        {
            S8Lit("py"), S8Lit("pyi"), S8Lit("pyw"),
        };
        for(int i = 0; i < ArrayCount(extensions); i += 1)
        {
            F4_RegisterLanguage(extensions[i],
                                F4_Py_IndexFile,
                                F4_Py_LexInit,
                                F4_Py_LexFullInput,
                                F4_Py_PosContext,
                                F4_Py_Highlight,
                                F4_Py_LexerState);
        }
    }
    
    // NOTE: Lua
    {
        F4_RegisterLanguage(S8Lit("lua"),
                            F4_Lua_IndexFile,
                            F4_Lua_LexInit,
                            F4_Lua_LexFullInput,
                            F4_Lua_PosContext,
                            F4_Lua_Highlight,
                            F4_Lua_LexerState);
    }
    
    // NOTE: Perl
    {
        String_Const_u8 extensions[] =
        {
            S8Lit("pl"), S8Lit("pm"), S8Lit("t"), S8Lit("pod"),
        };
        for(int i = 0; i < ArrayCount(extensions); i += 1)
        {
            F4_RegisterLanguage(extensions[i],
                                F4_Perl_IndexFile,
                                F4_Perl_LexInit,
                                F4_Perl_LexFullInput,
                                F4_Perl_PosContext,
                                F4_Perl_Highlight,
                                F4_Perl_LexerState);
        }
    }
}

#endif //FCODER_FLEURY_LANG_LIST_H
