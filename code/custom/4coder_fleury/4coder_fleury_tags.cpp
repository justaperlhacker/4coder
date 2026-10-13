/* date = October 12th 2026

   4coder_fleury_tags.cpp - Application_Links-dependent parts of ctags support:
   file IO, staleness reload, jump, go-to-definition, and the commands. The
   pure parser/table/lookup live in 4coder_fleury_tags_parse.h (pulled in via
   4coder_fleury_tags.h) so they are unit-testable. */

// TOP

#include "4coder_fleury_tags.h"

//~ Minimal substring finder (pattern-search fallback); returns start byte or -1.
internal i64
f4_tag_find(String_Const_u8 hay, String_Const_u8 needle)
{
    if(needle.size == 0 || needle.size > hay.size) return -1;
    i64 last = (i64)hay.size - (i64)needle.size;
    for(i64 i = 0; i <= last; i += 1)
    {
        u64 j = 0;
        for(; j < needle.size; j += 1)
        {
            if(hay.str[i + (i64)j] != needle.str[j]) break;
        }
        if(j == needle.size) return i;
    }
    return -1;
}

internal String_Const_u8
f4_tag_slurp(Arena *arena, String_Const_u8 path)
{
    String_Const_u8 text = {};
    if(path.size == 0) return text;
    char stack_buf[1024];
    char *cstr = stack_buf;
    if(path.size + 1 > sizeof(stack_buf))
    {
        cstr = push_array(arena, char, path.size + 1);
    }
    for(u64 i = 0; i < path.size; i += 1) cstr[i] = (char)path.str[i];
    cstr[path.size] = 0;
    FILE *f = fopen(cstr, "rb");
    if(!f) return text;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(size <= 0)
    {
        fclose(f);
        return text;
    }
    text.str = push_array(arena, u8, (u64)size + 1);
    text.size = (u64)size;
    fread(text.str, 1, (size_t)size, f);
    text.str[text.size] = 0;
    fclose(f);
    return text;
}

internal String_Const_u8
f4_tag_dir_of(Arena *arena, String_Const_u8 path)
{
    i64 last = (i64)path.size - 1;
    while(last >= 0 && path.str[last] != '/' && path.str[last] != '\\')
    {
        last -= 1;
    }
    String_Const_u8 dir = { path.str, (u64)(last > 0 ? last : 0) };
    return push_string_copy(arena, dir);
}

//~ Resolve config `tags_file` against the hot directory if it's relative.
internal String_Const_u8
f4_tag_resolve_path(Application_Links *app, Arena *arena)
{
    Scratch_Block scratch(app);
    String_Const_u8 cfg = def_get_config_string(scratch, vars_save_string_lit("tags_file"));
    String_Const_u8 hot = push_hot_directory(app, arena);
    if(cfg.size > 0)
    {
        if(cfg.str[0] == '/' || (cfg.size > 1 && cfg.str[1] == ':'))
        {
            return push_string_copy(arena, cfg);
        }
        return push_u8_stringf(arena, "%.*s/%.*s",
                               string_expand(hot), string_expand(cfg));
    }
    return push_u8_stringf(arena, "%.*s/tags", string_expand(hot));
}

function b32
F4_Tags_EnsureFresh(Application_Links *app, F4_TagTable *table)
{
    Scratch_Block scratch(app);
    String_Const_u8 path = f4_tag_resolve_path(app, scratch);
    if(path.size == 0) return 0;
    File_Attributes attr = system_quick_file_attributes(scratch, path);
    if(attr.size == 0)
    {
        return 0;   // missing/empty tags file (all-zero attrs on stat failure)
    }
    if(table->loaded && table->source_size == attr.size &&
       table->source_mtime == attr.last_write_time)
    {
        return 1;
    }
    //~ Fresh parse into a new arena (reloads are ctags-regen-triggered and rare;
    //~ the previous arena is released when the table is reassigned).
    table->loaded = 0;
    table->arena = make_arena_system(KB(64));
    F4_Tags_Reset(table);
    String_Const_u8 text = f4_tag_slurp(&table->arena, path);
    if(text.size == 0)
    {
        return 0;
    }
    F4_Tags_Parse(&table->arena, text, table);
    table->source_path = push_string_copy(&table->arena, path);
    table->source_size = attr.size;
    table->source_mtime = attr.last_write_time;
    table->loaded = 1;
    return 1;
}

//~ Make a possibly project-relative tag path absolute against the tags dir.
internal String_Const_u8
f4_tag_absolutize(Arena *arena, String_Const_u8 table_dir, String_Const_u8 file)
{
    if(file.size == 0) return file;
    u8 c0 = file.str[0];
    if(c0 == '/' || (file.size > 1 && file.str[1] == ':'))
    {
        return file;
    }
    if(table_dir.size == 0) return file;
    return push_u8_stringf(arena, "%.*s/%.*s",
                           string_expand(table_dir), string_expand(file));
}

//~ Resolve a tag into (buffer, pos), opening the target file if it isn't
//~ already loaded. Used by both the jump command and code peek.
function b32
F4_Tags_Resolve(Application_Links *app, Arena *scratch, F4_Tag *tag,
                Buffer_ID *buffer_out, i64 *pos_out)
{
    if(tag == 0) return 0;
    String_Const_u8 table_dir = f4_tag_dir_of(scratch, f4_tag_table.source_path);
    String_Const_u8 abs_path = f4_tag_absolutize(scratch, table_dir, tag->file);

    Buffer_ID buffer = 0;
    if(!open_file(app, &buffer, abs_path, 0, 1))
    {
        if(!open_file(app, &buffer, tag->file, 0, 1))
        {
            return 0;
        }
    }
    if(!buffer_exists(app, buffer)) return 0;

    i64 target_pos = -1;
    if(tag->line > 0)
    {
        i64 line_count = buffer_get_line_count(app, buffer);
        i64 line = tag->line;
        if(line > line_count) line = line_count;
        Buffer_Cursor c = buffer_compute_cursor(app, buffer, seek_line_col(line, 0));
        target_pos = c.pos;
    }
    else if(tag->pattern.size > 0)
    {
        String_Const_u8 whole = push_whole_buffer(app, scratch, buffer);
        i64 found = f4_tag_find(whole, tag->pattern);
        if(found >= 0)
        {
            target_pos = found;
        }
    }
    if(target_pos < 0)
    {
        target_pos = 0;
    }

    *buffer_out = buffer;
    *pos_out = target_pos;
    return 1;
}

