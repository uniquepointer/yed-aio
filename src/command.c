#include "internal.h"

void yed_init_commands(void) {
    ys->commands         = tree_make_c(yed_command_name_t, yed_command, strcmp);
    ys->default_commands = tree_make_c(yed_command_name_t, yed_command, strcmp);
    yed_set_default_commands();
    ys->cmd_buff = array_make(char);
}

yed_command yed_get_command(char *name) {
    tree_it(yed_command_name_t, yed_command) it;

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

void yed_set_command(char *name, yed_command command) {
    tree_it(yed_command_name_t, yed_command)  it;

    it = tree_lookup(ys->commands, name);

    if (tree_it_good(it)) {
        tree_insert(ys->commands, name, command);
    } else {
        tree_insert(ys->commands, strdup(name), command);
    }
}

void yed_unset_command(char *name) {
    tree_it(yed_command_name_t, yed_command)  it;
    char                                     *old_key;

    it = tree_lookup(ys->commands, name);

    if (tree_it_good(it)) {
        old_key = tree_it_key(it);
        tree_delete(ys->commands, name);
        free(old_key);
    }
}

void yed_set_default_command(char *name, yed_command command) {
    tree_it(yed_command_name_t, yed_command)  it;

    it = tree_lookup(ys->default_commands, name);

    if (tree_it_good(it)) {
        tree_insert(ys->default_commands, name, command);
    } else {
        tree_insert(ys->default_commands, strdup(name), command);
    }
}

void yed_set_default_commands(void) {
#define SET_DEFAULT_COMMAND(name1, name2)                         \
do {                                                              \
    yed_set_command(name1, &yed_default_command_##name2);         \
    yed_set_default_command(name1, &yed_default_command_##name2); \
} while (0)

    SET_DEFAULT_COMMAND("command-prompt",        command_prompt);
    SET_DEFAULT_COMMAND("quit",                  quit);
    SET_DEFAULT_COMMAND("reload",                reload);
    SET_DEFAULT_COMMAND("echo",                  echo);
    SET_DEFAULT_COMMAND("cursor-down",           cursor_down);
    SET_DEFAULT_COMMAND("cursor-up",             cursor_up);
    SET_DEFAULT_COMMAND("cursor-left",           cursor_left);
    SET_DEFAULT_COMMAND("cursor-right",          cursor_right);
    SET_DEFAULT_COMMAND("cursor-line-begin",     cursor_line_begin);
    SET_DEFAULT_COMMAND("cursor-line-end",       cursor_line_end);
    SET_DEFAULT_COMMAND("cursor-next-word",      cursor_next_word);
    SET_DEFAULT_COMMAND("cursor-prev-paragraph", cursor_prev_paragraph);
    SET_DEFAULT_COMMAND("cursor-next-paragraph", cursor_next_paragraph);
    SET_DEFAULT_COMMAND("cursor-buffer-begin",   cursor_buffer_begin);
    SET_DEFAULT_COMMAND("cursor-buffer-end",     cursor_buffer_end);
    SET_DEFAULT_COMMAND("cursor-line",           cursor_line);
    SET_DEFAULT_COMMAND("buffer-new",            buffer_new);
    SET_DEFAULT_COMMAND("frame",                 frame);
    SET_DEFAULT_COMMAND("frames-list",           frames_list);
    SET_DEFAULT_COMMAND("frame-set-buffer",      frame_set_buffer);
    SET_DEFAULT_COMMAND("frame-new-file",        frame_new_file);
    SET_DEFAULT_COMMAND("frame-split-new-file",  frame_split_new_file);
    SET_DEFAULT_COMMAND("frame-next",            frame_next);
    SET_DEFAULT_COMMAND("insert",                insert);
    SET_DEFAULT_COMMAND("delete-back",           delete_back);
    SET_DEFAULT_COMMAND("delete-line",           delete_line);
    SET_DEFAULT_COMMAND("write-buffer",          write_buffer);
    SET_DEFAULT_COMMAND("plugin-load",           plugin_load);
    SET_DEFAULT_COMMAND("plugin-unload",         plugin_unload);
    SET_DEFAULT_COMMAND("plugins-list",          plugins_list);
    SET_DEFAULT_COMMAND("plugins-list-dirs",     plugins_list_dirs);
    SET_DEFAULT_COMMAND("plugins-add-dir",       plugins_add_dir);
}

void yed_clear_cmd_buff(void) {
    array_clear(ys->cmd_buff);
    array_zero_term(ys->cmd_buff);
    ys->cmd_cursor_x = 1 + strlen(YED_CMD_PROMPT);
}

void yed_append_to_cmd_buff(char *s) {
    int i, len;

    len = strlen(s);
    for (i = 0; i < len; i += 1) {
        array_push(ys->cmd_buff, s[i]);
    }
}

void yed_append_text_to_cmd_buff(char *s) {
    yed_append_to_cmd_buff(s);
    ys->cmd_cursor_x += strlen(s);
}

void yed_append_non_text_to_cmd_buff(char *s) {
    yed_append_to_cmd_buff(s);
}

void yed_append_int_to_cmd_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    yed_append_text_to_cmd_buff(s);
}

void yed_cmd_buff_push(char c) {
    array_push(ys->cmd_buff, c);
    ys->cmd_cursor_x += 1;
}

void yed_cmd_buff_pop(void) {
    array_pop(ys->cmd_buff);
    ys->cmd_cursor_x -= 1;
}

void yed_draw_command_line(void) {
    yed_set_cursor(1, ys->term_rows);
    append_to_output_buff(TERM_CLEAR_LINE);
    if (ys->accepting_command) {
        append_to_output_buff(YED_CMD_PROMPT);
    }
    append_n_to_output_buff(array_data(ys->cmd_buff), array_len(ys->cmd_buff));
}

void yed_start_command_prompt(void) {
    ys->accepting_command = 1;
    ys->small_message = "";
    yed_clear_cmd_buff();
}

void yed_do_command(void) {
    char *cmd_cpy,
         *cmd_beg,
         *cmd_end,
         *cmd_curs;
    int   cmd_len;
    int   n_split,
          last_was_space;
    char *cmd_split[32];

    array_zero_term(ys->cmd_buff);
    cmd_cpy = malloc(array_len(ys->cmd_buff) + 1);
    memcpy(cmd_cpy, array_data(ys->cmd_buff), array_len(ys->cmd_buff));
    cmd_cpy[array_len(ys->cmd_buff)] = 0;

    yed_clear_cmd_buff();

    cmd_beg = cmd_curs = cmd_cpy;
    cmd_len = strlen(cmd_beg);
    cmd_end = cmd_beg + cmd_len;
    n_split = 0;

    cmd_split[n_split++] = cmd_beg;
    last_was_space       = 0;

    while (cmd_curs != cmd_end) {
        if (*cmd_curs == ' ') {
            last_was_space = 1;
            *cmd_curs      = 0;
        } else {
            if (last_was_space) {
                cmd_split[n_split++] = cmd_curs;
            }
            last_was_space = 0;
        }
        cmd_curs += 1;
    }

    ys->accepting_command = 0;

    yed_execute_command(cmd_beg, n_split - 1, cmd_split + 1);

    free(cmd_cpy);
}

void yed_command_take_key(int key) {
    tree_it(yed_command_name_t, yed_command) it;

    if (key == CTRL_F || key == CTRL_C) {
        ys->accepting_command = 0;
        yed_clear_cmd_buff();
    } else if (key == TAB) {
        array_zero_term(ys->cmd_buff);
        it = tree_gtr(ys->commands, array_data(ys->cmd_buff));
        if (tree_it_good(it)) {
            yed_clear_cmd_buff();
            yed_append_text_to_cmd_buff((char*)tree_it_key(it));
        }
    } else if (key == ENTER) {
        yed_do_command();
        ys->accepting_command = 0;
    } else {
        if (key == 127) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }
    }
}

void yed_default_command_command_prompt(int n_args, char **args) {
    int key;

    if (!ys->accepting_command) {
        yed_start_command_prompt();
    } else {
        sscanf(args[0], "%d", &key);
        yed_command_take_key(key);
    }
}

void yed_default_command_quit(int n_args, char **args) {
    ys->status = YED_QUIT;
}

void yed_default_command_reload(int n_args, char **args) {
    ys->status = YED_RELOAD;
    yed_unload_plugin_libs();
    yed_append_text_to_cmd_buff("issued reload");
}

void yed_default_command_echo(int n_args, char **args) {
    int         i;
    const char *space;

    space = "";
    for (i = 0; i < n_args; i += 1) {
        yed_append_text_to_cmd_buff((char*)space);
        yed_append_text_to_cmd_buff(args[i]);
        space = " ";
    }
}

void yed_default_command_cursor_down(int n_args, char **args) {
    int        rows;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }


    if (n_args == 0) {
        rows = 1;
    } else {
        rows = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, rows);
}

