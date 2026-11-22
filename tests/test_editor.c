#include "editor.h"
#include "cursor.h"
#include "render.h"
#include "syntax.h"

#include <tam.h>
#include <tam_uni.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "  [FAIL] %s:%d: %s\n", __FILE__, __LINE__,      \
                    #cond);                                                  \
            abort();                                                         \
        }                                                                    \
    } while (0)

static char *test_path(const char *name)
{
    static char buf[512];
    const char *dir = getenv("TMPDIR");
    if (dir == NULL || dir[0] == '\0') dir = "/tmp";
    snprintf(buf, sizeof(buf), "%s/tam-edit-test-%s-%ld", dir, name,
             (long)getpid());
    return buf;
}

static void write_file(const char *path, const char *data)
{
    CHECK(tam_fs_write_atomic(path, data, strlen(data), 0600));
}

static void check_contents(editor_t *ed, const char *expected)
{
    tam_str_t text = tam_str_new("");
    CHECK(text != NULL);
    for (size_t i = 0; i < ed->doc.count; i++) {
        if (i > 0) text = tam_str_cat(text, ed->crlf ? "\r\n" : "\n");
        text = tam_str_cat_len(text, ed->doc.lines[i], ed->doc.lens[i]);
    }
    CHECK(strcmp(text, expected) == 0);
    tam_str_free(text);
}

static void test_load_split(void)
{
    const char *path = test_path("split");
    write_file(path, "one\ntwo\n");

    editor_t ed;
    editor_init(&ed);
    CHECK(editor_load(&ed, path));
    CHECK(ed.doc.count == 3);
    CHECK(strcmp(ed.doc.lines[0], "one") == 0);
    CHECK(strcmp(ed.doc.lines[1], "two") == 0);
    CHECK(ed.doc.lens[2] == 0);
    CHECK(!ed.dirty);
    check_contents(&ed, "one\ntwo\n");

    CHECK(editor_save(&ed));
    size_t len = 0;
    char *data = tam_fs_read_file(path, &len);
    CHECK(data != NULL && len == 8 && memcmp(data, "one\ntwo\n", 8) == 0);
    free(data);

    editor_free(&ed);
    unlink(path);
    printf("  [PASS] test_load_split\n");
}

static void test_no_trailing_newline(void)
{
    const char *path = test_path("noeol");
    write_file(path, "abc");

    editor_t ed;
    editor_init(&ed);
    CHECK(editor_load(&ed, path));
    CHECK(ed.doc.count == 1);
    CHECK(editor_save(&ed));

    size_t len = 0;
    char *data = tam_fs_read_file(path, &len);
    CHECK(data != NULL && len == 3 && memcmp(data, "abc", 3) == 0);
    free(data);

    editor_free(&ed);
    unlink(path);
    printf("  [PASS] test_no_trailing_newline\n");
}

static void test_crlf(void)
{
    const char *path = test_path("crlf");
    write_file(path, "a\r\nb\r\n");

    editor_t ed;
    editor_init(&ed);
    CHECK(editor_load(&ed, path));
    CHECK(ed.crlf);
    CHECK(ed.doc.count == 3);
    CHECK(strcmp(ed.doc.lines[0], "a") == 0);
    CHECK(strcmp(ed.doc.lines[1], "b") == 0);

    CHECK(editor_save(&ed));
    size_t len = 0;
    char *data = tam_fs_read_file(path, &len);
    CHECK(data != NULL && len == 6 && memcmp(data, "a\r\nb\r\n", 6) == 0);
    free(data);

    editor_free(&ed);
    unlink(path);
    printf("  [PASS] test_crlf\n");
}

static void test_missing_file(void)
{
    const char *path = test_path("missing");
    unlink(path);

    editor_t ed;
    editor_init(&ed);
    CHECK(editor_load(&ed, path));
    CHECK(ed.doc.count == 1);
    CHECK(ed.doc.lens[0] == 0);
    CHECK(!ed.dirty);
    CHECK(ed.path != NULL && strcmp(ed.path, path) == 0);

    editor_free(&ed);
    printf("  [PASS] test_missing_file\n");
}