//~ Move `view` to (buffer,pos): record a jump point, switch buffers, center the
//~ line, and put cursor+mark at pos. Shared by jump and go-to-def.
internal void
f4_tag_goto_view(Application_Links *app, View_ID view, Buffer_ID buffer, i64 pos)
{
    Rect_f32 region = view_get_buffer_region(app, view);
    f32 view_height = rect_height(region);
    point_stack_push_view_cursor(app, view);
    view_set_buffer(app, view, buffer, 0);
    i64 line_number = get_line_number_from_pos(app, buffer, pos);
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
    scroll.position.line_number = line_number;
    scroll.target.line_number = line_number;
    scroll.position.pixel_shift.y = scroll.target.pixel_shift.y = -view_height*0.5f;
    view_set_buffer_scroll(app, view, scroll, SetBufferScroll_SnapCursorIntoView);
    view_set_cursor_and_preferred_x(app, view, seek_pos(pos));
    view_set_mark(app, view, seek_pos(pos));
}

function b32
F4_Tags_Jump(Application_Links *app, View_ID view, F4_Tag *tag)
{
    if(view == 0 || tag == 0) return 0;
    Scratch_Block scratch(app);
    Buffer_ID buffer = 0;
    i64 pos = 0;
    if(!F4_Tags_Resolve(app, scratch, tag, &buffer, &pos))
    {
        return 0;
    }
    f4_tag_goto_view(app, view, buffer, pos);
    return 1;
}

function b32
F4_Tags_Enabled(Application_Links *app)
{
    (void)app;
    return def_get_config_b32(vars_save_string_lit("disable_tags_fallback")) == 0;
}

//~ Resolve an identifier *string* to a (buffer,pos) location via tags, honouring
//~ the same-directory Pick heuristic relative to `from_buffer`. Returns 0 when
//~ tags are disabled, absent, or the name is not found. Shared by go-to-def
//~ fallback and code peek so both agree on where a symbol lives.
function b32
F4_Tags_ResolveByName(Application_Links *app, Arena *scratch, String_Const_u8 name,
                      Buffer_ID from_buffer, Buffer_ID *buffer_out, i64 *pos_out)
{
    if(!F4_Tags_Enabled(app) || name.size == 0) return 0;
    if(!F4_Tags_EnsureFresh(app, &f4_tag_table)) return 0;
    F4_Tag *chain = F4_Tags_Lookup(&f4_tag_table, name);
    if(chain == 0) return 0;
    String_Const_u8 from_file = (from_buffer != 0)
        ? push_buffer_file_name(app, scratch, from_buffer)
        : (String_Const_u8){0, 0};
    F4_Tag *best = F4_Tags_Pick(chain, from_file);
    return F4_Tags_Resolve(app, scratch, best, buffer_out, pos_out);
}

function b32
F4_Tags_GoToDef(Application_Links *app, View_ID view)
{
    if(view == 0) return 0;
    Scratch_Block scratch(app);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    String_Const_u8 name = push_token_or_word_under_active_cursor(app, scratch);
    Buffer_ID out_buffer = 0;
    i64 out_pos = 0;
    if(!F4_Tags_ResolveByName(app, scratch, name, buffer, &out_buffer, &out_pos))
    {
        return 0;
    }
    f4_tag_goto_view(app, view, out_buffer, out_pos);
    return 1;
}

//~ --------------------------------------------------------------------------
//~ Commands
//~ --------------------------------------------------------------------------

CUSTOM_COMMAND_SIG(f4_tags_go_to_definition)
CUSTOM_DOC("Go to the definition of the identifier under the cursor using the ctags tag file.")
{
    View_ID view = get_active_view(app, Access_Always);
    if(!F4_Tags_GoToDef(app, view))
    {
        print_message(app, S8Lit("f4_tags: no tag match (is a ctags 'tags' file present? try f4_rebuild_tags)\n"));
    }
}

CUSTOM_COMMAND_SIG(f4_rebuild_tags)
CUSTOM_DOC("Run ctags recursively in the hot directory to (re)generate the tags file.")
{
    Scratch_Block scratch(app);
    String_Const_u8 hot = push_hot_directory(app, scratch);
    if(hot.size == 0)
    {
        print_message(app, S8Lit("f4_rebuild_tags: no hot directory\n"));
        return;
    }
    String_Const_u8 langs = def_get_config_string(scratch, vars_save_string_lit("tags_languages"));
    if(langs.size == 0)
    {
        langs = S8Lit("Python,Perl,Lua");
    }
    String_Const_u8 cmd = push_u8_stringf(
        scratch,
        "ctags -R --languages=%.*s --fields=+n+S --exclude=.git --exclude=build --exclude=tags --exclude='*.so' -f tags .",
        string_expand(langs));
    exec_system_command(app, 0, buffer_identifier(0), hot, cmd, 0);
    print_message(app, push_u8_stringf(scratch,
        "f4_rebuild_tags: running ctags in %.*s (languages: %.*s)\n",
        string_expand(hot), string_expand(langs)));
}

// BOTTOM