void yed_default_command_cursor_up(int n_args, char **args) {
    int        rows;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }


    if (n_args == 0) {
        rows = -1;
    } else {
        rows = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, rows);
}

void yed_default_command_cursor_left(int n_args, char **args) {
    int        cols;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }


    if (n_args == 0) {
        cols = -1;
    } else {
        cols = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

void yed_default_command_cursor_right(int n_args, char **args) {
    int        cols;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (n_args == 0) {
        cols = 1;
    } else {
        cols = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

void yed_default_command_cursor_line_begin(int n_args, char **args) {
    yed_frame *frame;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);
}

void yed_default_command_cursor_line_end(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    yed_set_cursor_within_frame(frame, line->visual_width + 1, frame->cursor_line);
}

void yed_default_command_cursor_buffer_begin(int n_args, char **args) {
    yed_frame *frame;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    yed_set_cursor_far_within_frame(frame, 1, 1);
    frame->dirty = 1;
}

void yed_default_command_cursor_buffer_end(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *last_line;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    last_line = bucket_array_last(frame->buffer->lines);
    yed_set_cursor_far_within_frame(frame, last_line->visual_width + 1, bucket_array_len(frame->buffer->lines));
}

void yed_default_command_cursor_line(int n_args, char **args) {
    yed_frame *frame;
    int        line;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    sscanf(args[0], "%d", &line);
    yed_set_cursor_far_within_frame(frame, 1, line);
}

void yed_default_command_cursor_next_word(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    yed_cell  *cell_it;
    int        cols;
    char       c;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    cols    = 0;
    cell_it = array_item(line->cells, frame->cursor_col - 1);

	if (cell_it) {
		c       = cell_it->c;
		if (isspace(c)) {
			array_traverse_from(line->cells, cell_it, frame->cursor_col) {
				c = cell_it->c;
				cols += 1;
				if (!isspace(c)) {
					break;
				}
			}
		} else if (isalnum(c) || c == '_') {
			array_traverse_from(line->cells, cell_it, frame->cursor_col) {
				c = cell_it->c;
				cols += 1;
				if (!isalnum(c) && c != '_') {
					break;
				}
			}
			if ((isalnum(c) || c == '_')
			&&  (frame->cursor_col + cols == array_len(line->cells))) {
				cols += 1;
			} else if (isspace(c)) {
				array_traverse_from(line->cells, cell_it, frame->cursor_col + cols) {
					c = cell_it->c;
					cols += 1;
					if (!isspace(c)) {
						break;
					}
				}
			}
		} else {
			if (array_len(line->cells) >= frame->cursor_col) {
				cols += 1;
				cell_it = array_item(line->cells, frame->cursor_col - 1 + cols);
				c       = cell_it->c;
				if (isspace(c)) {
					array_traverse_from(line->cells, cell_it, frame->cursor_col + cols) {
						c = cell_it->c;
						cols += 1;
						if (!isspace(c)) {
							break;
						}
					}
				}
			}
		}
	}

    if ((frame->cursor_col + cols > array_len(line->cells))
	&&  (frame->cursor_line < bucket_array_len(frame->buffer->lines))) {
        yed_move_cursor_within_frame(frame, 0, 1);
        yed_set_cursor_within_frame(frame, 1, frame->cursor_line);
    } else {
        yed_move_cursor_within_frame(frame, cols, 0);
    }
}

void yed_default_command_cursor_prev_paragraph(int n_args, char **args) {
    yed_frame  *frame;
    yed_line   *line;
	int         i;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

	line = yed_buff_get_line(frame->buffer, frame->cursor_line);
	i    = 0;


	/* Move through lines in this paragraph. */
	if (line && line->visual_width != 0) {
		while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i - 1))
		&&      line->visual_width != 0) {
			i -= 1;
		}
	}

	/* Move through empty lines */
	while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i - 1))
	&&      line->visual_width == 0) {
		i -= 1;
	}

	if (line && line->visual_width != 0) {
		while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i - 1))
		&&      line->visual_width != 0) {
			i -= 1;
		}
	}

	yed_move_cursor_within_frame(frame, 0, i);
	frame->dirty = 1;
}