static void test_insert_and_backspace(void)
{
    editor_t ed;
    editor_init(&ed);

    editor_insert_codepoint(&ed, 'h');
    editor_insert_codepoint(&ed, 'i');
    CHECK(ed.cx_col == 2);
    CHECK(ed.doc.lens[0] == 2);
    CHECK(ed.dirty);

    editor_backspace(&ed);
    CHECK(ed.cx_col == 1);
    CHECK(ed.doc.lens[0] == 1);
    CHECK(memcmp(ed.doc.lines[0], "h", 1) == 0);

    editor_backspace(&ed);
    CHECK(ed.cx_col == 0);
    CHECK(ed.doc.lens[0] == 0);
    CHECK(ed.doc.count == 1);

    /* Backspace at the start of the first line is a no-op. */
    editor_backspace(&ed);
    CHECK(ed.doc.count == 1);

    editor_free(&ed);
    printf("  [PASS] test_insert_and_backspace\n");
}

static void test_utf8_edit(void)
{
    editor_t ed;
    editor_init(&ed);

    editor_insert_codepoint(&ed, 0x00E9); /* e acute, two bytes */
    CHECK(ed.cx_col == 1);
    CHECK(ed.doc.lens[0] == 2);
    CHECK(edit_line_runes(ed.doc.lines[0], ed.doc.lens[0]) == 1);

    editor_insert_codepoint(&ed, 0x4E2D); /* CJK, three bytes, two cells */
    CHECK(ed.cx_col == 2);
    CHECK(ed.doc.lens[0] == 5);
    CHECK(edit_visual_col(ed.doc.lines[0], ed.doc.lens[0], 2, 8) == 3);

    editor_backspace(&ed);
    CHECK(ed.cx_col == 1);
    CHECK(ed.doc.lens[0] == 2);
    CHECK(edit_visual_col(ed.doc.lines[0], ed.doc.lens[0], 1, 8) == 1);

    editor_backspace(&ed);
    CHECK(ed.cx_col == 0);
    CHECK(ed.doc.lens[0] == 0);

    editor_free(&ed);
    printf("  [PASS] test_utf8_edit\n");
}

static void test_newline_and_join(void)
{
    editor_t ed;
    editor_init(&ed);

    editor_insert_codepoint(&ed, 'a');
    editor_insert_codepoint(&ed, 'b');
    editor_insert_codepoint(&ed, 'c');
    editor_insert_codepoint(&ed, 'd');
    editor_home(&ed);
    editor_move(&ed, 2, 0);
    editor_insert_newline(&ed);

    CHECK(ed.doc.count == 2);
    CHECK(strcmp(ed.doc.lines[0], "ab") == 0);
    CHECK(strcmp(ed.doc.lines[1], "cd") == 0);
    CHECK(ed.cx_line == 1 && ed.cx_col == 0);

    editor_move(&ed, 0, -1);
    editor_end(&ed);
    editor_delete(&ed);
    CHECK(ed.doc.count == 1);
    CHECK(strcmp(ed.doc.lines[0], "abcd") == 0);

    editor_free(&ed);
    printf("  [PASS] test_newline_and_join\n");
}

static void test_visual_columns(void)
{
    const char *line = "a\tb";
    size_t len = strlen(line);

    CHECK(edit_line_runes(line, len) == 3);
    CHECK(edit_visual_col(line, len, 0, 8) == 0);
    CHECK(edit_visual_col(line, len, 1, 8) == 1);
    CHECK(edit_visual_col(line, len, 2, 8) == 8);
    CHECK(edit_visual_col(line, len, 3, 8) == 9);

    CHECK(edit_visual_to_rune(line, len, 0, 8) == 0);
    CHECK(edit_visual_to_rune(line, len, 1, 8) == 1);
    CHECK(edit_visual_to_rune(line, len, 8, 8) == 2);
    CHECK(edit_visual_to_rune(line, len, 9, 8) == 3);

    /* Configured tabsize (4 and 2, plus <=0 fallback to 4) */
    CHECK(edit_visual_col(line, len, 2, 4) == 4);
    CHECK(edit_visual_col(line, len, 3, 4) == 5);
    CHECK(edit_visual_to_rune(line, len, 4, 4) == 2);
    CHECK(edit_visual_to_rune(line, len, 5, 4) == 3);

    CHECK(edit_visual_col(line, len, 2, 2) == 2);
    CHECK(edit_visual_col(line, len, 3, 2) == 3);
    CHECK(edit_visual_to_rune(line, len, 2, 2) == 2);
    CHECK(edit_visual_to_rune(line, len, 3, 2) == 3);

    CHECK(edit_visual_col(line, len, 2, 0) == 4);
    CHECK(edit_visual_to_rune(line, len, 4, -1) == 2);

    CHECK(edit_rune_to_byte(line, len, 0) == 0);
    CHECK(edit_rune_to_byte(line, len, 1) == 1);
    CHECK(edit_rune_to_byte(line, len, 2) == 2);
    CHECK(edit_rune_to_byte(line, len, 3) == 3);

    printf("  [PASS] test_visual_columns\n");
}

