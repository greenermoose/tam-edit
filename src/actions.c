#include "actions.h"
#include "buffer.h"
#include "clipboard.h"
#include "cursor.h"
#include "prompt.h"
#include "undo.h"

#include <tam.h>
#include <tam_uni.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_text_rows = 24;

/* -------------------------------------------------------------------------- */
/* Action Implementations                                                     */
/* -------------------------------------------------------------------------- */

static void act_cursor_up(editor_t *ed)    { cursor_move(ed, 0, -1, ed->mark); }
static void act_cursor_down(editor_t *ed)  { cursor_move(ed, 0, 1, ed->mark); }
static void act_cursor_left(editor_t *ed)  { cursor_move(ed, -1, 0, ed->mark); }
static void act_cursor_right(editor_t *ed) { cursor_move(ed, 1, 0, ed->mark); }

static void act_select_up(editor_t *ed)    { cursor_move(ed, 0, -1, true); }
static void act_select_down(editor_t *ed)  { cursor_move(ed, 0, 1, true); }
static void act_select_left(editor_t *ed)  { cursor_move(ed, -1, 0, true); }
static void act_select_right(editor_t *ed) { cursor_move(ed, 1, 0, true); }

static void act_word_left(editor_t *ed)        { cursor_word_left(ed, ed->mark); }
static void act_word_right(editor_t *ed)       { cursor_word_right(ed, ed->mark); }
static void act_select_word_left(editor_t *ed)  { cursor_word_left(ed, true); }
static void act_select_word_right(editor_t *ed) { cursor_word_right(ed, true); }

static void act_start_of_text_toggle(editor_t *ed) { cursor_home(ed, ed->mark); }
static void act_select_to_start_of_text(editor_t *ed) { cursor_home(ed, true); }
static void act_end_of_line(editor_t *ed) { cursor_end(ed, ed->mark); }
static void act_select_to_end_of_line(editor_t *ed) { cursor_end(ed, true); }

static void act_cursor_start(editor_t *ed) { cursor_start(ed, ed->mark); }
static void act_cursor_end(editor_t *ed)   { cursor_end_doc(ed, ed->mark); }
static void act_select_to_start(editor_t *ed) { cursor_start(ed, true); }
static void act_select_to_end(editor_t *ed)   { cursor_end_doc(ed, true); }

static void act_cursor_page_up(editor_t *ed)   { cursor_page(ed, -1, g_text_rows, ed->mark); }
static void act_cursor_page_down(editor_t *ed) { cursor_page(ed, 1, g_text_rows, ed->mark); }
static void act_select_page_up(editor_t *ed)   { cursor_page(ed, -1, g_text_rows, true); }
static void act_select_page_down(editor_t *ed) { cursor_page(ed, 1, g_text_rows, true); }

static void act_select_all(editor_t *ed) { cursor_select_all(ed); }
static void act_deselect(editor_t *ed)   { ed->mark = false; cursor_deselect(ed); }

static void act_insert_newline(editor_t *ed) { ed->mark = false; editor_insert_newline(ed); }
static void act_insert_tab(editor_t *ed)     { ed->mark = false; editor_insert_tab(ed); }
static void act_backspace(editor_t *ed)      { ed->mark = false; editor_backspace(ed); }
static void act_delete(editor_t *ed)         { ed->mark = false; editor_delete(ed); }

static void act_delete_word_left(editor_t *ed) { ed->mark = false; editor_delete_word_left(ed); }

static bool selection_has_text(selection_t sel)
{
    return sel.active && (sel.anchor.line != sel.cursor.line ||
                          sel.anchor.col != sel.cursor.col);
}

static bool clipboard_from_buffer(editor_t *ed, bool *selected)
{
    tam_str_t joined = tam_str_new("");
    if (!joined) return false;
    const char *newline = ed->crlf ? "\r\n" : "\n";
    bool ok = true;
    *selected = false;
    editor_t cursor = *ed;
    for (size_t i = 0; i <= ed->extra_count; i++) {
        if (i > 0) cursor.sel = ed->extra_cursors[i - 1].sel;
        if (!selection_has_text(cursor.sel)) continue;
        size_t len;
        char *text = selection_get_text(&cursor, &len);
        tam_str_t next = *selected ? tam_str_cat(joined, newline) : joined;
        if (next) joined = next;
        ok = text && next;
        if (ok) {
            next = tam_str_cat_len(joined, text, len);
            if (next) joined = next;
            ok = next != NULL;
        }
        free(text);
        if (!ok) break;
        *selected = true;
    }
    if (ok && !*selected && ed->cx_line < ed->doc.count) {
        tam_str_t next = tam_str_cat_len(joined, ed->doc.lines[ed->cx_line],
                                         ed->doc.lens[ed->cx_line]);
        if (next) {
            joined = next;
            next = tam_str_cat(joined, newline);
            if (next) joined = next;
        }
        ok = next != NULL;
    }
    if (ok) ok = clipboard_set(joined, tam_str_len(joined), ed->tty_out_fd);
    tam_str_free(joined);
    return ok;
}

