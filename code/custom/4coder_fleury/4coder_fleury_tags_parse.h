/* date = October 12th 2026

   4coder_fleury_tags_parse.h - The pure, allocation-only part of the ctags
   support: the tag/table types and the parser + hash lookup. Split out so the
   standalone test suite can exercise it with just a few 4coder primitives
   (Arena, String_Const_u8, push_array_zero/push_string_copy, string_match,
   table_hash_u8) and no Application_Links.

   4coder_fleury_tags.h includes this and adds the parts that need the editor
   runtime (file IO, staleness reload, jump, commands). */

#ifndef FCODER_FLEURY_TAGS_PARSE_H
#define FCODER_FLEURY_TAGS_PARSE_H

#define F4_TAG_SLOTS 4096

//~ A single tag: a symbol name and its definition location. `file` is stored
//~ exactly as the tags file wrote it (may be project-relative).
struct F4_Tag
{
    F4_Tag *name_next;
    String_Const_u8 name;
    String_Const_u8 file;
    i64 line;              // 1-based; 0 if the address was a /regex/
    String_Const_u8 pattern;  // regex text (sans delimiters) when line == 0
};

struct F4_TagTable
{
    b32 loaded;
    Arena arena;
    F4_Tag *buckets[F4_TAG_SLOTS];
    i64 count;
    String_Const_u8 source_path;
    u64 source_mtime;
    u64 source_size;
};

internal u64
f4_tag_slot(String_Const_u8 name)
{
    u64 h = table_hash_u8(name.str, name.size);
    return h % F4_TAG_SLOTS;
}

internal F4_Tag *
f4_tag_insert(F4_TagTable *table, Arena *arena,
              String_Const_u8 name, String_Const_u8 file,
              i64 line, String_Const_u8 pattern)
{
    F4_Tag *tag = push_array_zero(arena, F4_Tag, 1);
    tag->name = push_string_copy(arena, name);
    tag->file = push_string_copy(arena, file);
    tag->line = line;
    tag->pattern = pattern.size ? push_string_copy(arena, pattern) : pattern;
    u64 slot = f4_tag_slot(name);
    tag->name_next = table->buckets[slot];
    table->buckets[slot] = tag;
    table->count += 1;
    return tag;
}

//~ Read the next TAB-delimited field from *cursor. Sets *last_out when the
//~ field ran to end-of-line (no trailing tab).
internal String_Const_u8
f4_tag_next_field(String_Const_u8 line, i64 *cursor, b32 *last_out)
{
    i64 start = *cursor;
    i64 i = start;
    *last_out = 0;
    while(i < (i64)line.size && line.str[i] != '\t')
    {
        i += 1;
    }
    String_Const_u8 field = { line.str + start, (u64)(i - start) };
    if(i >= (i64)line.size)
    {
        *last_out = 1;
        *cursor = i;
    }
    else
    {
        *cursor = i + 1;
    }
    return field;
}

internal b32
f4_tag_parse_i64(String_Const_u8 s, i64 *out)
{
    if(s.size == 0) return 0;
    i64 v = 0;
    for(u64 i = 0; i < s.size; i += 1)
    {
        u8 c = s.str[i];
        if(c < '0' || c > '9')
        {
            return 0;
        }
        v = v*10 + (c - '0');
        if(v > 1000000000LL)
        {
            return 0;
        }
    }
    *out = v;
    return 1;
}

//~ Decode a ctags address field into (line, pattern).
//~   /re/;"  or  ?re?;"   -> line 0, pattern = re (no delimiters)
//~   N;"     or  N        -> line = N, pattern empty
internal void
f4_tag_decode_address(String_Const_u8 addr, i64 *line_out, String_Const_u8 *pattern_out)
{
    *line_out = 0;
    *pattern_out = (String_Const_u8){0, 0};
    if(addr.size == 0) return;
    u8 c0 = addr.str[0];
    if(c0 == '/' || c0 == '?')
    {
        u64 end = addr.size;
        while(end > 0 && (addr.str[end-1] == '"' || addr.str[end-1] == ';'))
        {
            end -= 1;
        }
        if(end > 0 && (addr.str[end-1] == '/' || addr.str[end-1] == '?'))
        {
            end -= 1;
        }
        if(end > 1)
        {
            pattern_out->str = addr.str + 1;
            pattern_out->size = end - 1;
        }
    }
    else
    {
        i64 n_end = (i64)addr.size;
        while(n_end > 0 && (addr.str[n_end-1] == ';' || addr.str[n_end-1] == '"'))
        {
            n_end -= 1;
        }
        String_Const_u8 num = { addr.str, (u64)n_end };
        f4_tag_parse_i64(num, line_out);
    }
}