static void test_tabmovement_utf8(void)
{
    editor_t ed;
    editor_init(&ed);
    ed.config.tabmovement = true;
    ed.config.tabstospaces = true;
    ed.config.tabsize = 4;

    /* 4 spaces of indent + multi-byte UTF-8 text: tabmovement steps over indent */
    const char *s1 = "    \xC3\xA9x";
    editor_raw_insert(&ed, 0, 0, s1, strlen(s1));
    ed.cx_line = 0;
    ed.cx_col = 0;
    editor_move(&ed, 1, 0);
    CHECK(ed.cx_col == 4);
    editor_move(&ed, -1, 0);
    CHECK(ed.cx_col == 0);

    /* Non-ASCII prefix before spaces: must NOT trigger tabmovement */
    editor_free(&ed);
    editor_init(&ed);
    ed.config.tabmovement = true;
    ed.config.tabstospaces = true;
    ed.config.tabsize = 4;
    const char *s2 = "\xC3\xA9    x";
    editor_raw_insert(&ed, 0, 0, s2, strlen(s2));
    ed.cx_line = 0;
    ed.cx_col = 1;
    editor_move(&ed, 1, 0);
    CHECK(ed.cx_col == 2);

    editor_free(&ed);
    printf("  [PASS] test_tabmovement_utf8\n");
}

static void test_cursor_clamp_empty_doc(void)
{
    editor_t ed;
    editor_init(&ed);

    /* Simulate a zero-line document state with out-of-range cursors */
    for (size_t i = 0; i < ed.doc.count; i++) free(ed.doc.lines[i]);
    ed.doc.count = 0;

    ed.cx_line = 5;
    ed.cx_col = 12;
    ed.sel.active = true;
    ed.extra_cursors = calloc(1, sizeof(*ed.extra_cursors));
    ed.extra_cap = 1;
    ed.extra_count = 1;
    ed.extra_cursors[0].cx_line = 3;
    ed.extra_cursors[0].cx_col = 7;
    ed.extra_cursors[0].sel.active = true;

    cursor_clamp(&ed);
    CHECK(ed.cx_line == 0 && ed.cx_col == 0);
    CHECK(!ed.sel.active);
    CHECK(ed.extra_cursors[0].cx_line == 0 && ed.extra_cursors[0].cx_col == 0);
    CHECK(!ed.extra_cursors[0].sel.active);

    editor_free(&ed);
    printf("  [PASS] test_cursor_clamp_empty_doc\n");
}

static void test_vertical_goal(void)
{
    const char *path = test_path("goal");
    write_file(path, "long line here\nx\nanother long line\n");

    editor_t ed;
    editor_init(&ed);
    CHECK(editor_load(&ed, path));

    editor_move(&ed, 10, 0);
    CHECK(ed.cx_col == 10);

    editor_move(&ed, 0, 1);
    CHECK(ed.cx_line == 1);
    CHECK(ed.cx_col == 1);

    editor_move(&ed, 0, 1);
    CHECK(ed.cx_line == 2);
    CHECK(ed.cx_col == 10);

    editor_move(&ed, 0, -2);
    CHECK(ed.cx_line == 0);
    CHECK(ed.cx_col == 10);

    editor_free(&ed);
    unlink(path);
    printf("  [PASS] test_vertical_goal\n");
}

