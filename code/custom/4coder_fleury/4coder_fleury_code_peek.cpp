
 global b32 global_code_peek_open = 0;

//~ A resolved peek target: the buffer a symbol is defined in, plus a byte range
//~ for its definition (pos_max only matters for the no-tokens fallback).
struct F4_Peek_Loc
{
    b32 found;
    Buffer_ID buffer;
    i64 pos_min;
    i64 pos_max;
};

//~ Resolve a symbol name to a definition location for peeking, trying in order:
//~   1. F4 live custom index (open buffers; exact ranges).
//~   2. ctags table (any project file; opens the buffer on demand).
//~   3. 4coder's built-in code index (keeps C/C++ peek working exactly as before).
function F4_Peek_Loc
F4_Peek_Resolve(Application_Links *app, Arena *scratch, String_Const_u8 name, Buffer_ID current_buffer)
{
    F4_Peek_Loc loc = {};
    if(name.size == 0) return loc;

    F4_Index_Note *note = F4_Index_LookupNote(name);
    if(note != 0 && note->file != 0)
    {
        loc.found = 1;
        loc.buffer = note->file->buffer;
        loc.pos_min = note->range.min;
        loc.pos_max = note->range.max;
        return loc;
    }

    Buffer_ID tb = 0;
    i64 tp = 0;
    if(F4_Tags_ResolveByName(app, scratch, name, current_buffer, &tb, &tp))
    {
        Buffer_Cursor a = buffer_compute_cursor(app, tb, seek_pos(tp));
        Buffer_Cursor b = buffer_compute_cursor(app, tb, seek_line_col(a.line + 1, 0));
        loc.found = 1;
        loc.buffer = tb;
        loc.pos_min = tp;
        loc.pos_max = (b.pos > a.pos) ? b.pos : (tp + 1);
        return loc;
    }

    Code_Index_Note *cin = code_index_note_from_string(name);
    if(cin != 0 && cin->file != 0)
    {
        loc.found = 1;
        loc.buffer = cin->file->buffer;
        loc.pos_min = cin->pos.min;
        loc.pos_max = cin->pos.max;
        return loc;
    }

    return loc;
}

function void
F4_CodePeek_Render(Application_Links *app, View_ID view_id, Face_ID face_id,
                   Buffer_ID buffer, Frame_Info frame_info)
{
    Scratch_Block scratch(app);
    Buffer_ID peek_buf = get_buffer_by_name(app, string_u8_litexpr("*peek*"), AccessFlag_Read);

    if(!global_code_peek_open && buffer != peek_buf)
    {
        return;
    }

    Rect_f32 view_rect = view_get_screen_rect(app, view_id);

    struct Peek
    {
        Peek *next;
        String_Const_u8 string;
        F4_Peek_Loc loc;
    };

    Peek *first = 0;
    Peek *last = 0;
    int peek_count = 0;

#define PushPeek(str) \
{\
Peek *p = push_array_zero(scratch, Peek, 1);\
p->string = str;\
p->loc = F4_Peek_Resolve(app, scratch, str, buffer);\
if(p->loc.found)\
{\
if(last == 0)\
{\
first = last = p;\
}\
else\
{\
last->next = p;\
last = last->next;\
}\
peek_count += 1;\
}\
}

    PushPeek(push_token_or_word_under_active_cursor(app, scratch));

    // NOTE(rjf): Push identifiers left around in *peek*.
    {
        Token_Array token_array = get_token_array_from_buffer(app, peek_buf);
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, 0);
        for(;token_it_inc_non_whitespace(&it);)
        {
            Token *token = token_it_read(&it);
            if(token == 0)
            {
                break;
            }
            if(token->kind == TokenBaseKind_Identifier)
            {
                PushPeek(push_buffer_range(app, scratch, peek_buf, Ii64(token->pos, token->pos + token->size)));
            }
        }
    }

#undef PushPeek

    f32 peek_height = (f32)((view_rect.y1 - view_rect.y0) * (0.5f + 0.4f*(clamp_top(peek_count / 4, 1)))) / peek_count;
    Rect_f32 rect = {0};
    {
        rect.x0 = view_rect.x0;
        rect.x1 = view_rect.x1;
        rect.y0 = view_rect.y1 - peek_height*peek_count;
        rect.y1 = rect.y0 + peek_height;
    }

    for(Peek *peek = first; peek; peek = peek->next)
    {
        F4_Peek_Loc loc = peek->loc;
        {
            Buffer_ID match_buffer = loc.buffer;

            F4_DrawTooltipRect(app, rect);

            // NOTE(rjf): Draw code.
            {
                Rect_f32 inner_rect = rect_inner(rect, 30);

                Buffer_Point buffer_point =
                {
                    get_line_number_from_pos(app, match_buffer, loc.pos_min),
                    0,
                };
                Text_Layout_ID text_layout_id = text_layout_create(app, match_buffer, inner_rect, buffer_point);

                Rect_f32 prev_prev_clip = draw_set_clip(app, inner_rect);
                {
                    Token_Array match_token_array = get_token_array_from_buffer(app, match_buffer);
                    if(match_token_array.tokens != 0)
                    {
                        F4_SyntaxHighlight(app, text_layout_id, &match_token_array);
                    }
                    else
                    {
                        Range_i64 visible_range = Ii64(loc.pos_min, loc.pos_max);
                        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
                    }

                    draw_text_layout_default(app, text_layout_id);
                }
                draw_set_clip(app, prev_prev_clip);
                text_layout_free(app, text_layout_id);
            }
        }

        f32 height = (rect.y1 - rect.y0);
        rect.y0 += height;
        rect.y1 += height;
    }

}

CUSTOM_COMMAND_SIG(f4_code_peek)
CUSTOM_DOC("Toggles code peek.")
{
    global_code_peek_open ^= 1;
}

CUSTOM_COMMAND_SIG(f4_code_peek_yank)
CUSTOM_DOC("Yanks the current cursor identifier into the *peek* buffer.")
{
    Scratch_Block scratch(app);
    String_Const_u8 string = push_token_or_word_under_active_cursor(app, scratch);
    Buffer_ID buffer = get_buffer_by_name(app, string_u8_litexpr("*peek*"), Access_Read | Access_Write);
    F4_Peek_Loc loc = F4_Peek_Resolve(app, scratch, string, buffer);
    if(buffer != 0 && loc.found)
    {
        buffer_replace_range(app, buffer, Ii64(buffer_get_size(app, buffer)), string_u8_litexpr("\n"));
        buffer_replace_range(app, buffer, Ii64(buffer_get_size(app, buffer)), string);
    }
}

CUSTOM_COMMAND_SIG(f4_code_peek_clear)
CUSTOM_DOC("Clears the *peek* buffer.")
{
    Buffer_ID buffer = get_buffer_by_name(app, string_u8_litexpr("*peek*"), Access_Read | Access_Write);
    if(buffer)
    {
        clear_buffer(app, buffer);
    }
}
