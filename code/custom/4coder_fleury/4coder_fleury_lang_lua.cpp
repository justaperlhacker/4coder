/* date = October 12th 2026 */

#ifndef FCODER_FLEURY_LANG_LUA_CPP
#define FCODER_FLEURY_LANG_LUA_CPP

//~ NOTE: Hand-written Lua lexer for the Fleury layer.
// Covers Lua 5.1-5.4 lexical grammar; syntax highlighting only for now.

struct F4_Lua_LexerState
{
    String_Const_u8 string;
    u8 *at;
    u8 *one_past_last;
};

static const char *f4_lua_keywords[] =
{
    "and", "break", "do", "else", "elseif", "end", "false", "for",
    "function", "goto", "if", "in", "local", "nil", "not", "or",
    "repeat", "return", "then", "true", "until", "while",
    0
};

internal b32
F4_Lua_MatchLit(String_Const_u8 s, const char *lit)
{
    u64 n = 0;
    while(lit[n] != 0)
    {
        n += 1;
    }
    if(s.size != n)
    {
        return 0;
    }
    for(u64 i = 0; i < n; i += 1)
    {
        if(s.str[i] != (u8)lit[i])
        {
            return 0;
        }
    }
    return 1;
}

internal b32
F4_Lua_IsKeyword(String_Const_u8 s)
{
    b32 result = 0;
    for(int i = 0; f4_lua_keywords[i] != 0; i += 1)
    {
        if(F4_Lua_MatchLit(s, f4_lua_keywords[i]))
        {
            result = 1;
            break;
        }
    }
    return result;
}

internal b32
F4_Lua_IdentStart(u8 c)
{
    return (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c >= 0x80);
}

internal b32
F4_Lua_IdentCont(u8 c)
{
    return (F4_Lua_IdentStart(c) || (c >= '0' && c <= '9'));
}

internal b32
F4_Lua_IsHexDigit(u8 c)
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

internal b32
F4_Lua_CharIsSymbol(u8 c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '^' || c == '#' || c == '~' || c == '<' || c == '>' ||
            c == '=' || '&' == c || '|' == c || ':' == c);
}

// NOTE: Returns the length of the opening long-bracket sequence at s[i..], or
// 0 if it isn't a long bracket. Long brackets look like [[ or [=* [.
internal i64
F4_Lua_LongBracketOpen(u8 *s, i64 i, i64 strmax)
{
    if(i >= strmax || s[i] != '[')
    {
        return 0;
    }
    i64 j = i+1;
    i64 eqs = 0;
    while(j < strmax && s[j] == '=')
    {
        eqs += 1;
        j += 1;
    }
    if(j < strmax && s[j] == '[')
    {
        return 2 + eqs;
    }
    return 0;
}

//~ NOTE: Index Lua function definitions so the code index colors them and
//~ drives go-to-definition. Covers `function NAME`, `local function NAME`, and
//~ the method forms `function M.NAME` / `function M:NAME` (leaf name wins).
internal F4_LANGUAGE_INDEXFILE(F4_Lua_IndexFile)
{
    for(;!ctx->done;)
    {
        Token *name = 0;
        F4_Index_TokenSkipFlags flags = F4_Index_TokenSkipFlag_SkipWhitespace;

        if(F4_Index_ParsePattern(ctx, "%t%k", "function", TokenBaseKind_Identifier, &name))
        {
            for(;;)
            {
                b32 dot = F4_Index_PeekToken(ctx, S8Lit("."));
                b32 colon = F4_Index_PeekToken(ctx, S8Lit(":"));
                if(!(dot || colon))
                {
                    break;
                }
                Token *sep = 0;
                F4_Index_RequireTokenKind(ctx, TokenBaseKind_Operator, &sep, flags);
                Token *tail = 0;
                if(F4_Index_RequireTokenKind(ctx, TokenBaseKind_Identifier, &tail, flags))
                {
                    name = tail;
                }
                else
                {
                    break;
                }
            }
            F4_Index_MakeNote(ctx, Ii64(name), F4_Index_NoteKind_Function, 0);
        }
        else if(F4_Index_RequireTokenKind(ctx, TokenBaseKind_Comment, &name, flags))
        {
            F4_Index_ParseComment(ctx, name);
        }
        else
        {
            F4_Index_ParseCtx_Inc(ctx, flags);
        }
    }
}

internal F4_LANGUAGE_LEXINIT(F4_Lua_LexInit)
{
    F4_Lua_LexerState *state = (F4_Lua_LexerState *)state_ptr;
    state->string = contents;
    state->at = contents.str;
    state->one_past_last = contents.str + contents.size;
}

