/* date = October 12th 2026

   4coder_fleury_lang_script_shared.h - Shared call-site scanner for the
   script-language PosContext hooks (Python / Lua / Perl).

   Fleury's C++ PosContext (4coder_fleury_lang_cpp.cpp) walks left from the
   cursor looking for a '(' whose preceding token is the callee identifier,
   counting commas at paren-depth 0 to derive the argument index; its
   member-access ('.'/'->') half is disabled with #if 0. We reproduce only the
   call-site half, but written against base Token kinds (not C++ sub_kinds) so
   one implementation serves every language whose lexer emits:
     - TokenBaseKind_ParentheticalOpen / Close for ( )
     - TokenBaseKind_StatementClose for the comma (comma-ness is confirmed by
       the token text, because ':' and ';' are also StatementClose)
     - TokenBaseKind_Identifier for the callee name

   The scanner is pure (arena + source text + token array + cursor pos) so it
   can be unit-tested without a live Buffer. The language hook turns each hit
   into an F4_Language_PosContextData via F4_Language_PosContext_PushData_Call,
   which resolves the callee against the F4 code index. */

#ifndef FCODER_FLEURY_LANG_SCRIPT_SHARED_H
#define FCODER_FLEURY_LANG_SCRIPT_SHARED_H

//~ A single resolved call site: the callee name text and which argument the
//~ cursor currently sits in (0-based). Newest outermost call first.
struct F4_ScriptCallHit
{
    F4_ScriptCallHit *next;
    String_Const_u8 name;
    int argument_index;
};

internal b32
f4_script_token_is_comma(String_Const_u8 text, Token *token)
{
    String_Const_u8 t = string_substring(text, Ii64(token));
    return (t.size == 1 && t.str[0] == ',');
}

//~ Scan left from `pos` collecting up to 4 nested call sites. Mirrors the
//~ control flow of F4_CPP_PosContext (same double-decrement behaviour), only
//~ using base kinds + text-based comma detection.
internal F4_ScriptCallHit *
f4_script_scan_calls(Arena *arena, String_Const_u8 text, Token_Array *tokens, i64 pos)
{
    F4_ScriptCallHit *first = 0;
    F4_ScriptCallHit *last = 0;
    if(tokens->count == 0)
    {
        return 0;
    }

    Token_Iterator_Array it = token_iterator_pos(0, tokens, pos);
    int count = 0;
    int paren_nest = 0;
    int arg_idx = 0;

    for(int i = 0; count < 4; i += 1)
    {
        Token *token = token_it_read(&it);
        if(!token)
        {
            break;
        }

        if(paren_nest == 0 &&
           token->kind == TokenBaseKind_ParentheticalOpen &&
           token_it_dec_non_whitespace(&it))
        {
            Token *name = token_it_read(&it);
            if(name && name->kind == TokenBaseKind_Identifier)
            {
                F4_ScriptCallHit *hit = push_array_zero(arena, F4_ScriptCallHit, 1);
                hit->name = string_substring(text, Ii64(name));
                hit->argument_index = arg_idx;
                if(last)
                {
                    last->next = hit;
                }
                else
                {
                    first = hit;
                }
                last = hit;
                count += 1;
                arg_idx = 0;
            }
        }
        else if(token->kind == TokenBaseKind_ParentheticalOpen)
        {
            paren_nest -= 1;
        }
        else if(token->kind == TokenBaseKind_ParentheticalClose && i > 0)
        {
            paren_nest += 1;
        }
        else if(token->kind == TokenBaseKind_StatementClose && i > 0 &&
                paren_nest == 0 && f4_script_token_is_comma(text, token))
        {
            arg_idx += 1;
        }

        if(!token_it_dec_non_whitespace(&it))
        {
            break;
        }
    }

    return first;
}

#endif // FCODER_FLEURY_LANG_SCRIPT_SHARED_H