static void test_multicursor_downstream_metadata(void)
{
    /* 1. Backspace line-join with a downstream cursor on the same joined line */
    editor_t ed;
    editor_init(&ed);
    ed.config.autoindent = false;
    editor_insert_text(&ed, "ab\ncd ef", 8);

    ed.cx_line = 1;
    ed.cx_col = 0;
    editor_toggle_cursor_at(&ed, 1, 3);
    CHECK(ed.extra_count == 1);

    editor_backspace(&ed);
    /* Cursor at (1,3) deletes ' ' -> "cdef" (col 2), then cursor at (1,0) joins
     * onto "ab" (len 2) -> "abcdef", primary at (0,2), extra at (0,4). */
    CHECK(ed.doc.count == 1);
    CHECK(strcmp(ed.doc.lines[0], "abcdef") == 0);
    CHECK(ed.cx_line == 0 && ed.cx_col == 2);
    CHECK(ed.extra_count == 1);
    CHECK(ed.extra_cursors[0].cx_line == 0 && ed.extra_cursors[0].cx_col == 4);

    /* 2. Downstream cursor with active selection translates anchor & cursor on newline */
    editor_free(&ed);
    editor_init(&ed);
    ed.config.autoindent = false;
    editor_insert_text(&ed, "aa bb cc", 8);

    ed.cx_line = 0;
    ed.cx_col = 2;
    ed.sel.active = false;
    editor_toggle_cursor_at(&ed, 0, 8);
    CHECK(ed.extra_count == 1);
    /* Give downstream extra cursor an active selection covering "cc" (6..8) */
    ed.extra_cursors[0].sel.active = true;
    ed.extra_cursors[0].sel.anchor = (loc_t){0, 6};
    ed.extra_cursors[0].sel.cursor = (loc_t){0, 8};

    /* Insert newline: extra replaces "cc" with "\n" at (0,6)->(1,0),
     * then primary inserts "\n" at (0,2)->(1,0), shifting extra to (2,0)
     * and keeping selection inactive after edit. Next test: upstream insert
     * with downstream active selection preserved when op doesn't clear it. */
    editor_insert_newline(&ed);
    CHECK(ed.doc.count == 3);
    CHECK(strcmp(ed.doc.lines[0], "aa") == 0);
    CHECK(strcmp(ed.doc.lines[1], " bb ") == 0);
    CHECK(strcmp(ed.doc.lines[2], "") == 0);
    CHECK(ed.cx_line == 1 && ed.cx_col == 0);
    CHECK(ed.extra_count == 1);
    CHECK(ed.extra_cursors[0].cx_line == 2 && ed.extra_cursors[0].cx_col == 0);

    /* 3. Softwrap multi-cursor edit recomputes visual-X goal_col */
    editor_free(&ed);
    editor_init(&ed);
    ed.config.autoindent = false;
    ed.config.ruler = false;
    ed.active_view->w = 10;
    ed.active_view->h = 10;
    ed.active_view->softwrap = true;
    ed.active_view->wordwrap = false;

    editor_insert_text(&ed, "0123456789abc\n0123456789xyz", 27);
    ed.cx_line = 0;
    ed.cx_col = 12; /* subline 1, vx = 2 */
    editor_toggle_cursor_at(&ed, 1, 12);
    editor_insert_codepoint(&ed, 'Z'); /* now col 13 => subline 1, vx = 3 */
    CHECK(ed.cx_line == 0 && ed.cx_col == 13);
    CHECK(ed.goal_col == 3);
    CHECK(ed.extra_count == 1);
    CHECK(ed.extra_cursors[0].cx_line == 1 && ed.extra_cursors[0].cx_col == 13);
    CHECK(ed.extra_cursors[0].goal_col == 3);

    editor_free(&ed);
    printf("  [PASS] test_multicursor_downstream_metadata\n");
}

static void test_scroll_without_render(void)
{
    editor_t ed;
    editor_init(&ed);
    ed.config.ruler = false;
    ed.config.scrollmargin = 1;
    ed.config.autoindent = false;

    for (int i = 0; i < 30; i++) {
        editor_insert_text(&ed, "0123456789abcdefghijklmnopqrstuvwxyz\n", 37);
    }

    /* Size viewport via editor_layout (rows=10 -> content_height=8 -> text_rows=7, cols=20) */
    cursor_start(&ed, false);
    editor_layout(&ed, 10, 20);
    CHECK(ed.top_line == 0 && ed.left_col == 0);

    /* Move down past viewport without calling render_draw */
    cursor_move(&ed, 0, 15, false);
    CHECK(ed.cx_line == 15);
    CHECK(ed.top_line == 10); /* 15 + sm(1) - text_rows(7) + 1 == 10 */
    CHECK(ed.active_view->top_line == 10);

    /* Move right past viewport without calling render_draw */
    cursor_end(&ed, false);
    CHECK(ed.left_col > 0);
    CHECK(ed.active_view->left_col == ed.left_col);

    /* Softwrap scroll updates without render_draw */
    ed.active_view->softwrap = true;
    editor_ensure_cursor_visible(&ed);
    CHECK(ed.left_col == 0);
    CHECK(ed.active_view->left_col == 0);

    /* Verify render_draw does NOT mutate scroll state */
    ed.active_view->softwrap = false;
    ed.top_line = 0;
    ed.active_view->top_line = 0;
    renderer_t ren;
    renderer_init(&ren);
    renderer_resize(&ren, 10, 20);
    tam_tty_t tty = {.rows = 10, .cols = 20, .out_fd = -1};
    render_draw(&ren, &ed, &tty);
    CHECK(ed.top_line == 0);
    CHECK(ed.active_view->top_line == 0);
    renderer_free(&ren);

    editor_free(&ed);
    printf("  [PASS] test_scroll_without_render\n");
}