static void act_copy(editor_t *ed)
{
    bool selected;
    if (clipboard_from_buffer(ed, &selected))
        editor_set_message(ed, selected ? "Copied selection" : "Copied line");
}

static void act_cut(editor_t *ed)
{
    if (ed->active_view && ed->active_view->buf && ed->active_view->buf->readonly) {
        editor_set_message(ed, "Buffer is read-only");
        return;
    }

    bool selected;
    if (!clipboard_from_buffer(ed, &selected)) return;
    ed->mark = false;
    if (selected) {
        editor_delete_selections(ed);
    } else {
        cursor_deselect(ed);
        editor_delete_line(ed);
    }
    editor_set_message(ed, selected ? "Cut selection" : "Cut line");
}

static void act_paste(editor_t *ed)
{
    size_t len = 0;
    char *text = clipboard_get(&len);
    if (text != NULL && len > 0) {
        ed->mark = false;
        editor_insert_text(ed, text, len);
        free(text);
        editor_set_message(ed, "Pasted");
    } else {
        free(text);
    }
}

static void act_duplicate_line(editor_t *ed) { editor_duplicate(ed); }
static void act_delete_line(editor_t *ed)    { editor_delete_line(ed); }
static void act_move_lines_up(editor_t *ed)  { editor_move_lines(ed, -1); }
static void act_move_lines_down(editor_t *ed){ editor_move_lines(ed, 1); }

static void act_undo(editor_t *ed) { ed->mark = false; editor_undo(ed); }
static void act_redo(editor_t *ed) { ed->mark = false; editor_redo(ed); }

static void act_save(editor_t *ed)
{
    if (ed->path == NULL || ed->path[0] == '\0') {
        prompt_open_save_as(ed);
    } else {
        char err[192] = "";
        editor_save_configured(ed, err, sizeof(err));
    }
}

static void act_save_as(editor_t *ed)
{
    prompt_open_save_as(ed);
}

static void act_open_file(editor_t *ed)
{
    prompt_open_command_prefill(ed, "open ");
}

static void act_quit(editor_t *ed)
{
    bool dirty = ed->dirty;
    for (size_t i = 0; i < ed->tabs.count; i++) dirty |= tab_is_dirty(ed->tabs.tabs[i]);
    if (dirty) {
        if (!ed->quit_armed) {
            editor_set_message(ed,
                "Unsaved changes: Ctrl+Q again to quit, Ctrl+S to save");
            ed->quit_armed = true;
            return;
        }
    }
    ed->running = false;
}

static void act_quit_all(editor_t *ed)
{
    bool any_dirty = false;
    for (size_t i = 0; i < ed->tabs.count; i++) {
        if (tab_is_dirty(ed->tabs.tabs[i])) any_dirty = true;
    }
    if (any_dirty || ed->dirty) {
        prompt_open_quit_confirm(ed);
        return;
    }
    ed->running = false;
}

static void act_command_mode(editor_t *ed) { prompt_open_palette(ed); }
static void act_goto(editor_t *ed)         { prompt_open_goto(ed); }
static void act_find(editor_t *ed)         { prompt_open_find(ed); }
static void act_find_literal(editor_t *ed) { prompt_open_find_literal(ed); }
static void act_find_next(editor_t *ed)    { search_find_next(ed); }
static void act_find_prev(editor_t *ed)    { search_find_prev(ed); }
static void act_run_shell(editor_t *ed)    { prompt_open_shell(ed); }
static void act_shell_mode(editor_t *ed)
{
    if (!ed->tty) {
        prompt_open_shell(ed);
        return;
    }
    int rc = shell_run_interactive(ed->tty, NULL, false, true);
    editor_set_message(ed, "Shell exited (%d)", rc);
}
static bool is_help_view(const view_t *v)
{
    return v != NULL && v->buf != NULL && v->buf->scratch &&
           v->buf->path != NULL && strncmp(v->buf->path, "help:", 5) == 0;
}