void yed_default_command_cursor_next_paragraph(int n_args, char **args) {
    yed_frame  *frame;
    yed_line   *line;
	int         i;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

	line = yed_buff_get_line(frame->buffer, frame->cursor_line);
	i    = 0;

	/* Move through lines in this paragraph. */
	if (line && line->visual_width != 0) {
		while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i + 1))
		&&      line->visual_width != 0) {
			i += 1;
		}
	}

	/* Move through empty lines */
	while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i + 1))
	&&      line->visual_width == 0) {
		i += 1;
	}

	i += 1;

	yed_move_cursor_within_frame(frame, 0, i);
	frame->dirty = 1;
}

void yed_default_command_buffer_new(int n_args, char **args) {
    int          buff_nr;
    yed_buffer **buffer_ptr, *buffer;

    yed_set_cursor(1, ys->term_rows);

    if (n_args > 2) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_add_new_buff();
    buffer_ptr = array_last(ys->buff_list);
    buffer     = *buffer_ptr;
    buff_nr    = array_len(ys->buff_list) - 1;

    if (n_args == 1) {
        yed_fill_buff_from_file(buffer, args[0]);
    }

    yed_append_text_to_cmd_buff("new buffer number is ");
    yed_append_int_to_cmd_buff(buff_nr);
}