static void test_unicode_word_classes(void)
{
    /* Direct character class checks */
    CHECK(edit_is_space_char(' '));
    CHECK(edit_is_space_char(0x00A0)); /* NBSP */
    CHECK(edit_is_space_char(0x3000)); /* Ideographic space */
    CHECK(!edit_is_word_char(0x00A0));
    CHECK(!edit_is_word_char(0x2014)); /* Em dash */
    CHECK(!edit_is_word_char(0x201C)); /* Left double quotation mark */
    CHECK(!edit_is_word_char(0x3002)); /* Ideographic full stop */
    CHECK(edit_is_word_char('_'));
    CHECK(edit_is_word_char(0x00E9));  /* é */
    CHECK(edit_is_word_char(0x03B1));  /* α */
    CHECK(edit_is_word_char(0x65E5));  /* 日 */

    /* Word motion across NBSP and Unicode punctuation: "café<NBSP>—αβ" */
    editor_t ed;
    editor_init(&ed);
    const char *s = "caf\xC3\xA9\xC2\xA0\xE2\x80\x94\xCE\xB1\xCE\xB2";
    editor_insert_text(&ed, s, strlen(s));
    /* Runes: 0:'c' 1:'a' 2:'f' 3:'é' 4:NBSP 5:'—' 6:'α' 7:'β' (len 8) */
    cursor_start(&ed, false);
    cursor_word_right(&ed, false);
    CHECK(ed.cx_col == 5); /* skipped "café" + NBSP, landed on '—' */
    cursor_word_right(&ed, false);
    CHECK(ed.cx_col == 6); /* skipped '—', landed on 'α' */
    cursor_word_right(&ed, false);
    CHECK(ed.cx_col == 8); /* skipped "αβ" to EOL */

    cursor_word_left(&ed, false);
    CHECK(ed.cx_col == 6); /* back to 'α' */
    cursor_word_left(&ed, false);
    CHECK(ed.cx_col == 5); /* back to '—' */
    cursor_word_left(&ed, false);
    CHECK(ed.cx_col == 0); /* skipped NBSP + "café" back to 0 */

    /* editor_spawn_cursor_match selects only "café", stopping at em-dash */
    ed.cx_col = 1;
    editor_spawn_cursor_match(&ed, false);
    CHECK(ed.sel.active);
    CHECK(ed.sel.anchor.col == 0 && ed.sel.cursor.col == 4);

    editor_free(&ed);
    printf("  [PASS] test_unicode_word_classes\n");
}

static void test_save_without_path(void)
{
    editor_t ed;
    editor_init(&ed);
    CHECK(!editor_save(&ed));
    CHECK(strstr(ed.message, "No file name") != NULL);
    editor_free(&ed);
    printf("  [PASS] test_save_without_path\n");
}

void test_milestone1_run(void);
int  run_milestone2_tests(void);
int  run_milestone3_tests(void);
int  run_milestone4_tests(void);
int  run_milestone5_tests(void);

int main(void)
{
    setlocale(LC_CTYPE, "");

    char test_cfg_dir[256];
    snprintf(test_cfg_dir, sizeof(test_cfg_dir), "/tmp/tam-edit-test-cfg-%ld",
             (long)getpid());
    mkdir(test_cfg_dir, 0755);
    setenv("TAM_EDIT_CONFIG_HOME", test_cfg_dir, 1);

    printf("tam-edit editor tests\n");
    test_load_split();
    test_no_trailing_newline();
    test_crlf();
    test_missing_file();
    test_insert_and_backspace();
    test_utf8_edit();
    test_newline_and_join();
    test_visual_columns();
    test_tabmovement_utf8();
    test_cursor_clamp_empty_doc();
    test_multicursor_downstream_metadata();
    test_scroll_without_render();
    test_unicode_word_classes();
    test_vertical_goal();
    test_save_without_path();
    printf("editor tests passed\n\n");

    test_milestone1_run();
    run_milestone2_tests();
    run_milestone3_tests();
    run_milestone4_tests();
    run_milestone5_tests();
    syntax_registry_free();
    char ini_path[300];
    snprintf(ini_path, sizeof(ini_path), "%s/settings.ini", test_cfg_dir);
    unlink(ini_path);
    rmdir(test_cfg_dir);
    return 0;
}