static bool close_help_view(editor_t *ed, view_t *v)
{
    if (!ed || !is_help_view(v) || ed->tabs.count == 0) return false;
    tab_t *cur_tab = ed->tabs.tabs[ed->tabs.active];
    pane_node_t *node = pane_find_view(cur_tab->root, v);
    if (node == NULL) return false;
    if (node != cur_tab->root) {
        if (v == ed->active_view) {
            view_t *next_v = pane_next_view(cur_tab->root, v);
            if (next_v != NULL && next_v != v) {
                editor_set_active_view(ed, next_v);
            } else {
                ed->active_view = NULL;
            }
        }
        pane_close(cur_tab->root, node, &cur_tab->root);
        return true;
    }
    if (ed->tabs.count > 1) {
        return editor_close_tab(ed);
    }
    return false;
}

static view_t *find_help_view_in_node(pane_node_t *n)
{
    if (!n) return NULL;
    if (n->view != NULL) {
        return is_help_view(n->view) ? n->view : NULL;
    }
    view_t *c1 = find_help_view_in_node(n->child1);
    return c1 ? c1 : find_help_view_in_node(n->child2);
}

static void act_toggle_help(editor_t *ed)
{
    if (is_help_view(ed->active_view)) {
        if (close_help_view(ed, ed->active_view)) return;
    }
    if (ed->tabs.count > 0 && ed->tabs.active < ed->tabs.count) {
        view_t *hv = find_help_view_in_node(ed->tabs.tabs[ed->tabs.active]->root);
        if (hv != NULL && close_help_view(ed, hv)) return;
    }
    prompt_execute_command(ed, "help");
}
static void act_toggle_key_menu(editor_t *ed) { actions_open_keybindings_help(ed); }
static void act_toggle_overwrite(editor_t *ed)
{
    ed->config.overwrite = !ed->config.overwrite;
    editor_set_message(ed, "Overwrite mode %s", ed->config.overwrite ? "on" : "off");
}

static void act_spawn_cursor(editor_t *ed)        { editor_spawn_cursor_match(ed, false); }
static void act_skip_cursor(editor_t *ed)         { editor_spawn_cursor_match(ed, true); }
static void act_spawn_cursor_up(editor_t *ed)     { editor_spawn_cursor_dir(ed, -1); }
static void act_spawn_cursor_down(editor_t *ed)   { editor_spawn_cursor_dir(ed, 1); }
static void act_spawn_cursor_select(editor_t *ed) { editor_spawn_cursors_on_selection(ed); }
static void act_remove_cursor(editor_t *ed)       { editor_remove_last_cursor(ed); }
static void act_remove_all_cursors(editor_t *ed)  { editor_remove_all_cursors(ed); }

/* Milestone 2 Action implementations */
static void act_vsplit(editor_t *ed)          { editor_vsplit(ed, NULL); }
static void act_hsplit(editor_t *ed)          { editor_hsplit(ed, NULL); }
static void act_next_split(editor_t *ed)      { editor_next_split(ed); }
static void act_prev_split(editor_t *ed)      { editor_prev_split(ed); }
static void act_unsplit(editor_t *ed)         { editor_unsplit(ed); }
static void act_add_tab(editor_t *ed)         { editor_add_tab(ed, NULL); }
static void act_next_tab(editor_t *ed)        { editor_next_tab(ed); }
static void act_prev_tab(editor_t *ed)        { editor_prev_tab(ed); }
static void act_first_tab(editor_t *ed)
{
    if (ed->tabs.count > 0) {
        ed->tabs.active = 0;
        editor_set_active_view(ed, ed->tabs.tabs[0]->active_view);
    }
}
static void act_last_tab(editor_t *ed)
{
    if (ed->tabs.count > 0) {
        ed->tabs.active = ed->tabs.count - 1;
        editor_set_active_view(ed, ed->tabs.tabs[ed->tabs.count - 1]->active_view);
    }
}
static void act_toggle_softwrap(editor_t *ed)
{
    if (ed->active_view) {
        ed->active_view->softwrap = !ed->active_view->softwrap;
        editor_set_message(ed, "Softwrap toggled %s", ed->active_view->softwrap ? "on" : "off");
    }
}
static void act_toggle_ruler(editor_t *ed)
{
    ed->config.ruler = !ed->config.ruler;
    editor_set_message(ed, "Ruler toggled %s", ed->config.ruler ? "on" : "off");
}

