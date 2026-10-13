/* date = October 12th 2026 */

#ifndef FCODER_FLEURY_LANG_PERL_CPP
#define FCODER_FLEURY_LANG_PERL_CPP

//~ NOTE: Hand-written Perl lexer for the Fleury layer.
// Regex/heredoc handling is intentionally deferred; the goal here is
// readable syntax colouring for the common cases.

struct F4_Perl_LexerState
{
    String_Const_u8 string;
    u8 *at;
    u8 *one_past_last;
    // NOTE: Tracks the last emitted non-whitespace token kind. Used to
    // disambiguate % * & between sigil-prefix and binary operator.
    Token_Base_Kind last_kind;
};

static const char *f4_perl_keywords[] =
{
    "my", "our", "local", "sub", "use", "no", "package", "require",
    "if", "elsif", "else", "unless", "while", "until", "for", "foreach",
    "do", "last", "next", "redo", "return", "given", "when", "default",
    "print", "printf", "say", "sort", "map", "grep", "defined", "undef",
    "eval", "BEGIN", "END", "CHECK", "INIT", "UNITCHECK", "DESTROY",
    "AUTOLOAD", "die", "warn", "exit",
    "and", "or", "not", "eq", "ne", "lt", "gt", "le", "ge", "cmp",
    0
};

internal b32
F4_Perl_MatchLit(String_Const_u8 s, const char *lit)
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
F4_Perl_IsKeyword(String_Const_u8 s)
{
    b32 result = 0;
    for(int i = 0; f4_perl_keywords[i] != 0; i += 1)
    {
        if(F4_Perl_MatchLit(s, f4_perl_keywords[i]))
        {
            result = 1;
            break;
        }
    }
    return result;
}

internal b32
F4_Perl_IdentStart(u8 c)
{
    return (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c >= 0x80);
}

internal b32
F4_Perl_IdentCont(u8 c)
{
    return (F4_Perl_IdentStart(c) || (c >= '0' && c <= '9'));
}

internal b32
F4_Perl_IsHexDigit(u8 c)
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

//~ NOTE: In Perl $ @ % & * all lead variable/typeglob names ("sigils").
// & is the legacy sub-call form (passes @_ by default) and * is the typeglob;
// both are only sigils when they actually prefix a name or a {${...}} block,
// so the sigil branch below falls back to operator otherwise.
internal b32
F4_Perl_CharIsSigil(u8 c)
{
    return (c == '$' || c == '@' || c == '%' || c == '&' || c == '*');
}

internal b32
F4_Perl_IsSpecialVarChar(u8 c)
{
    switch(c)
    {
        case '!': case '"': case '#': case '$': case '%': case '&':
        case '\'': case '(': case ')': case '*': case '+': case ',':
        case '-': case '.': case '/': case ':': case ';': case '<':
        case '=': case '>': case '?': case '@': case '\\': case '^':
        case '`': case '|': case '~':
        {
            return 1;
        }
    }
    return 0;
}

internal b32
F4_Perl_CharIsSymbol(u8 c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '<' || c == '>' || c == '=' || c == '!' || c == '~' ||
            c == '^' || c == '|' || c == '&' || c == '?' || c == '.' ||
            c == ':');
}

// NOTE: Returns 1 when the previous emitted token ends a value/expression.
// In that position, a bare '%' '*' '&' reads as a binary operator rather
// than a sigil. Everything else (line start, after ( , ; = { [ keyword
// etc.) allows the sigil reading.
internal b32
F4_Perl_PrevEndsTerm(Token_Base_Kind k)
{
    switch(k)
    {
        case TokenBaseKind_Identifier:
        case TokenBaseKind_LiteralInteger:
        case TokenBaseKind_LiteralFloat:
        case TokenBaseKind_LiteralString:
        case TokenBaseKind_ParentheticalClose:
        case TokenBaseKind_ScopeClose:
        {
            return 1;
        }
    }
    return 0;
}