void yed_default_command_frame(int n_args, char **args) {
    yed_frame *frame;
    int        created_new;

    created_new = 0;

    if (n_args == 0) {
        if (!ys->active_frame) {
            yed_append_text_to_cmd_buff("[!] no active frame");
            return;
        }
        frame = ys->active_frame;
    } else if (n_args == 1) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            created_new = 1;
        }
        frame = yed_get_or_add_frame(args[0]);
    } else if (n_args == 5) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            created_new = 1;
        }
        frame = yed_get_or_add_frame(args[0]);

        yed_clear_frame(frame);

        frame->top    = s_to_i(args[1]);
        frame->left   = s_to_i(args[2]);
        frame->height = s_to_i(args[3]);
        frame->width  = s_to_i(args[4]);

        if (!created_new) {
            yed_frame_reset_cursor(frame);
        }
    } else {
        yed_append_text_to_cmd_buff("[!] expected zero, one, or five arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (created_new) {
        yed_append_text_to_cmd_buff("(new frame '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("') ");
    }

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");
}

void yed_default_command_frames_list(int n_args, char **args) {
    const char *comma;
    tree_it(yed_frame_id_t, yed_frame_ptr_t) it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    comma = "";

    tree_traverse(ys->frames, it) {
        yed_append_text_to_cmd_buff((char*)comma);
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff((char*)tree_it_val(it)->id);
        yed_append_text_to_cmd_buff("'");
        comma = ", ";
    }
}

void yed_default_command_frame_set_buffer(int n_args, char **args) {
    yed_buffer *buffer, **buffer_ptr;
    int         buff_nr;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    buff_nr = s_to_i(args[0]);

    if (buff_nr >= array_len(ys->buff_list)) {
        yed_append_text_to_cmd_buff("[!] no buffer ");
        yed_append_int_to_cmd_buff(buff_nr);
        return;
    }

    buffer_ptr = array_item(ys->buff_list, buff_nr);
    buffer     = *buffer_ptr;

    yed_frame_set_buff(ys->active_frame, buffer);
    yed_clear_frame(ys->active_frame);
}

void yed_default_command_frame_new_file(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buffer, **buffer_ptr;
    int         new_file;
    FILE       *f_test;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    frame = yed_get_or_add_frame(args[0]);

    yed_add_new_buff();
    buffer_ptr = array_last(ys->buff_list);
    buffer     = *buffer_ptr;

    new_file = 0;
    f_test   = fopen(args[0], "r");

    if (f_test) {
        fclose(f_test);
        yed_fill_buff_from_file(buffer, args[0]);
    } else {
        buffer->path = args[0];
        new_file     = 1;
    }

    yed_frame_set_buff(frame, buffer);

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");

    if (new_file) {
        yed_append_text_to_cmd_buff(" (new file)");
    }
}

void yed_default_command_frame_split_new_file(int n_args, char **args) {
    yed_frame  *frame1,
               *frame2;
    yed_buffer *buffer, **buffer_ptr;
    int         new_file;
    FILE       *f_test;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame1 = ys->active_frame;
    frame2 = yed_get_or_add_frame(args[0]);

    if (frame1->width & 1) {
        frame1->width  >>= 1;
        frame2->width    = frame1->width + 1;
    } else {
        frame1->width  >>= 1;
        frame2->width    = frame1->width;
    }
    frame2->top       = frame1->top;
    frame2->left      = frame1->left + frame1->width;
    frame2->height    = frame1->height;
    frame2->cur_y     = frame2->top;
    frame2->cur_x     = frame2->left;
    frame2->desired_x = frame2->cur_x;

    yed_add_new_buff();
    buffer_ptr = array_last(ys->buff_list);
    buffer     = *buffer_ptr;

    new_file = 0;
    f_test   = fopen(args[0], "r");

    if (f_test) {
        fclose(f_test);
        yed_fill_buff_from_file(buffer, args[0]);
    } else {
        buffer->path = args[0];
        new_file     = 1;
    }

    yed_frame_set_buff(frame2, buffer);

    yed_clear_frame(frame1);
    yed_clear_frame(frame2);

    yed_activate_frame(frame2);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");

    if (new_file) {
        yed_append_text_to_cmd_buff(" (new file)");
    }
}

void yed_default_command_frame_next(int n_args, char **args) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    it = tree_lookup(ys->frames, frame->id);

    tree_it_next(it);

    if (!tree_it_good(it)) {
        it = tree_begin(ys->frames);
    }

    frame = tree_it_val(it);

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");
}

