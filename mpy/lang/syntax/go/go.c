#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo;

void
unload(yed_plugin* self);
void
syntax_go_line_handler(yed_event* event);
void
syntax_go_frame_handler(yed_event* event);
void
syntax_go_buff_mod_pre_handler(yed_event* event);
void
syntax_go_buff_mod_post_handler(yed_event* event);

int
yed_plugin_boot(yed_plugin* self)
{
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;

    char*             kwds[] = {
        "func",
        "interface",
        "package",
        "const",
        "import",
        "var",

    };
    char* special_kwds[] = {
        "import",
        "package",
        "defer",
        "go",
        "make",
        "chan",
    };
    char* control_flow[] = {
        "break",
        "case",
        "continue",
        "default",
        "fallthrough",
        "else",
        "for",
        "goto",
        "if",
        "return",
        "switch",
        "range",
        "select",
    };
    char* typenames[] = {
        "int",     "int8",      "int16",      "int32",  "int64",   "uint",
        "uint8",   "uint16",    "uint32",     "uint64", "uintptr", "float",
        "float32", "complex64", "complex128", "string", "byte",    "rune",
        "bool",    "map",       "struct",     "type",
    };

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    frame.kind         = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn           = syntax_go_frame_handler;
    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = syntax_go_line_handler;
    buff_mod_pre.kind  = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn    = syntax_go_buff_mod_pre_handler;
    buff_mod_post.kind = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn   = syntax_go_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);

    highlight_info_make(&hinfo);

    ARRAY_LOOP(kwds)
        highlight_add_kwd(&hinfo, *it, HL_KEY);
    ARRAY_LOOP(special_kwds)
    ARRAY_LOOP(control_flow)
        highlight_add_kwd(&hinfo, *it, HL_CF);
    ARRAY_LOOP(typenames)
        highlight_add_kwd(&hinfo, *it, HL_TYPE);
    highlight_add_kwd(&hinfo, "nil", HL_CON);
    highlight_add_kwd(&hinfo, "stdin", HL_CON);
    highlight_add_kwd(&hinfo, "stdout", HL_CON);
    highlight_add_kwd(&hinfo, "stderr", HL_CON);
    highlight_suffixed_words(&hinfo, '(', HL_CALL);
    highlight_numbers(&hinfo);
    highlight_within(&hinfo, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo, "'", "'", '\\', 1, HL_CHAR);
    highlight_to_eol_from(&hinfo, "//", HL_COMMENT);
    highlight_within_multiline(&hinfo, "/*", "*/", 0, HL_COMMENT);

    ys->redraw = 1;

    return 0;
}

void
unload(yed_plugin* self)
{
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void
syntax_go_frame_handler(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame || !frame->buffer || frame->buffer->kind != BUFF_KIND_FILE ||
        frame->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo, event);
}

void
syntax_go_line_handler(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame || !frame->buffer || frame->buffer->kind != BUFF_KIND_FILE ||
        frame->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_line(&hinfo, event);
}

void
syntax_go_buff_mod_pre_handler(yed_event* event)
{
    if (event->buffer == NULL || event->buffer->kind != BUFF_KIND_FILE ||
        event->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo, event);
}

void
syntax_go_buff_mod_post_handler(yed_event* event)
{
    if (event->buffer == NULL || event->buffer->kind != BUFF_KIND_FILE ||
        event->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo, event);
}