static void act_close(editor_t *ed) { editor_close_current(ed, false); }
static void act_update(editor_t *ed)
{
    if (!ed->active_view || ed->active_view->buf->scratch || !ed->path || !*ed->path) return;
    if (ed->dirty) prompt_open_confirm_reload(ed, ed->path, true);
    else editor_reopen(ed, NULL);
}
static void act_replace(editor_t *ed)
{
    if (ed->active_view && !ed->active_view->buf->readonly)
        prompt_open_command_prefill(ed, "replace ");
}
static void act_redraw(editor_t *ed) { ed->force_redraw = true; }
static void act_prompt(editor_t *ed) { prompt_open_command(ed); }
static void act_delete_word_right(editor_t *ed) { ed->mark = false; editor_delete_word_right(ed); }
static void act_line_start(editor_t *ed) { cursor_line_start(ed, ed->mark); }
static void act_select_line_start(editor_t *ed) { cursor_line_start(ed, true); }
static void act_toggle_mark(editor_t *ed)
{
    ed->mark = !ed->mark;
    cursor_deselect(ed);
    editor_set_message(ed, ed->mark ? "Mark set" : "Mark cleared");
}
static void act_cancel(editor_t *ed)
{
    ed->mark = false;
    if (close_help_view(ed, ed->active_view)) return;
    if (ed->extra_count) editor_remove_all_cursors(ed);
    cursor_deselect(ed);
}
static void act_kill_line(editor_t *ed)
{
    if (ed->active_view && ed->active_view->buf->readonly) return;
    ed->mark = false;
    cursor_deselect(ed);
    editor_delete_line(ed);
}

/* -------------------------------------------------------------------------- */
/* Action Registry Table                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    const char *name;
    const char *legacy;
    action_fn   func;
} action_def_t;

static const action_def_t g_actions[] = {
    {"line-up", "CursorUp", act_cursor_up},
    {"line-down", "CursorDown", act_cursor_down},
    {"char-left", "CursorLeft", act_cursor_left},
    {"char-right", "CursorRight", act_cursor_right},
    {"select-up", "SelectUp", act_select_up},
    {"select-down", "SelectDown", act_select_down},
    {"select-char-left", "SelectLeft", act_select_left},
    {"select-char-right", "SelectRight", act_select_right},
    {"word-left", "WordLeft", act_word_left},
    {"word-right", "WordRight", act_word_right},
    {"select-word-left", "SelectWordLeft", act_select_word_left},
    {"select-word-right", "SelectWordRight", act_select_word_right},
    {"smart-home", "StartOfTextToggle", act_start_of_text_toggle},
    {"select-smart-home", "SelectToStartOfTextToggle", act_select_to_start_of_text},
    {"line-end", "EndOfLine", act_end_of_line},
    {"select-line-end", "SelectToEndOfLine", act_select_to_end_of_line},
    {"buffer-start", "CursorStart", act_cursor_start},
    {"buffer-end", "CursorEnd", act_cursor_end},
    {"select-buffer-start", "SelectToStart", act_select_to_start},
    {"select-buffer-end", "SelectToEnd", act_select_to_end},
    {"page-up", "CursorPageUp", act_cursor_page_up},
    {"page-down", "CursorPageDown", act_cursor_page_down},
    {"select-page-up", "SelectPageUp", act_select_page_up},
    {"select-page-down", "SelectPageDown", act_select_page_down},
    {"select-all", "SelectAll", act_select_all},
    {"deselect", "Deselect", act_deselect},
    {"insert-newline", "InsertNewline", act_insert_newline},
    {"insert-tab", "InsertTab", act_insert_tab},
    {"delete-char-left", "Backspace", act_backspace},
    {"delete-char-right", "Delete", act_delete},
    {"delete-word-left", "DeleteWordLeft", act_delete_word_left},
    {"copy", "Copy", act_copy},
    {NULL, "CopyLine", act_copy},
    {"cut", "Cut", act_cut},
    {NULL, "CutLine", act_cut},
    {"paste", "Paste", act_paste},
    {"delete-line", "DeleteLine", act_delete_line},
    {"move-lines-up", "MoveLinesUp", act_move_lines_up},
    {"move-lines-down", "MoveLinesDown", act_move_lines_down},
    {"duplicate-line", "Duplicate", act_duplicate_line},
    {NULL, "DuplicateLine", act_duplicate_line},
    {"undo", "Undo", act_undo},
    {"redo", "Redo", act_redo},
    {"save", "Save", act_save},
    {"save-as", "SaveAs", act_save_as},
    {"open", "OpenFile", act_open_file},
    {"quit", "Quit", act_quit},
    {"quit-all", "QuitAll", act_quit_all},
    {"execute", "CommandMode", act_command_mode},
    {"goto-line", "Goto", act_goto},
    {"find", "Find", act_find},
    {"find-literal", "FindLiteral", act_find_literal},
    {"find-next", "FindNext", act_find_next},
    {"find-prev", "FindPrevious", act_find_prev},
    {"shell-mode", "ShellMode", act_shell_mode},
    {"run-shell", "RunShell", act_run_shell},
    {"toggle-help", "ToggleHelp", act_toggle_help},
    {"guidance", "ToggleKeyMenu", act_toggle_key_menu},
    {"toggle-overwrite", "ToggleOverwriteMode", act_toggle_overwrite},

    /* Multi-cursor Actions */
    {"spawn-multi-cursor", "SpawnMultiCursor", act_spawn_cursor},
    {"skip-multi-cursor", "SkipMultiCursor", act_skip_cursor},
    {"spawn-multi-cursor-up", "SpawnMultiCursorUp", act_spawn_cursor_up},
    {"spawn-multi-cursor-down", "SpawnMultiCursorDown", act_spawn_cursor_down},
    {"spawn-multi-cursor-select", "SpawnMultiCursorSelect", act_spawn_cursor_select},
    {"remove-multi-cursor", "RemoveMultiCursor", act_remove_cursor},
    {"remove-all-multi-cursors", "RemoveAllMultiCursors", act_remove_all_cursors},

    /* Milestone 2 Actions */
    {"vsplit", "VSplit", act_vsplit},
    {"hsplit", "HSplit", act_hsplit},
    {"next-split", "NextSplit", act_next_split},
    {"prev-split", "PreviousSplit", act_prev_split},
    {"first-split", "FirstSplit", act_next_split},
    {"last-split", "LastSplit", act_prev_split},
    {"unsplit", "Unsplit", act_unsplit},
    {"new", "AddTab", act_add_tab},
    {"next-tab", "NextTab", act_next_tab},
    {"prev-tab", "PreviousTab", act_prev_tab},
    {"first-tab", "FirstTab", act_first_tab},
    {"last-tab", "LastTab", act_last_tab},
    {"toggle-softwrap", "ToggleSoftwrap", act_toggle_softwrap},
    {"toggle-ruler", "ToggleRuler", act_toggle_ruler},
    {"close", NULL, act_close},
    {"update", NULL, act_update},
    {"replace", NULL, act_replace},
    {"clear-screen", NULL, act_redraw},
    {"command-prompt", NULL, act_prompt},
    {"delete-word-right", NULL, act_delete_word_right},
    {"line-start", NULL, act_line_start},
    {"select-line-start", NULL, act_select_line_start},
    {"toggle-mark", NULL, act_toggle_mark},
    {"cancel-mode", NULL, act_cancel},
    {"kill-line", NULL, act_kill_line},
    {NULL, NULL, NULL}
};

