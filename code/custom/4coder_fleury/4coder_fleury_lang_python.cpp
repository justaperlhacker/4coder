/* date = October 12th 2026 */

#ifndef FCODER_FLEURY_LANG_PYTHON_CPP
#define FCODER_FLEURY_LANG_PYTHON_CPP

//~ NOTE: Hand-written Python lexer for the Fleury layer.
// Syntax highlighting only for now; the IndexFile hook is a stub and
// can grow into real python code indexing later.

struct F4_Py_LexerState
{
    String_Const_u8 string;
    u8 *at;
    u8 *one_past_last;
};

static const char *f4_py_keywords[] =
{
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is",
    "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
    "while", "with", "yield",
    0
};

internal b32
F4_Py_MatchLit(String_Const_u8 s, const char *lit)
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
F4_Py_IsKeyword(String_Const_u8 s)
{
    b32 result = 0;
    for(int i = 0; f4_py_keywords[i] != 0; i += 1)
    {
        if(F4_Py_MatchLit(s, f4_py_keywords[i]))
        {
            result = 1;
            break;
        }
    }
    return result;
}

internal b32
F4_Py_IdentStart(u8 c)
{
    return (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c >= 0x80);
}

internal b32
F4_Py_IdentCont(u8 c)
{
    return (F4_Py_IdentStart(c) || (c >= '0' && c <= '9'));
}

internal b32
F4_Py_IsStrPrefixChar(u8 c)
{
    return (c == 'r' || c == 'R' || c == 'b' || c == 'B' ||
            c == 'u' || c == 'U' || c == 'f' || c == 'F');
}

internal b32
F4_Py_CharIsSymbol(u8 c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '~' || c == '<' || c == '>' || c == '=' || c == '!' ||
            c == '&' || c == '|' || c == '^' || c == '@');
}

internal F4_LANGUAGE_INDEXFILE(F4_Py_IndexFile)
{
    // NOTE: syntax-only for now; code index hooks land with the "intelligence" pass.
}

internal F4_LANGUAGE_LEXINIT(F4_Py_LexInit)
{
    F4_Py_LexerState *state = (F4_Py_LexerState *)state_ptr;
    state->string = contents;
    state->at = contents.str;
    state->one_past_last = contents.str + contents.size;
}