internal F4_LANGUAGE_LEXFULLINPUT(F4_Lua_LexFullInput)
{
    b32 result = false;
    F4_Lua_LexerState state_ = *(F4_Lua_LexerState *)state_ptr;
    F4_Lua_LexerState *state = &state_;
    u64 emit_counter = 0;
    u8 *s = state->string.str;
    i64 strmax = (i64)state->string.size;

    for(i64 i = (i64)(state->at - s); i < strmax;)
    {
        i64 start_i = i;
        u8 chr = s[i];

        // NOTE: Shebang (only legal on line 1; Lua explicitly ignores it).
        if(i == 0 && strmax >= 2 && s[0] == '#' && s[1] == '!')
        {
            i64 j = 0;
            for(; j < strmax && s[j] != '\n'; j += 1);
            Token token = { 0, j, TokenBaseKind_Comment, 0 };
            token_list_push(arena, list, &token);
            i = j;
        }
        
        // NOTE: Comments (line or long-bracket).
        else if(chr == '-' && i+1 < strmax && s[i+1] == '-')
        {
            i64 lb = F4_Lua_LongBracketOpen(s, i+2, strmax);
            if(lb > 0)
            {
                // NOTE: find matching close bracket with same number of '='.
                i64 eqs = lb - 2;
                i64 j = i+2+lb;
                i64 end = strmax;
                for(; j+1 < strmax; j += 1)
                {
                    if(s[j] == ']' && j+eqs+1 < strmax)
                    {
                        i64 k = j+1;
                        i64 cnt = 0;
                        while(k < strmax && s[k] == '=' && cnt < eqs)
                        {
                            cnt += 1;
                            k += 1;
                        }
                        if(cnt == eqs && k < strmax && s[k] == ']')
                        {
                            end = k+1;
                            break;
                        }
                    }
                }
                Token token = { i, end - i, TokenBaseKind_Comment, 0 };
                token_list_push(arena, list, &token);
                i = end;
            }
            else
            {
                Token token = { i, 2, TokenBaseKind_Comment, 0 };
                for(i64 j = i+2; j < strmax && s[j] != '\n'; j += 1, token.size += 1);
                token_list_push(arena, list, &token);
                i += token.size;
            }
        }

        // NOTE: Whitespace.
        else if(character_is_whitespace(chr))
        {
            Token token = { i, 1, TokenBaseKind_Whitespace, 0 };
            for(i64 j = i+1; j < strmax && character_is_whitespace(s[j]);
                j += 1, token.size += 1);
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: Long-bracket string.
        else if(chr == '[')
        {
            i64 lb = F4_Lua_LongBracketOpen(s, i, strmax);
            if(lb > 0)
            {
                i64 eqs = lb - 2;
                i64 j = i+lb;
                i64 end = strmax;
                for(; j+1 < strmax; j += 1)
                {
                    if(s[j] == ']' && j+eqs+1 < strmax)
                    {
                        i64 k = j+1;
                        i64 cnt = 0;
                        while(k < strmax && s[k] == '=' && cnt < eqs)
                        {
                            cnt += 1;
                            k += 1;
                        }
                        if(cnt == eqs && k < strmax && s[k] == ']')
                        {
                            end = k+1;
                            break;
                        }
                    }
                }
                Token token = { i, end - i, TokenBaseKind_LiteralString, 0 };
                token_list_push(arena, list, &token);
                i = end;
            }
            else
            {
                Token token = { i, 1, TokenBaseKind_ParentheticalOpen, 0 };
                token_list_push(arena, list, &token);
                i += 1;
            }
        }

        // NOTE: Short string literal.
        else if(chr == '"' || chr == '\'')
        {
            i64 j = i+1;
            for(; j < strmax && s[j] != chr && s[j] != '\n'; j += 1)
            {
                if(s[j] == '\\')
                {
                    j += 1;
                }
            }
            i64 size = (j < strmax && s[j] == chr) ? (j+1 - i) : (j - i);
            Token token = { i, size, TokenBaseKind_LiteralString, 0 };
            token_list_push(arena, list, &token);
            i += size;
        }

        // NOTE: Identifier / keyword.
        else if(F4_Lua_IdentStart(chr))
        {
            Token token = { i, 1, TokenBaseKind_Identifier, 0 };
            for(i64 j = i+1; j < strmax && F4_Lua_IdentCont(s[j]);
                j += 1, token.size += 1);
            String_Const_u8 name = { s + token.pos, (u64)token.size };
            if(F4_Lua_IsKeyword(name))
            {
                token.kind = TokenBaseKind_Keyword;
            }
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: Numeric literal (decimal, hex, optional float / exponent).
        else if((chr >= '0' && chr <= '9') ||
                (chr == '.' && i+1 < strmax && s[i+1] >= '0' && s[i+1] <= '9'))
        {
            b32 is_float = (chr == '.');
            i64 j = i;
            if(chr == '0' && i+1 < strmax && (s[i+1] == 'x' || s[i+1] == 'X'))
            {
                j = i+2;
                for(; j < strmax && F4_Lua_IsHexDigit(s[j]); j += 1);
                if(j < strmax && s[j] == '.')
                {
                    is_float = 1;
                    j += 1;
                    for(; j < strmax && F4_Lua_IsHexDigit(s[j]); j += 1);
                }
                if(j < strmax && (s[j] == 'p' || s[j] == 'P'))
                {
                    i64 k = j+1;
                    if(k < strmax && (s[k] == '+' || s[k] == '-'))
                    {
                        k += 1;
                    }
                    if(k < strmax && s[k] >= '0' && s[k] <= '9')
                    {
                        is_float = 1;
                        j = k;
                        for(; j < strmax && (s[j] >= '0' && s[j] <= '9'); j += 1);
                    }
                }
            }
            else
            {
                j += 1;
                for(; j < strmax && (s[j] >= '0' && s[j] <= '9'); j += 1);
                if(j < strmax && s[j] == '.')
                {
                    is_float = 1;
                    j += 1;
                    for(; j < strmax && (s[j] >= '0' && s[j] <= '9'); j += 1);
                }
                if(j < strmax && (s[j] == 'e' || s[j] == 'E'))
                {
                    i64 k = j+1;
                    if(k < strmax && (s[k] == '+' || s[k] == '-'))
                    {
                        k += 1;
                    }
                    if(k < strmax && s[k] >= '0' && s[k] <= '9')
                    {
                        is_float = 1;
                        j = k;
                        for(; j < strmax && (s[j] >= '0' && s[j] <= '9'); j += 1);
                    }
                }
            }
            Token token = { i, j - i,
                (is_float ? TokenBaseKind_LiteralFloat : TokenBaseKind_LiteralInteger), 0 };
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: Brackets / scope.
        else if(chr == '{' || chr == '}')
        {
            Token token = { i, 1,
                (chr == '{') ? TokenBaseKind_ScopeOpen : TokenBaseKind_ScopeClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }
        else if(chr == '(')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalOpen, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }
        else if(chr == ')')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }
        else if(chr == ']')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }

        // NOTE: Statement separators.
        else if(chr == ';' || chr == ',')
        {
            Token token = { i, 1, TokenBaseKind_StatementClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }

        // NOTE: Lua's '.' / '..' / '...' operator run.
        else if(chr == '.')
        {
            i64 size = 1;
            if(i+1 < strmax && s[i+1] == '.')
            {
                size = 2;
                if(i+2 < strmax && s[i+2] == '.')
                {
                    size = 3;
                }
            }
            Token token = { i, size, TokenBaseKind_Operator, 0 };
            token_list_push(arena, list, &token);
            i += size;
        }

        // NOTE: Operator runs.
        else if(F4_Lua_CharIsSymbol(chr))
        {
            Token token = { i, 1, TokenBaseKind_Operator, 0 };
            for(i64 j = i+1; j < strmax && F4_Lua_CharIsSymbol(s[j]);
                j += 1, token.size += 1);
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: Catch-all.
        else
        {
            Token token = { i, 1, TokenBaseKind_LexError, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }

        if(i == start_i)
        {
            i += 1;
        }
        state->at = s + i;
        if(i >= strmax)
        {
            goto eof;
        }
        emit_counter += 1;
        if(emit_counter >= max)
        {
            goto end;
        }
    }

    eof:;
    {
        result = true;
        Token token = { strmax, 1, TokenBaseKind_EOF, 0 };
        token_list_push(arena, list, &token);
    }

    end:;
    *(F4_Lua_LexerState *)state_ptr = *state;
    return result;
}

internal F4_LANGUAGE_POSCONTEXT(F4_Lua_PosContext)
{
    return 0;
}

internal F4_LANGUAGE_HIGHLIGHT(F4_Lua_Highlight)
{
    // NOTE: base kinds cover everything for now.
}

#endif // FCODER_FLEURY_LANG_LUA_CPP