const char *actions_name_at(size_t index)
{
    if (index >= sizeof(g_actions) / sizeof(g_actions[0]) - 1) return NULL;
    return g_actions[index].name ? g_actions[index].name : "";
}

void actions_init(void) {}
void actions_free(void) {}

bool actions_execute(editor_t *ed, const char *name)
{
    if (name == NULL || *name == '\0') return false;

    if (strncmp(name, "command-edit:", 13) == 0) {
        prompt_open_command_prefill(ed, name + 13);
        return true;
    }

    if (strncmp(name, "command:", 8) == 0) {
        const char *cmd = name + 8;
        if (strcmp(cmd, "togglelocal softwrap") == 0 ||
            strcmp(cmd, "toggle softwrap") == 0) {
            act_toggle_softwrap(ed);
            return true;
        }
        prompt_execute_command(ed, cmd);
        return true;
    }

    if (strcmp(name, "togglelocal softwrap") == 0 ||
        strcmp(name, "toggle softwrap") == 0) {
        act_toggle_softwrap(ed);
        return true;
    }

    /* Handle multi-action pipeline 'Action1|Action2' */
    const char *pipe = strchr(name, '|');
    if (pipe != NULL) {
        char first[64];
        size_t n = (size_t)(pipe - name);
        if (n >= sizeof(first)) n = sizeof(first) - 1;
        memcpy(first, name, n);
        first[n] = '\0';
        if (actions_execute(ed, first)) return true;
        return actions_execute(ed, pipe + 1);
    }

    if (strcmp(name, "CloseHelp") == 0) {
        return close_help_view(ed, ed->active_view);
    }

    if (strcmp(name, "RemoveAllMultiCursors") == 0) {
        if (ed->extra_count == 0) return false;
        editor_remove_all_cursors(ed);
        return true;
    }

    for (size_t i = 0; g_actions[i].func != NULL; i++) {
        if (((g_actions[i].name && strcmp(g_actions[i].name, name) == 0) ||
            (g_actions[i].legacy && strcmp(g_actions[i].legacy, name) == 0))) {
            g_actions[i].func(ed);
            return true;
        }
    }
    return false;
}