void yed_default_command_insert(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *new_line;
    int        key,
               col, idx,
               i, len;
    yed_cell  *cell_it;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    sscanf(args[0], "%d", &key);

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    col  = frame->cursor_col;

    if (key == ENTER) {
        /*
         * Must get 'line' _after_ we've added 'new_line' because
         * the array might have expanded and so the reference to
         * 'line' could be invalid if we had acquired it before.
         */
        new_line = yed_buff_insert_line(frame->buffer, frame->cursor_line + 1);
        line     = yed_buff_get_line(frame->buffer, frame->cursor_line);

        if (col == 1) {
            new_line->cells        = line->cells;
            line->cells            = array_make(yed_cell);
            new_line->visual_width = line->visual_width;
            line->visual_width     = 0;
        } else {
            len = 0;
            idx = yed_line_col_to_cell_idx(line, col);
            array_traverse_from(line->cells, cell_it, idx) {
                yed_line_append_cell(new_line, cell_it);
                len += 1;
            }
            for (; len > 0; len -= 1)    { yed_line_pop_cell(line); }
        }

        yed_set_cursor_within_frame(frame, 1, frame->cursor_line + 1);
    } else {
        if (key == TAB) {
            for (i = 0; i < 4; i += 1) {
                yed_insert_into_line(frame->buffer, frame->cursor_line, col + i, ' ');
            }
            yed_move_cursor_within_frame(frame, 4, 0);
        } else {
            yed_insert_into_line(frame->buffer, frame->cursor_line, col, key);
            yed_move_cursor_within_frame(frame, 1, 0);
        }
    }
}

void yed_default_command_delete_back(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *previous_line;
    int        col,
               buff_n_lines,
               prev_line_len, old_line_nr;
    yed_cell  *cell_it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    col  = frame->cursor_col;
    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    if (col == 1) {
        if (frame->cursor_line > 1) {
            old_line_nr   = frame->cursor_line;
            previous_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
            prev_line_len = previous_line->visual_width;

            if (prev_line_len == 0) {
                previous_line->cells        = line->cells;
                line->cells                 = array_make(yed_cell);
                previous_line->visual_width = line->visual_width;
                line->visual_width          = 0;
            } else {
                array_traverse(line->cells, cell_it) {
                    yed_line_append_cell(previous_line, cell_it);
                }
            }

            /*
             * Move to the previous line.
             */
            yed_set_cursor_within_frame(frame, prev_line_len + 1, frame->cursor_line - 1);

            /*
             * Kinda hacky, but this will help us pull the buffer
             * up if there is a buffer_y_offset and there's content
             * to pull up.
             */
            buff_n_lines = bucket_array_len(frame->buffer->lines);
            if (frame->buffer_y_offset >= buff_n_lines - frame->height) {
                yed_frame_reset_cursor(frame);
            }

            /*
             * Delete the old line.
             */
            yed_buff_delete_line(frame->buffer, old_line_nr);
        }
    } else {
        yed_delete_from_line(frame->buffer, frame->cursor_line, col - 1);
        yed_move_cursor_within_frame(frame, -1, 0);
    }

}