function void
F4_Tags_Reset(F4_TagTable *table)
{
    for(i32 i = 0; i < F4_TAG_SLOTS; i += 1)
    {
        table->buckets[i] = 0;
    }
    table->count = 0;
}

function i64
F4_Tags_Parse(Arena *arena, String_Const_u8 text, F4_TagTable *table)
{
    i64 before = table->count;
    i64 i = 0;
    i64 n = (i64)text.size;
    while(i < n)
    {
        i64 line_start = i;
        i64 line_end = i;
        while(line_end < n && text.str[line_end] != '\n')
        {
            line_end += 1;
        }
        i = line_end + 1;
        i64 content_end = line_end;
        if(content_end > line_start && text.str[content_end-1] == '\r')
        {
            content_end -= 1;
        }
        String_Const_u8 line = { text.str + line_start, (u64)(content_end - line_start) };
        if(line.size < 6) continue;
        if(line.str[0] == '!' && string_match(S8Lit("!_TAG_"), (String_Const_u8){line.str, 6}))
        {
            continue;
        }
        i64 cur = 0;
        b32 last = 0;
        String_Const_u8 name = f4_tag_next_field(line, &cur, &last);
        if(name.size == 0 || last) continue;
        String_Const_u8 file = f4_tag_next_field(line, &cur, &last);
        if(last) continue;
        String_Const_u8 addr = f4_tag_next_field(line, &cur, &last);
        i64 line_num = 0;
        String_Const_u8 pattern;
        f4_tag_decode_address(addr, &line_num, &pattern);
        while(!last)
        {
            String_Const_u8 field = f4_tag_next_field(line, &cur, &last);
            if(field.size > 5 && field.str[0]=='l' && field.str[1]=='i' &&
               field.str[2]=='n' && field.str[3]=='e' && field.str[4]==':')
            {
                String_Const_u8 num = { field.str + 5, field.size - 5 };
                i64 v = 0;
                if(f4_tag_parse_i64(num, &v) && v > 0)
                {
                    line_num = v;
                    pattern = (String_Const_u8){0, 0};
                }
            }
        }
        f4_tag_insert(table, arena, name, file, line_num, pattern);
    }
    return table->count - before;
}

function F4_Tag *
F4_Tags_Lookup(F4_TagTable *table, String_Const_u8 name)
{
    if(!table->loaded || name.size == 0) return 0;
    u64 slot = f4_tag_slot(name);
    for(F4_Tag *t = table->buckets[slot]; t; t = t->name_next)
    {
        if(string_match(t->name, name))
        {
            return t;
        }
    }
    return 0;
}

//~ Number of matching leading bytes between two paths, used as the "how close
//~ are these files" score when disambiguating a duplicate-name tag chain.
//~ Counting the whole prefix (not just up to the last '/') lets identical or
//~ same-dir files win even in flat single-directory tags files.
internal i64
f4_tag_common_dir_prefix(String_Const_u8 a, String_Const_u8 b)
{
    i64 n = (i64)(a.size < b.size ? a.size : b.size);
    i64 i = 0;
    for(; i < n; i += 1)
    {
        if(a.str[i] != b.str[i]) break;
    }
    return i;
}

function F4_Tag *
F4_Tags_Pick(F4_Tag *chain, String_Const_u8 from_file)
{
    if(chain == 0) return 0;
    if(chain->name_next == 0) return chain;
    F4_Tag *best = chain;
    i64 best_score = f4_tag_common_dir_prefix(chain->file, from_file);
    for(F4_Tag *t = chain->name_next; t; t = t->name_next)
    {
        i64 score = f4_tag_common_dir_prefix(t->file, from_file);
        if(score > best_score)
        {
            best_score = score;
            best = t;
        }
    }
    return best;
}

#endif // FCODER_FLEURY_TAGS_PARSE_H