/* -------------------------------------------------------------------------- */
/* Keybinding Lookup Table                                                    */
/* -------------------------------------------------------------------------- */

typedef struct {
    tam_key_type_t type;
    uint32_t       codepoint;
    unsigned       mods;
    const char    *action;
} keybind_entry_t;

static const keybind_entry_t g_keybindings[] = {
    /* Arrow navigation */
    {TAM_KEY_UP, 0, 0, "line-up"},
    {TAM_KEY_DOWN, 0, 0, "line-down"},
    {TAM_KEY_LEFT, 0, 0, "char-left"},
    {TAM_KEY_RIGHT, 0, 0, "char-right"},
    {TAM_KEY_UP, 0, TAM_MOD_SHIFT, "select-up"},
    {TAM_KEY_DOWN, 0, TAM_MOD_SHIFT, "select-down"},
    {TAM_KEY_LEFT, 0, TAM_MOD_SHIFT, "select-char-left"},
    {TAM_KEY_RIGHT, 0, TAM_MOD_SHIFT, "select-char-right"},
    {TAM_KEY_LEFT, 0, TAM_MOD_CTRL, "word-left"},
    {TAM_KEY_RIGHT, 0, TAM_MOD_CTRL, "word-right"},
    {TAM_KEY_LEFT, 0, TAM_MOD_CTRL | TAM_MOD_SHIFT, "select-word-left"},
    {TAM_KEY_RIGHT, 0, TAM_MOD_CTRL | TAM_MOD_SHIFT, "select-word-right"},

    /* Home / End / Page */
    {TAM_KEY_HOME, 0, 0, "line-start"},
    {TAM_KEY_END, 0, 0, "line-end"},
    {TAM_KEY_HOME, 0, TAM_MOD_SHIFT, "select-line-start"},
    {TAM_KEY_END, 0, TAM_MOD_SHIFT, "select-line-end"},
    {TAM_KEY_HOME, 0, TAM_MOD_CTRL, "buffer-start"},
    {TAM_KEY_END, 0, TAM_MOD_CTRL, "buffer-end"},
    {TAM_KEY_HOME, 0, TAM_MOD_CTRL | TAM_MOD_SHIFT, "select-buffer-start"},
    {TAM_KEY_END, 0, TAM_MOD_CTRL | TAM_MOD_SHIFT, "select-buffer-end"},
    {TAM_KEY_PAGE_UP, 0, 0, "page-up"},
    {TAM_KEY_PAGE_DOWN, 0, 0, "page-down"},
    {TAM_KEY_PAGE_UP, 0, TAM_MOD_SHIFT, "select-page-up"},
    {TAM_KEY_PAGE_DOWN, 0, TAM_MOD_SHIFT, "select-page-down"},

    /* Tab navigation with Ctrl-PageUp/Down */
    {TAM_KEY_PAGE_UP, 0, TAM_MOD_CTRL, "prev-tab"},
    {TAM_KEY_PAGE_DOWN, 0, TAM_MOD_CTRL, "next-tab"},

    /* Basic edits */
    {TAM_KEY_ENTER, 0, 0, "insert-newline"},
    {TAM_KEY_TAB, 0, 0, "insert-tab"},
    {TAM_KEY_BACKSPACE, 0, 0, "delete-char-left"},
    {TAM_KEY_DELETE, 0, 0, "delete-char-right"},
    {TAM_KEY_INSERT, 0, 0, "toggle-overwrite"},
    {TAM_KEY_BACKSPACE, 0, TAM_MOD_CTRL, "delete-word-left"},
    {TAM_KEY_BACKSPACE, 0, TAM_MOD_ALT, "delete-word-left"},

    /* Common Micro Ctrl keys */
    {TAM_KEY_CHAR, 's', TAM_MOD_CTRL, "save"},
    {TAM_KEY_CHAR, 'o', TAM_MOD_CTRL, "open"},
    {TAM_KEY_CHAR, 'q', TAM_MOD_CTRL, "quit"},
    {TAM_KEY_CHAR, 'z', TAM_MOD_CTRL, "undo"},
    {TAM_KEY_CHAR, 'y', TAM_MOD_CTRL, "redo"},
    {TAM_KEY_CHAR, 'c', TAM_MOD_CTRL, "copy"},
    {TAM_KEY_CHAR, 'x', TAM_MOD_CTRL, "cut"},
    {TAM_KEY_CHAR, 'v', TAM_MOD_CTRL, "paste"},
    {TAM_KEY_CHAR, 'a', TAM_MOD_CTRL, "select-all"},
    {TAM_KEY_CHAR, 'd', TAM_MOD_CTRL, "close"},
    {TAM_KEY_CHAR, 'e', TAM_MOD_CTRL, "execute"},
    {TAM_KEY_CHAR, 'l', TAM_MOD_CTRL, "clear-screen"},
    {TAM_KEY_CHAR, 'f', TAM_MOD_CTRL, "find"},
    {TAM_KEY_CHAR, 'n', TAM_MOD_CTRL, "new"},
    {TAM_KEY_CHAR, 'k', TAM_MOD_CTRL, "kill-line"},
    {TAM_KEY_CHAR, 'r', TAM_MOD_CTRL, "replace"},

    {TAM_KEY_CHAR, 'u', TAM_MOD_CTRL, "update"},
    {TAM_KEY_CHAR, 'g', TAM_MOD_CTRL, "guidance"},
    {TAM_KEY_CHAR, '<', TAM_MOD_ALT, "buffer-start"},
    {TAM_KEY_CHAR, '>', TAM_MOD_ALT, "buffer-end"},
    {TAM_KEY_CHAR, 'd', TAM_MOD_ALT, "delete-word-right"},
    {TAM_KEY_CHAR, 'e', TAM_MOD_ALT, "command-prompt"},
    {TAM_KEY_CHAR, 'j', TAM_MOD_ALT, "find-next"},
    {TAM_KEY_CHAR, 'J', TAM_MOD_ALT, "find-prev"},
    {TAM_KEY_CHAR, 't', TAM_MOD_ALT, "next-tab"},
    {TAM_KEY_CHAR, 'T', TAM_MOD_ALT, "prev-tab"},
    {TAM_KEY_DELETE, 0, TAM_MOD_CTRL, "delete-word-right"},
    {TAM_KEY_ENTER, 0, TAM_MOD_ALT, "insert-newline"},

    /* Secondary editor bindings */
    {TAM_KEY_CHAR, 'f', TAM_MOD_ALT, "find-literal"},
    {TAM_KEY_CHAR, 'w', TAM_MOD_ALT, "toggle-softwrap"},
    {TAM_KEY_CHAR, 'm', TAM_MOD_ALT, "toggle-mark"},
    {TAM_KEY_CHAR, '.', TAM_MOD_ALT, "word-right"},
    {TAM_KEY_CHAR, ',', TAM_MOD_ALT, "word-left"},
    {TAM_KEY_CHAR, 'n', TAM_MOD_ALT, "spawn-multi-cursor"},
    {TAM_KEY_CHAR, 'M', TAM_MOD_ALT, "spawn-multi-cursor-select"},
    {TAM_KEY_CHAR, 'x', TAM_MOD_ALT, "skip-multi-cursor"},
    {TAM_KEY_CHAR, 'p', TAM_MOD_ALT, "remove-multi-cursor"},
    {TAM_KEY_CHAR, 'c', TAM_MOD_ALT, "remove-all-multi-cursors"},
    {TAM_KEY_UP, 0, TAM_MOD_ALT | TAM_MOD_SHIFT, "spawn-multi-cursor-up"},
    {TAM_KEY_DOWN, 0, TAM_MOD_ALT | TAM_MOD_SHIFT, "spawn-multi-cursor-down"},
    {TAM_KEY_UP, 0, TAM_MOD_ALT, "move-lines-up"},
    {TAM_KEY_DOWN, 0, TAM_MOD_ALT, "move-lines-down"},

    /* Function keys */
    {TAM_KEY_F1, 0, 0, "guidance"},
    {TAM_KEY_F2, 0, 0, "save"},
    {TAM_KEY_F3, 0, 0, "find-next"},
    {TAM_KEY_F4, 0, 0, "quit"},
    {TAM_KEY_F7, 0, 0, "find"},
    {TAM_KEY_F10, 0, 0, "quit"},

    /* Esc */
    {TAM_KEY_ESC, 0, 0, "cancel-mode"},

    {TAM_KEY_NONE, 0, 0, NULL}
};