void yed_default_command_delete_line(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    int        row,
               n_lines;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    n_lines = bucket_array_len(frame->buffer->lines);
    row     = frame->cursor_line;

    if (n_lines > 1) {
        if (row == n_lines) {
            yed_move_cursor_within_frame(frame, 0, -1);
        }
        yed_buff_delete_line(frame->buffer, row);
    } else {
        ASSERT(row == 1, "only one line, but not on line one");

        line = yed_buff_get_line(frame->buffer, row);
        yed_set_cursor_within_frame(frame, 1, 1);
        yed_line_clear(line);
    }

    /*
     * Kinda hacky, but this will help us pull the buffer
     * up if there is a buffer_y_offset and there's content
     * to pull up.
     */
    n_lines = bucket_array_len(frame->buffer->lines);
    if (frame->buffer_y_offset >= n_lines - frame->height) {
        yed_frame_reset_cursor(frame);
    }
}

void yed_default_command_write_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    const char *path;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (n_args == 0) {
        if (buff->path == NULL) {
            yed_append_text_to_cmd_buff("[!] buffer is not associated with a file yet -- provide a file name");
            return;
        }
        path = buff->path;
    } else if (n_args == 1) {
        path = args[0];
    } else {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_write_buff_to_file(buff, path);

    yed_append_text_to_cmd_buff("wrote to '");
    yed_append_text_to_cmd_buff((char*)path);
    yed_append_text_to_cmd_buff("'");
}

void yed_default_command_plugin_load(int n_args, char **args) {
    int err;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    err = yed_load_plugin(args[0]);
    if (err == YED_PLUG_SUCCESS) {
        yed_append_text_to_cmd_buff("loaded plugin '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("'");
    } else {
        yed_append_text_to_cmd_buff("[!] ('");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("') -- ");

        switch (err) {
            case YED_PLUG_NOT_FOUND:
                yed_append_text_to_cmd_buff("could not find plugin");
                break;
            case YED_PLUG_NO_BOOT:
                yed_append_text_to_cmd_buff("could not find symbol 'yed_plugin_boot'");
                break;
            case YED_PLUG_BOOT_FAIL:
                yed_append_text_to_cmd_buff("'yed_plugin_boot' failed");
                break;
        }
    }
}

void yed_default_command_plugin_unload(int n_args, char **args) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    it = tree_lookup(ys->plugins, args[0]);

    if (!tree_it_good(it)) {
        yed_append_text_to_cmd_buff("[!] no plugin named '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("' is loaded");
        return;
    }

    yed_unload_plugin(args[0]);
    yed_append_text_to_cmd_buff("success");
}

void yed_default_command_plugins_list(int n_args, char **args) {
    const char *comma;
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    comma = "";

    tree_traverse(ys->plugins, it) {
        yed_append_text_to_cmd_buff((char*)comma);
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff((char*)tree_it_key(it));
        yed_append_text_to_cmd_buff("'");
        comma = ", ";
    }
}

void yed_default_command_plugins_add_dir(int n_args, char **args) {
    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

	yed_add_plugin_dir(args[0]);
}

void yed_default_command_plugins_list_dirs(int n_args, char **args) {
    char  *comma;
	char **dir_it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    comma = "";

	array_traverse(ys->plugin_dirs, dir_it) {
        yed_append_text_to_cmd_buff((char*)comma);
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(*dir_it);
        yed_append_text_to_cmd_buff("'");
        comma = ", ";
    }
}



int yed_execute_command(char *name, int n_args, char **args) {
    tree_it(yed_command_name_t, yed_command)  it;
    yed_command                               cmd;
    int                                       is_cmd_prompt;

    is_cmd_prompt = (strcmp(name, "command-prompt") == 0);

    if (!is_cmd_prompt) {
        yed_clear_cmd_buff();
    }

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it)) {
        yed_append_non_text_to_cmd_buff(TERM_RED);
        yed_append_text_to_cmd_buff("unknown command '");
        yed_append_text_to_cmd_buff(name);
        yed_append_text_to_cmd_buff("'");
        yed_append_non_text_to_cmd_buff(TERM_RESET);
        yed_append_non_text_to_cmd_buff(TERM_CURSOR_HIDE);

        return 1;
    }

    cmd = tree_it_val(it);

    if (!is_cmd_prompt) {
        yed_append_text_to_cmd_buff("(");
        yed_append_text_to_cmd_buff(name);
        yed_append_text_to_cmd_buff(") ");
    }

    cmd(n_args, args);

    yed_draw_command_line();

    return 0;
}