//~ NOTE: Index Perl definitions. `sub NAME` -> Function, `package NAME` -> Type
//~ (namespaces color like types), `use constant NAME => ...` -> Constant.
//~ Package chains (`My::Deep`) resolve to the leaf segment.
internal F4_LANGUAGE_INDEXFILE(F4_Perl_IndexFile)
{
    for(;!ctx->done;)
    {
        Token *name = 0;
        F4_Index_TokenSkipFlags flags = F4_Index_TokenSkipFlag_SkipWhitespace;

        if(F4_Index_ParsePattern(ctx, "%t%k", "sub", TokenBaseKind_Identifier, &name))
        {
            F4_Index_MakeNote(ctx, Ii64(name), F4_Index_NoteKind_Function, 0);
        }
        else if(F4_Index_ParsePattern(ctx, "%t%k", "package", TokenBaseKind_Identifier, &name))
        {
            while(F4_Index_PeekToken(ctx, S8Lit("::")))
            {
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
            F4_Index_MakeNote(ctx, Ii64(name), F4_Index_NoteKind_Type, F4_Index_NoteFlag_ProductType);
        }
        else if(F4_Index_ParsePattern(ctx, "%t%t%k", "use", "constant", TokenBaseKind_Identifier, &name))
        {
            F4_Index_MakeNote(ctx, Ii64(name), F4_Index_NoteKind_Constant, 0);
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

internal F4_LANGUAGE_LEXINIT(F4_Perl_LexInit)
{
    F4_Perl_LexerState *state = (F4_Perl_LexerState *)state_ptr;
    state->string = contents;
    state->at = contents.str;
    state->one_past_last = contents.str + contents.size;
    state->last_kind = TokenBaseKind_EOF;
}

internal F4_LANGUAGE_LEXFULLINPUT(F4_Perl_LexFullInput)
{
    b32 result = false;
    F4_Perl_LexerState state_ = *(F4_Perl_LexerState *)state_ptr;
    F4_Perl_LexerState *state = &state_;
    u64 emit_counter = 0;
    u8 *s = state->string.str;
    i64 strmax = (i64)state->string.size;

    for(i64 i = (i64)(state->at - s); i < strmax;)
    {
        i64 start_i = i;
        u8 chr = s[i];

        // NOTE: POD block (must start at column 0 with '=' + alpha).
        if(chr == '=' && (i == 0 || s[i-1] == '\n') &&
           i+1 < strmax && F4_Perl_IdentStart(s[i+1]))
        {
            i64 j = i;
            for(; j < strmax;)
            {
                // NOTE: end-of-block check on each line boundary.
                if(j > i && s[j-1] == '\n' && s[j] == '=' && j+1 < strmax &&
                   (s[j+1] == 'c' || s[j+1] == 'C'))
                {
                    // NOTE: assume "=cut" terminates POD.
                    i64 k = j;
                    while(k < strmax && s[k] != '\n')
                    {
                        k += 1;
                    }
                    if(k < strmax)
                    {
                        k += 1;
                    }
                    j = k;
                    break;
                }
                while(j < strmax && s[j] != '\n')
                {
                    j += 1;
                }
                if(j < strmax)
                {
                    j += 1;
                }
                if(j >= strmax)
                {
                    break;
                }
                // NOTE: if this line starts a new POD directive we still
                // consume it as part of the comment block, so keep going.
            }
            Token token = { i, j - i, TokenBaseKind_Comment, 0 };
            token_list_push(arena, list, &token);
            i = j;
        }

        // NOTE: Line comment.
        else if(chr == '#')
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

        // NOTE: String literal (single or double quotes).
        else if(chr == '"' || chr == '\'')
        {
            i64 j = i+1;
            for(; j < strmax && s[j] != chr; j += 1)
            {
                if(s[j] == '\\')
                {
                    j += 1;
                }
            }
            i64 size = (j < strmax && s[j] == chr) ? (j+1 - i) : (j - i);
            Token token = { i, size, TokenBaseKind_LiteralString, 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += size;
        }

        // NOTE: Sigil variable (scalar / array / hash / sub / typeglob).
        // Grammar (per perldata): sigil [$@%&*], optionally $# for last-index,
        // then either a bareword identifier, a {braced} block, a package chain
        // with ::, a $$ / $! style punctuation special var, a digit-only match
        // var $1..$99, or a caret var $^X. Anything else is a binary operator
        // (e.g. modulo %, glob deref *, bitwise &).
        //
        // Because % * & are also arithmetic/bitwise ops, they only count as
        // sigils when the previous token doesn't end a term (i.e. not right
        // after an identifier / literal / close bracket).
        else if(F4_Perl_CharIsSigil(chr) &&
                (chr == '$' || chr == '@' || !F4_Perl_PrevEndsTerm(state->last_kind)))
        {
            i64 j = i+1;
            b32 is_name = 0;
            if(j < strmax)
            {
                u8 next = s[j];
                if(F4_Perl_IdentStart(next))
                {
                    is_name = 1;
                    for(; j < strmax && F4_Perl_IdentCont(s[j]); j += 1);
                    // NOTE: Package-qualified chain foo::bar::baz.
                    while(j+1 < strmax && s[j] == ':' && s[j+1] == ':')
                    {
                        i64 k = j+2;
                        if(k < strmax && F4_Perl_IdentStart(s[k]))
                        {
                            j = k;
                            for(; j < strmax && F4_Perl_IdentCont(s[j]); j += 1);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else if(next == '{')
                {
                    // NOTE: block deref ${...} — emit just the sigil and let
                    // the next iteration handle '{'.
                    is_name = 1;
                }
                else if(next == '$')
                {
                    // NOTE: $$ is the process-id special var when nothing
                    // follows that looks like a name; otherwise $$ref is a
                    // deref chain so we emit just the leading sigil.
                    is_name = 1;
                    if(!(j+1 < strmax &&
                         (F4_Perl_IdentStart(s[j+1]) ||
                          (s[j+1] >= '0' && s[j+1] <= '9') ||
                          s[j+1] == '{')))
                    {
                        j += 1;
                    }
                }
                else if(next == '^' && chr == '$')
                {
                    // NOTE: $^V, $^O, etc.
                    if(j+1 < strmax &&
                       (F4_Perl_IdentStart(s[j+1]) || s[j+1] == '^' || s[j+1] == '?'))
                    {
                        is_name = 1;
                        j += 2;
                        for(; j < strmax && F4_Perl_IdentCont(s[j]); j += 1);
                    }
                }
                else if(chr == '$' && next >= '0' && next <= '9')
                {
                    // NOTE: $0, $1, ... digit-only match variables.
                    is_name = 1;
                    for(j += 1; j < strmax && s[j] >= '0' && s[j] <= '9'; j += 1);
                }
                else if(chr == '$' && F4_Perl_IsSpecialVarChar(next))
                {
                    // NOTE: punctuation specials $!, $/, $., $$, $#...
                    is_name = 1;
                    j += 1;
                }
            }
            if(is_name)
            {
                Token token = { i, j - i, TokenBaseKind_Identifier, 0 };
                token_list_push(arena, list, &token);
                state->last_kind = token.kind;
                i += token.size;
            }
            else
            {
                // NOTE: standalone % * & (and $@ in weird positions) act as
                // binary operators.
                Token token = { i, 1, TokenBaseKind_Operator, 0 };
                token_list_push(arena, list, &token);
                state->last_kind = token.kind;
                i += 1;
            }
        }

        // NOTE: Identifier / keyword.
        else if(F4_Perl_IdentStart(chr))
        {
            Token token = { i, 1, TokenBaseKind_Identifier, 0 };
            for(i64 j = i+1; j < strmax && F4_Perl_IdentCont(s[j]);
                j += 1, token.size += 1);
            String_Const_u8 name = { s + token.pos, (u64)token.size };
            if(F4_Perl_IsKeyword(name))
            {
                token.kind = TokenBaseKind_Keyword;
            }
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += token.size;
        }

        // NOTE: Numeric literal. Guard '.'-consumption so '..' stays a range
        // operator (a '.' is only treated as a decimal point when followed
        // by another digit).
        else if((chr >= '0' && chr <= '9') ||
                (chr == '.' && i+1 < strmax && s[i+1] >= '0' && s[i+1] <= '9'))
        {
            b32 is_float = (chr == '.');
            i64 j = i;
            if(chr == '0' && i+1 < strmax && (s[i+1] == 'x' || s[i+1] == 'X'))
            {
                j = i+2;
                for(; j < strmax && (F4_Perl_IsHexDigit(s[j]) || s[j] == '_');
                    j += 1);
            }
            else if(chr == '0' && i+1 < strmax && (s[i+1] == 'b' || s[i+1] == 'B'))
            {
                j = i+2;
                for(; j < strmax && (s[j] == '0' || s[j] == '1' || s[j] == '_');
                    j += 1);
            }
            else
            {
                j += 1;
                for(; j < strmax && ((s[j] >= '0' && s[j] <= '9') || s[j] == '_');
                    j += 1);
                if(j+1 < strmax && s[j] == '.' &&
                   ((s[j+1] >= '0' && s[j+1] <= '9') || s[j+1] == '_'))
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
            }
            Token token = { i, j - i,
                (is_float ? TokenBaseKind_LiteralFloat : TokenBaseKind_LiteralInteger), 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += token.size;
        }

        // NOTE: Brackets.
        else if(chr == '{' || chr == '}')
        {
            Token token = { i, 1,
                (chr == '{') ? TokenBaseKind_ScopeOpen : TokenBaseKind_ScopeClose, 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += 1;
        }
        else if(chr == '(' || chr == '[')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalOpen, 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += 1;
        }
        else if(chr == ')' || chr == ']')
        {
            Token token = { i, 1, TokenBaseKind_ParentheticalClose, 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += 1;
        }

        // NOTE: Statement closes.
        else if(chr == ';' || chr == ',')
        {
            Token token = { i, 1, TokenBaseKind_StatementClose, 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += 1;
        }

        // NOTE: '\' reference operator (\/\%/\/\&/\*) — emit standalone so a
        // following sigil still parses as a variable name.
        else if(chr == '\\')
        {
            Token token = { i, 1, TokenBaseKind_Operator, 0 };
            token_list_push(arena, list, &token);
            // NOTE: don't set last_kind; a backslash doesn't end a term.
            i += 1;
        }

        // NOTE: Operator runs.
        else if(F4_Perl_CharIsSymbol(chr))
        {
            Token token = { i, 1, TokenBaseKind_Operator, 0 };
            for(i64 j = i+1; j < strmax && F4_Perl_CharIsSymbol(s[j]);
                j += 1, token.size += 1);
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
            i += token.size;
        }

        // NOTE: Catch-all.
        else
        {
            Token token = { i, 1, TokenBaseKind_LexError, 0 };
            token_list_push(arena, list, &token);
            state->last_kind = token.kind;
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
    *(F4_Perl_LexerState *)state_ptr = *state;
    return result;
}

internal F4_LANGUAGE_POSCONTEXT(F4_Perl_PosContext)
{
    return 0;
}

internal F4_LANGUAGE_HIGHLIGHT(F4_Perl_Highlight)
{
    // NOTE: base kinds cover everything for now.
}

#endif // FCODER_FLEURY_LANG_PERL_CPP