const char *actions_lookup_key_description(const editor_t *ed, const tam_key_event_t *ev)
{
    if (!ed || !ev) return NULL;
    for (custom_keybind_t *ckb = ed->keybindings; ckb != NULL; ckb = ckb->next) {
        if (ckb->type == ev->type && ckb->mods == ev->mods) {
            if (ckb->type == TAM_KEY_CHAR && ckb->codepoint != 0) {
                if (ckb->codepoint != ev->codepoint) {
                    continue;
                }
            }
            return ckb->action;
        }
    }
    for (size_t i = 0; g_keybindings[i].action != NULL; i++) {
        const keybind_entry_t *kb = &g_keybindings[i];
        if (kb->type == ev->type && kb->mods == ev->mods) {
            if (kb->type == TAM_KEY_CHAR && kb->codepoint != 0) {
                if (kb->codepoint != ev->codepoint) {
                    continue;
                }
            }
            return kb->action;
        }
    }
    return NULL;
}

void actions_open_keybindings_help(editor_t *ed)
{
    if (!ed) return;
    if (is_help_view(ed->active_view) && strcmp(ed->path, "help:keybindings") == 0 &&
        close_help_view(ed, ed->active_view)) return;
    tam_str_t out = tam_str_new("# tam-edit Keybindings\n\n");
    if (!out) return;

    if (ed->keybindings != NULL) {
        out = tam_str_cat(out, "## Custom Bindings (settings.ini)\n\n");
        for (custom_keybind_t *ckb = ed->keybindings; ckb != NULL; ckb = ckb->next) {
            char kbuf[64];
            keybind_format_string(ckb->type, ckb->codepoint, ckb->mods, kbuf, sizeof(kbuf));
            char line[256];
            snprintf(line, sizeof(line), "  %-20s -> %s\n", kbuf, ckb->action ? ckb->action : "");
            out = tam_str_cat(out, line);
        }
        out = tam_str_cat(out, "\n");
    }

    out = tam_str_cat(out, "## Default Keybindings\n\n");
    for (size_t i = 0; g_keybindings[i].action != NULL; i++) {
        tam_key_event_t key = {g_keybindings[i].type, g_keybindings[i].codepoint,
                               g_keybindings[i].mods};
        if (actions_lookup_key_description(ed, &key) != g_keybindings[i].action) continue;
        char kbuf[64];
        keybind_format_string(g_keybindings[i].type, g_keybindings[i].codepoint,
                              g_keybindings[i].mods, kbuf, sizeof(kbuf));
        char line[256];
        snprintf(line, sizeof(line), "  %-20s -> %s\n", kbuf, g_keybindings[i].action);
        out = tam_str_cat(out, line);
    }

    out = tam_str_cat(out,
        "\n## Mouse Bindings\n\n"
        "  LeftClick            -> Move cursor\n"
        "  LeftDrag             -> Select text\n"
        "  WheelUp / WheelDown  -> Scroll view\n"
        "  Ctrl+LeftClick       -> Toggle multi-cursor\n");

    editor_open_scratch(ed, "help:keybindings", out, true);
    tam_str_free(out);
}

