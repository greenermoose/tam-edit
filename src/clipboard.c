#include "clipboard.h"

#include <tam.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char  *g_internal_clip = NULL;
static size_t g_internal_len  = 0;

void clipboard_init(void)
{
    g_internal_clip = NULL;
    g_internal_len = 0;
}

void clipboard_free(void)
{
    free(g_internal_clip);
    g_internal_clip = NULL;
    g_internal_len = 0;
}


static bool console_clipboard(void)
{
    const char *term = getenv("TERM");
    return term && strcmp(term, "linux") == 0;
}

bool clipboard_set(const char *text, size_t len, int tty_fd)
{
    if (text == NULL) return false;

    /* 1. Internal buffer */
    char *copy = malloc(len + 1);
    if (copy == NULL) return false;
    memcpy(copy, text, len);
    copy[len] = '\0';
    free(g_internal_clip);
    g_internal_clip = copy;
    g_internal_len = len;
    if (console_clipboard()) return true;

    /* 2. OSC 52 sequence to the terminal */
    if (tty_fd >= 0) {
        /* ESC ] 52 ; c ; <base64> BEL, written whole even when the terminal
         * takes it in pieces. */
        static const char head[] = "\x1b]52;c;";
        size_t enc = tam_b64_encoded_len(len);
        char *osc = malloc(sizeof(head) - 1 + enc + 2);
        if (osc != NULL) {
            memcpy(osc, head, sizeof(head) - 1);
            tam_b64_encode(text, len, osc + sizeof(head) - 1);
            osc[sizeof(head) - 1 + enc] = '\x07';
            (void)tam_write_all(tty_fd, osc, sizeof(head) - 1 + enc + 1);
            free(osc);
        }
    }

    /* 3. External clipboard helpers if available */
    if (!console_clipboard() && getenv("WAYLAND_DISPLAY") != NULL) {
        const char *const argv[] = {"wl-copy", NULL};
        tam_exec_opts_t opts = {
            .timeout_ms = 300,
            .input      = text,
            .input_len  = len,
            .max_output = 1024,
        };
        tam_exec_result_t res = tam_exec_opts(argv, &opts);
        tam_exec_result_free(&res);
    } else if (!console_clipboard() && getenv("DISPLAY") != NULL) {
        const char *const argv[] = {"xclip", "-selection", "clipboard", NULL};
        tam_exec_opts_t opts = {
            .timeout_ms = 300,
            .input      = text,
            .input_len  = len,
            .max_output = 1024,
        };
        tam_exec_result_t res = tam_exec_opts(argv, &opts);
        tam_exec_result_free(&res);
    }

    return true;
}

char *clipboard_get(size_t *out_len)
{
    /* 1. Try Wayland wl-paste */
    if (!console_clipboard() && getenv("WAYLAND_DISPLAY") != NULL) {
        const char *const argv[] = {"wl-paste", "--no-newline", NULL};
        tam_exec_result_t res = tam_exec(argv, 300);
        if (res.exit_code == 0 && res.stdout_buf != NULL && res.stdout_len > 0) {
            char *ret = malloc(res.stdout_len + 1);
            if (ret) memcpy(ret, res.stdout_buf, res.stdout_len + 1);
            if (out_len != NULL) *out_len = ret ? res.stdout_len : 0;
            tam_exec_result_free(&res);
            return ret;
        }
        tam_exec_result_free(&res);
    }

    /* 2. Try X11 xclip */
    if (!console_clipboard() && getenv("DISPLAY") != NULL) {
        const char *const argv[] = {"xclip", "-out", "-selection", "clipboard", NULL};
        tam_exec_result_t res = tam_exec(argv, 300);
        if (res.exit_code == 0 && res.stdout_buf != NULL && res.stdout_len > 0) {
            char *ret = malloc(res.stdout_len + 1);
            if (ret) memcpy(ret, res.stdout_buf, res.stdout_len + 1);
            if (out_len != NULL) *out_len = ret ? res.stdout_len : 0;
            tam_exec_result_free(&res);
            return ret;
        }
        tam_exec_result_free(&res);
    }

    /* 3. Fallback to internal buffer */
    if (g_internal_clip != NULL) {
        char *ret = malloc(g_internal_len + 1);
        if (ret) memcpy(ret, g_internal_clip, g_internal_len + 1);
        if (out_len != NULL) *out_len = g_internal_len;
        return ret;
    }

    if (out_len != NULL) *out_len = 0;
    return NULL;
}