internal F4_LANGUAGE_LEXFULLINPUT(F4_Py_LexFullInput)
{
    b32 result = false;
    F4_Py_LexerState state_ = *(F4_Py_LexerState *)state_ptr;
    F4_Py_LexerState *state = &state_;
    u64 emit_counter = 0;
    u8 *s = state->string.str;
    i64 strmax = (i64)state->string.size;

    for(i64 i = (i64)(state->at - s); i < strmax;)
    {
        i64 start_i = i;
        u8 chr = s[i];

        // NOTE: Line comment.
        if(chr == '#')
        {
            Token token = { i, 1, TokenBaseKind_Comment, 0 };
            for(i64 j = i+1; j < strmax && s[j] != '\n'; j += 1, token.size += 1);
            token_list_push(arena, list, &token);
            i += token.size;
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

        // NOTE: Line-continuation backslash treated as whitespace.
        else if(chr == '\\' && i+1 < strmax && (s[i+1] == '\n' || s[i+1] == '\r'))
        {
            Token token = { i, 2, TokenBaseKind_Whitespace, 0 };
            if(s[i+1] == '\r' && i+2 < strmax && s[i+2] == '\n')
            {
                token.size += 1;
            }
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: String literal, possibly with an r/b/u/f prefix.
        else if(chr == '"' || chr == '\'' ||
                (F4_Py_IsStrPrefixChar(chr) &&
                 ((i+1 < strmax && (s[i+1] == '"' || s[i+1] == '\'')) ||
                  (i+2 < strmax && F4_Py_IsStrPrefixChar(s[i+1]) &&
                   s[i+1] != chr && (s[i+2] == '"' || s[i+2] == '\'')))))
        {
            i64 q = i;
            while(q < strmax && s[q] != '"' && s[q] != '\'')
            {
                q += 1;
            }
            if(q >= strmax)
            {
                // NOTE: shouldn't happen given our guard, but be defensive.
                Token token = { i, 1, TokenBaseKind_LexError, 0 };
                token_list_push(arena, list, &token);
                i += 1;
            }
            else
            {
                u8 quote = s[q];
                b32 triple = (q+2 < strmax && s[q+1] == quote && s[q+2] == quote);
                i64 end;
                if(triple)
                {
                    i64 j = q+3;
                    for(; j+2 < strmax; j += 1)
                    {
                        if(s[j] == '\\')
                        {
                            j += 1;
                            continue;
                        }
                        if(s[j] == quote && s[j+1] == quote && s[j+2] == quote)
                        {
                            break;
                        }
                    }
                    if(j+2 < strmax)
                    {
                        end = j+3;
                    }
                    else
                    {
                        end = strmax;
                    }
                }
                else
                {
                    i64 j = q+1;
                    for(; j < strmax && s[j] != quote && s[j] != '\n'; j += 1)
                    {
                        if(s[j] == '\\')
                        {
                            j += 1;
                        }
                    }
                    if(j < strmax && s[j] == quote)
                    {
                        end = j+1;
                    }
                    else
                    {
                        end = j;
                    }
                }
                Token token = { i, end - i, TokenBaseKind_LiteralString, 0 };
                token_list_push(arena, list, &token);
                i = end;
            }
        }

        // NOTE: Identifier / keyword.
        else if(F4_Py_IdentStart(chr))
        {
            Token token = { i, 1, TokenBaseKind_Identifier, 0 };
            for(i64 j = i+1; j < strmax && F4_Py_IdentCont(s[j]);
                j += 1, token.size += 1);
            String_Const_u8 name = { s + token.pos, (u64)token.size };
            if(F4_Py_IsKeyword(name))
            {
                token.kind = TokenBaseKind_Keyword;
            }
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: Numeric literal.
        else if((chr >= '0' && chr <= '9') ||
                (chr == '.' && i+1 < strmax && s[i+1] >= '0' && s[i+1] <= '9'))
        {
            b32 is_float = (chr == '.');
            Token token = { i, 1, TokenBaseKind_LiteralInteger, 0 };
            i64 j = i+1;
            if(chr == '0' && j < strmax && (s[j] == 'x' || s[j] == 'X' ||
                                            s[j] == 'o' || s[j] == 'O' ||
                                            s[j] == 'b' || s[j] == 'B'))
            {
                j += 1;
                for(; j < strmax && (((s[j] >= '0' && s[j] <= '9') ||
                                      (s[j] >= 'a' && s[j] <= 'f') ||
                                      (s[j] >= 'A' && s[j] <= 'F') ||
                                      s[j] == '_'));
                    j += 1);
            }
            else
            {
                for(; j < strmax && ((s[j] >= '0' && s[j] <= '9') || s[j] == '_');
                    j += 1);
                if(j < strmax && s[j] == '.')
                {
                    is_float = 1;
                    j += 1;
                    for(; j < strmax && ((s[j] >= '0' && s[j] <= '9') || s[j] == '_');
                        j += 1);
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
                        for(; j < strmax && ((s[j] >= '0' && s[j] <= '9') || s[j] == '_');
                            j += 1);
                    }
                }
                if(j < strmax && (s[j] == 'j' || s[j] == 'J'))
                {
                    j += 1;
                }
            }
            token.size = j - i;
            token.kind = is_float ? TokenBaseKind_LiteralFloat : TokenBaseKind_LiteralInteger;
            token_list_push(arena, list, &token);
            i += token.size;
        }

        // NOTE: Scope/paren brackets.
        else if(chr == '{' || chr == '}')
        {
            Token token = { i, 1,
                (chr == '{') ? TokenBaseKind_ScopeOpen : TokenBaseKind_ScopeClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }
        else if(chr == '(' || chr == '[')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalOpen, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }
        else if(chr == ')' || chr == ']')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }

        // NOTE: Walrus operator (:=) is a distinct assignment operator, not
        // a colon followed by '='.
        else if(chr == ':' && i+1 < strmax && s[i+1] == '=')
        {
            Token token = { i, 2, TokenBaseKind_Operator, 0 };
            token_list_push(arena, list, &token);
            i += 2;
        }

        // NOTE: Statement separator-ish punctuation.
        else if(chr == ',' || chr == ';' || chr == ':')
        {
            Token token = { i, 1, TokenBaseKind_StatementClose, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }

        // NOTE: Dot (attribute access).
        else if(chr == '.')
        {
            Token token = { i, 1, TokenBaseKind_Operator, 0 };
            token_list_push(arena, list, &token);
            i += 1;
        }

        // NOTE: Operator runs.
        else if(F4_Py_CharIsSymbol(chr))
        {
            Token token = { i, 1, TokenBaseKind_Operator, 0 };
            for(i64 j = i+1; j < strmax && F4_Py_CharIsSymbol(s[j]);
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
    *(F4_Py_LexerState *)state_ptr = *state;
    return result;
}

internal F4_LANGUAGE_POSCONTEXT(F4_Py_PosContext)
{
    return 0;
}

internal F4_LANGUAGE_HIGHLIGHT(F4_Py_Highlight)
{
    // NOTE: base kinds cover everything for now.
}

#endif // FCODER_FLEURY_LANG_PYTHON_CPP