bool actions_handle_key(editor_t *ed, const tam_key_event_t *ev, int text_rows)
{
    g_text_rows = text_rows > 0 ? text_rows : 24;

    /* Reset quit confirmation on any key other than Ctrl+Q */
    const char *binding = actions_lookup_key_description(ed, ev);
    if (!binding || (strcmp(binding, "quit") != 0 && strcmp(binding, "Quit") != 0)) {
        ed->quit_armed = false;
    }

    /* 1. Look up in custom user keybindings first */
    for (custom_keybind_t *ckb = ed->keybindings; ckb != NULL; ckb = ckb->next) {
        if (ckb->type == ev->type && ckb->mods == ev->mods) {
            if (ckb->type == TAM_KEY_CHAR && ckb->codepoint != 0) {
                if (ckb->codepoint != ev->codepoint) {
                    continue;
                }
            }
            if (actions_execute(ed, ckb->action)) {
                return true;
            }
        }
    }

    /* 2. Look up in default keybindings table */
    for (size_t i = 0; g_keybindings[i].action != NULL; i++) {
        const keybind_entry_t *kb = &g_keybindings[i];
        if (kb->type == ev->type && kb->mods == ev->mods) {
            if (kb->type == TAM_KEY_CHAR && kb->codepoint != 0) {
                if (kb->codepoint != ev->codepoint) {
                    continue;
                }
            }
            if (actions_execute(ed, kb->action)) {
                return true;
            }
        }
    }

    /* 3. Regular character insertion */
    if (ev->type == TAM_KEY_CHAR &&
        (ev->mods == 0 || ev->mods == TAM_MOD_SHIFT) &&
        ev->codepoint >= 0x20 && ev->codepoint != 0x7F) {
        ed->mark = false;
        editor_insert_codepoint(ed, ev->codepoint);
        return true;
    }

    return false;
}
