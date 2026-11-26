#include "config.h"
#include "encoding.h"

#include <tam.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void config_init_defaults(config_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    strncpy(cfg->colorscheme, "default", sizeof(cfg->colorscheme) - 1);
    cfg->softwrap     = false;
    cfg->wordwrap     = true;
    cfg->tabsize      = 4;
    cfg->tabstospaces = false;
    cfg->autoindent   = true;
    cfg->cursorline   = true;
    cfg->ruler        = true;
    cfg->statusline   = true;
    cfg->tabbar       = TABBAR_AUTO;
    cfg->scrollsync   = false;
    cfg->splitbottom  = true;
    cfg->splitright   = true;
    cfg->eofnewline   = true;
    cfg->rmtrailingws = false;
    cfg->truecolor    = TRUECOLOR_AUTO;
    strncpy(cfg->clipboard, "terminal", sizeof(cfg->clipboard) - 1);
    cfg->syntax       = true;
    cfg->detectlimit  = 100;
    strncpy(cfg->encoding, "utf-8", sizeof(cfg->encoding) - 1);
    cfg->ignorecase   = true;
    cfg->incsearch    = true;
    cfg->hlsearch     = false;
    cfg->mkparents    = false;
    cfg->scrollmargin = 3;
    cfg->tabmovement  = false;
    cfg->smartpaste   = true;
    cfg->overwrite    = false;
    strncpy(cfg->reload, "prompt", sizeof(cfg->reload) - 1);
    strncpy(cfg->helpsplit, "hsplit", sizeof(cfg->helpsplit) - 1);
}

void keybinds_free(custom_keybind_t *kb)
{
    while (kb != NULL) {
        custom_keybind_t *next = kb->next;
        free(kb->action);
        free(kb);
        kb = next;
    }
}

static bool parse_bool(const char *val, bool *out)
{
    if (strcasecmp(val, "true") == 0 ||
        strcasecmp(val, "on") == 0 ||
        strcasecmp(val, "yes") == 0 ||
        strcmp(val, "1") == 0) {
        *out = true;
        return true;
    }
    if (strcasecmp(val, "false") == 0 ||
        strcasecmp(val, "off") == 0 ||
        strcasecmp(val, "no") == 0 ||
        strcmp(val, "0") == 0) {
        *out = false;
        return true;
    }
    return false;
}

bool config_set(config_t *cfg, const char *name, const char *value)
{
    if (cfg == NULL || name == NULL || value == NULL) return false;

    if (strcmp(name, "colorscheme") == 0) {
        strncpy(cfg->colorscheme, value, sizeof(cfg->colorscheme) - 1);
        cfg->colorscheme[sizeof(cfg->colorscheme) - 1] = '\0';
        return true;
    }
    if (strcmp(name, "softwrap") == 0) {
        return parse_bool(value, &cfg->softwrap);
    }
    if (strcmp(name, "wordwrap") == 0) {
        return parse_bool(value, &cfg->wordwrap);
    }
    if (strcmp(name, "tabsize") == 0) {
        char *end = NULL;
        long v = strtol(value, &end, 10);
        if (end != value && *end == '\0' && v >= 1 && v <= 16) {
            cfg->tabsize = (int)v;
            return true;
        }
        return false;
    }
    if (strcmp(name, "tabstospaces") == 0) {
        return parse_bool(value, &cfg->tabstospaces);
    }
    if (strcmp(name, "autoindent") == 0) {
        return parse_bool(value, &cfg->autoindent);
    }
    if (strcmp(name, "cursorline") == 0) {
        return parse_bool(value, &cfg->cursorline);
    }
    if (strcmp(name, "ruler") == 0) {
        return parse_bool(value, &cfg->ruler);
    }
    if (strcmp(name, "statusline") == 0) {
        return parse_bool(value, &cfg->statusline);
    }
    if (strcmp(name, "tabbar") == 0) {
        if (strcasecmp(value, "auto") == 0) {
            cfg->tabbar = TABBAR_AUTO;
            return true;
        }
        bool b = false;
        if (parse_bool(value, &b)) {
            cfg->tabbar = b ? TABBAR_ON : TABBAR_OFF;
            return true;
        }
        return false;
    }
    if (strcmp(name, "scrollsync") == 0) {
        return parse_bool(value, &cfg->scrollsync);
    }
    if (strcmp(name, "splitbottom") == 0) {
        return parse_bool(value, &cfg->splitbottom);
    }
    if (strcmp(name, "splitright") == 0) {
        return parse_bool(value, &cfg->splitright);
    }
    if (strcmp(name, "eofnewline") == 0) {
        return parse_bool(value, &cfg->eofnewline);
    }
    if (strcmp(name, "rmtrailingws") == 0) {
        return parse_bool(value, &cfg->rmtrailingws);
    }
    if (strcmp(name, "truecolor") == 0) {
        if (strcasecmp(value, "auto") == 0) {
            cfg->truecolor = TRUECOLOR_AUTO;
            return true;
        }
        bool b = false;
        if (parse_bool(value, &b)) {
            cfg->truecolor = b ? TRUECOLOR_ON : TRUECOLOR_OFF;
            return true;
        }
        return false;
    }
    if (strcmp(name, "clipboard") == 0) {
        if (strcmp(value, "terminal") == 0 ||
            strcmp(value, "external") == 0 ||
            strcmp(value, "internal") == 0) {
            strncpy(cfg->clipboard, value, sizeof(cfg->clipboard) - 1);
            cfg->clipboard[sizeof(cfg->clipboard) - 1] = '\0';
            return true;
        }
        return false;
    }
    if (strcmp(name, "syntax") == 0) {
        return parse_bool(value, &cfg->syntax);
    }
    if (strcmp(name, "detectlimit") == 0) {
        char *end = NULL;
        long v = strtol(value, &end, 10);
        if (end != value && *end == '\0' && v >= 0 && v <= 100000) {
            cfg->detectlimit = (int)v;
            return true;
        }
        return false;
    }
    if (strcmp(name, "encoding") == 0) {
        if (!encoding_is_valid(value)) return false;
        encoding_canonical_name(value, cfg->encoding, sizeof(cfg->encoding));
        return true;
    }
    if (strcmp(name, "ignorecase") == 0) {
        return parse_bool(value, &cfg->ignorecase);
    }
    if (strcmp(name, "incsearch") == 0) {
        return parse_bool(value, &cfg->incsearch);
    }
    if (strcmp(name, "hlsearch") == 0) {
        return parse_bool(value, &cfg->hlsearch);
    }
    if (strcmp(name, "mkparents") == 0) {
        return parse_bool(value, &cfg->mkparents);
    }
    if (strcmp(name, "scrollmargin") == 0) {
        char *end = NULL;
        long v = strtol(value, &end, 10);
        if (end != value && *end == '\0' && v >= 0 && v <= 50) {
            cfg->scrollmargin = (int)v;
            return true;
        }
        return false;
    }
    if (strcmp(name, "tabmovement") == 0) {
        return parse_bool(value, &cfg->tabmovement);
    }
    if (strcmp(name, "smartpaste") == 0) {
        return parse_bool(value, &cfg->smartpaste);
    }
    if (strcmp(name, "overwrite") == 0) {
        return parse_bool(value, &cfg->overwrite);
    }
    if (strcmp(name, "reload") == 0) {
        if (strcasecmp(value, "prompt") == 0 ||
            strcasecmp(value, "auto") == 0 ||
            strcasecmp(value, "disabled") == 0) {
            for (size_t i = 0; i < sizeof(cfg->reload) - 1 && value[i]; i++) {
                cfg->reload[i] = (char)tolower((unsigned char)value[i]);
                cfg->reload[i + 1] = '\0';
            }
            return true;
        }
        return false;
    }
    if (strcmp(name, "helpsplit") == 0) {
        if (strcasecmp(value, "hsplit") == 0 ||
            strcasecmp(value, "vsplit") == 0) {
            snprintf(cfg->helpsplit, sizeof(cfg->helpsplit), "%s",
                     strcasecmp(value, "vsplit") == 0 ? "vsplit" : "hsplit");
            return true;
        }
        return false;
    }

    return false;
}

bool config_get(const config_t *cfg, const char *name, char *buf, size_t buflen)
{
    if (cfg == NULL || name == NULL || buf == NULL || buflen == 0) return false;

    if (strcmp(name, "colorscheme") == 0) {
        snprintf(buf, buflen, "%s", cfg->colorscheme);
        return true;
    }
    if (strcmp(name, "softwrap") == 0) {
        snprintf(buf, buflen, "%s", cfg->softwrap ? "true" : "false");
        return true;
    }
    if (strcmp(name, "wordwrap") == 0) {
        snprintf(buf, buflen, "%s", cfg->wordwrap ? "true" : "false");
        return true;
    }
    if (strcmp(name, "tabsize") == 0) {
        snprintf(buf, buflen, "%d", cfg->tabsize);
        return true;
    }
    if (strcmp(name, "tabstospaces") == 0) {
        snprintf(buf, buflen, "%s", cfg->tabstospaces ? "true" : "false");
        return true;
    }
    if (strcmp(name, "autoindent") == 0) {
        snprintf(buf, buflen, "%s", cfg->autoindent ? "true" : "false");
        return true;
    }
    if (strcmp(name, "cursorline") == 0) {
        snprintf(buf, buflen, "%s", cfg->cursorline ? "true" : "false");
        return true;
    }
    if (strcmp(name, "ruler") == 0) {
        snprintf(buf, buflen, "%s", cfg->ruler ? "true" : "false");
        return true;
    }
    if (strcmp(name, "statusline") == 0) {
        snprintf(buf, buflen, "%s", cfg->statusline ? "true" : "false");
        return true;
    }
    if (strcmp(name, "tabbar") == 0) {
        snprintf(buf, buflen, "%s", cfg->tabbar == TABBAR_AUTO ? "auto" :
                                    (cfg->tabbar == TABBAR_ON ? "true" : "false"));
        return true;
    }
    if (strcmp(name, "scrollsync") == 0) {
        snprintf(buf, buflen, "%s", cfg->scrollsync ? "true" : "false");
        return true;
    }
    if (strcmp(name, "splitbottom") == 0) {
        snprintf(buf, buflen, "%s", cfg->splitbottom ? "true" : "false");
        return true;
    }
    if (strcmp(name, "splitright") == 0) {
        snprintf(buf, buflen, "%s", cfg->splitright ? "true" : "false");
        return true;
    }
    if (strcmp(name, "eofnewline") == 0) {
        snprintf(buf, buflen, "%s", cfg->eofnewline ? "true" : "false");
        return true;
    }
    if (strcmp(name, "rmtrailingws") == 0) {
        snprintf(buf, buflen, "%s", cfg->rmtrailingws ? "true" : "false");
        return true;
    }
    if (strcmp(name, "truecolor") == 0) {
        snprintf(buf, buflen, "%s", cfg->truecolor == TRUECOLOR_AUTO ? "auto" :
                                    (cfg->truecolor == TRUECOLOR_ON ? "true" : "false"));
        return true;
    }
    if (strcmp(name, "clipboard") == 0) {
        snprintf(buf, buflen, "%s", cfg->clipboard);
        return true;
    }
    if (strcmp(name, "syntax") == 0) {
        snprintf(buf, buflen, "%s", cfg->syntax ? "true" : "false");
        return true;
    }
    if (strcmp(name, "detectlimit") == 0) {
        snprintf(buf, buflen, "%d", cfg->detectlimit);
        return true;
    }
    if (strcmp(name, "encoding") == 0) {
        snprintf(buf, buflen, "%s", cfg->encoding);
        return true;
    }
    if (strcmp(name, "ignorecase") == 0) {
        snprintf(buf, buflen, "%s", cfg->ignorecase ? "true" : "false");
        return true;
    }
    if (strcmp(name, "incsearch") == 0) {
        snprintf(buf, buflen, "%s", cfg->incsearch ? "true" : "false");
        return true;
    }
    if (strcmp(name, "hlsearch") == 0) {
        snprintf(buf, buflen, "%s", cfg->hlsearch ? "true" : "false");
        return true;
    }
    if (strcmp(name, "mkparents") == 0) {
        snprintf(buf, buflen, "%s", cfg->mkparents ? "true" : "false");
        return true;
    }
    if (strcmp(name, "scrollmargin") == 0) {
        snprintf(buf, buflen, "%d", cfg->scrollmargin);
        return true;
    }
    if (strcmp(name, "tabmovement") == 0) {
        snprintf(buf, buflen, "%s", cfg->tabmovement ? "true" : "false");
        return true;
    }
    if (strcmp(name, "smartpaste") == 0) {
        snprintf(buf, buflen, "%s", cfg->smartpaste ? "true" : "false");
        return true;
    }
    if (strcmp(name, "overwrite") == 0) {
        snprintf(buf, buflen, "%s", cfg->overwrite ? "true" : "false");
        return true;
    }
    if (strcmp(name, "reload") == 0) {
        snprintf(buf, buflen, "%s", cfg->reload);
        return true;
    }
    if (strcmp(name, "helpsplit") == 0) {
        snprintf(buf, buflen, "%s", cfg->helpsplit);
        return true;
    }
    return false;
}

bool config_get_default(const char *name, char *buf, size_t buflen)
{
    config_t def;
    config_init_defaults(&def);
    return config_get(&def, name, buf, buflen);
}

static const char *const k_option_names[] = {
    "autoindent", "clipboard", "colorscheme", "cursorline", "detectlimit",
    "encoding", "eofnewline", "helpsplit", "hlsearch", "ignorecase",
    "incsearch", "mkparents", "overwrite", "reload", "rmtrailingws",
    "ruler", "scrollmargin", "scrollsync", "smartpaste", "softwrap",
    "splitbottom", "splitright", "statusline", "syntax", "tabbar",
    "tabmovement", "tabsize", "tabstospaces", "truecolor", "wordwrap"
};

const char *const *config_option_names(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_option_names) / sizeof(k_option_names[0]);
    return k_option_names;
}

static char *trim_inplace(char *s)
{
    while (*s && isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
    return s;
}

bool keybind_parse_string(const char *str, tam_key_type_t *type_out,
                          uint32_t *cp_out, unsigned *mods_out)
{
    if (str == NULL || *str == '\0') return false;

    char key_buf[128];
    snprintf(key_buf, sizeof(key_buf), "%s", str);
    const char *p = trim_inplace(key_buf);
    if (*p == '\0') return false;

    /* tam-edit's own spellings, put into the form tam_key_name_parse
     * reads: run-together modifiers as micro writes them ("CtrlPageUp",
     * "AltShiftLeft") get a '+', and a lone letter means the lowercase key,
     * as it always has here. */
    char name[160];
    size_t n = 0;
    bool any_mod = false;
    for (;;) {
        static const struct { const char *word; size_t len; } mods[] = {
            {"ctrl", 4}, {"alt", 3}, {"shift", 5},
        };
        bool matched = false;
        for (size_t m = 0; m < sizeof(mods) / sizeof(mods[0]); m++) {
            size_t len = mods[m].len;
            if (strncasecmp(p, mods[m].word, len) != 0) continue;
            char next = p[len];
            if ((next == '-' || next == '+') && p[len + 1] != '\0') {
                memcpy(name + n, p, len + 1);
                n += len + 1;
                p += len + 1;
            } else if (isupper((unsigned char)next)) {
                memcpy(name + n, p, len);
                n += len;
                name[n++] = '+';
                p += len;
            } else {
                continue;
            }
            matched = any_mod = true;
            break;
        }
        if (!matched || n > sizeof(name) - 40) break;
    }
    size_t rest = strlen(p);
    if (n + rest + 1 > sizeof(name)) return false;
    memcpy(name + n, p, rest + 1);
    if (!any_mod && rest == 1 && isalpha((unsigned char)p[0]))
        name[n] = (char)tolower((unsigned char)p[0]);

    tam_key_event_t ev;
    if (!tam_key_name_parse(name, &ev)) return false;
    *type_out = ev.type;
    *cp_out   = ev.codepoint;
    *mods_out = ev.mods;
    return true;
}

void keybind_format_string(tam_key_type_t type, uint32_t cp, unsigned mods,
                           char *buf, size_t buflen)
{
    if (!buf || buflen == 0) return;
    tam_key_event_t ev = {.type = type, .codepoint = cp, .mods = mods};
    (void)tam_key_name_format(&ev, buf, buflen);
}

typedef struct {
    config_t          *cfg;
    custom_keybind_t **kb_head;
    FILE              *check_out;
    bool               check_mode;
    bool               has_error;
} ini_load_state_t;

static int ini_entry_handler(void *user, const char *section,
                             const char *name, const char *value)
{
    ini_load_state_t *st = (ini_load_state_t *)user;

    char sec_buf[128];
    char name_buf[128];
    char val_buf[256];
    snprintf(sec_buf, sizeof(sec_buf), "%s", section ? section : "");
    snprintf(name_buf, sizeof(name_buf), "%s", name ? name : "");
    snprintf(val_buf, sizeof(val_buf), "%s", value ? value : "");
    char *sec = trim_inplace(sec_buf);
    char *nm  = trim_inplace(name_buf);
    char *vl  = trim_inplace(val_buf);

    if (sec[0] == '[' && sec[strlen(sec) - 1] == ']') {
        sec[strlen(sec) - 1] = '\0';
        sec++;
        sec = trim_inplace(sec);
    }

    if (*sec == '\0' ||
        strcasecmp(sec, "settings") == 0 ||
        strcasecmp(sec, "options") == 0) {
        bool ok = config_set(st->cfg, nm, vl);
        if (st->check_mode && st->check_out != NULL) {
            if (ok) {
                fprintf(st->check_out, "  [VALID] setting '%s' = '%s'\n", nm, vl);
            } else {
                fprintf(st->check_out, "  [ERROR] invalid setting or value: '%s' = '%s'\n", nm, vl);
                st->has_error = true;
            }
        }
        return 0;
    }

    if (strcasecmp(sec, "bindings") == 0) {
        tam_key_type_t type = TAM_KEY_NONE;
        uint32_t cp = 0;
        unsigned mods = 0;
        bool ok = keybind_parse_string(nm, &type, &cp, &mods);
        if (ok) {
            if (st->kb_head != NULL) {
                custom_keybind_t *kb = malloc(sizeof(*kb));
                if (kb != NULL) {
                    kb->type      = type;
                    kb->codepoint = cp;
                    kb->mods      = mods;
                    kb->action    = strdup(vl);
                    kb->next      = *st->kb_head;
                    *st->kb_head  = kb;
                }
            }
            if (st->check_mode && st->check_out != NULL) {
                fprintf(st->check_out, "  [VALID] binding '%s' -> '%s'\n", nm, vl);
            }
        } else {
            if (st->check_mode && st->check_out != NULL) {
                fprintf(st->check_out, "  [ERROR] invalid key specifier: '%s'\n", nm);
                st->has_error = true;
            }
        }
        return 0;
    }

    if (st->check_mode && st->check_out != NULL) {
        fprintf(st->check_out, "  [WARN] unknown section '[%s]'\n", sec);
    }
    return 0;
}

static char *get_config_dir(bool create)
{
    const char *override = getenv("TAM_EDIT_CONFIG_HOME");
    char base[512];
    if (override != NULL && *override != '\0') {
        snprintf(base, sizeof(base), "%s", override);
        if (create) tam_fs_mkdir_p(base, 0755);
        return strdup(base);
    }

    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg != NULL && *xdg != '\0') {
        snprintf(base, sizeof(base), "%s/tam-edit", xdg);
    } else {
        const char *home = getenv("HOME");
        if (home == NULL) home = "";
        snprintf(base, sizeof(base), "%s/.config/tam-edit", home);
    }

    struct stat st;
    if (stat(base, &st) == 0 && S_ISDIR(st.st_mode)) {
        return strdup(base);
    }

    /* Check tamlinux/tam-edit path from libtam */
    char *tam_dir = tam_xdg_path(TAM_XDG_CONFIG, "tam-edit", NULL, false);
    if (tam_dir != NULL) {
        char ini_check[512];
        snprintf(ini_check, sizeof(ini_check), "%s/settings.ini", tam_dir);
        if (access(ini_check, F_OK) == 0) {
            return tam_dir;
        }
        free(tam_dir);
    }

    if (create) {
        tam_fs_mkdir_p(base, 0755);
    }
    return strdup(base);
}

bool config_load_file(const char *path, config_t *cfg, custom_keybind_t **kb_head)
{
    if (path == NULL || cfg == NULL) return false;
    ini_load_state_t st;
    st.cfg        = cfg;
    st.kb_head    = kb_head;
    st.check_out  = NULL;
    st.check_mode = false;
    st.has_error  = false;

    int rc = tam_ini_parse_file(path, ini_entry_handler, &st);
    return rc >= 0;
}

bool config_load(config_t *cfg, custom_keybind_t **kb_head)
{
    char *cfg_dir = get_config_dir(false);
    if (cfg_dir == NULL) return false;

    char ini_path[512];
    snprintf(ini_path, sizeof(ini_path), "%s/settings.ini", cfg_dir);
    free(cfg_dir);

    return config_load_file(ini_path, cfg, kb_head);
}

bool config_check(const char *path, FILE *out)
{
    char ini_path[512] = {0};
    if (path != NULL && *path != '\0') {
        strncpy(ini_path, path, sizeof(ini_path) - 1);
    } else {
        char *cfg_dir = get_config_dir(false);
        if (cfg_dir == NULL) return false;
        snprintf(ini_path, sizeof(ini_path), "%s/settings.ini", cfg_dir);
        free(cfg_dir);
    }

    if (out == NULL) out = stdout;
    fprintf(out, "Checking configuration '%s'...\n", ini_path);

    FILE *f = fopen(ini_path, "r");
    if (f == NULL) {
        fprintf(out, "Configuration file not found (default settings will be used).\n");
        return true;
    }
    fclose(f);

    config_t cfg;
    config_init_defaults(&cfg);

    ini_load_state_t st;
    st.cfg        = &cfg;
    st.kb_head    = NULL;
    st.check_out  = out;
    st.check_mode = true;
    st.has_error  = false;

    int rc = tam_ini_parse_file(ini_path, ini_entry_handler, &st);
    if (rc < 0 || st.has_error) {
        fprintf(out, "Configuration check failed with errors.\n");
        return false;
    }

    fprintf(out, "Configuration check passed successfully.\n");
    return true;
}

/* -------------------------------------------------------------------------- */
/* Lightweight JSON5 Parser for Micro Config Import                           */
/* -------------------------------------------------------------------------- */

typedef struct {
    char key[128];
    char val[256];
} kv_pair_t;

static size_t parse_json5_pairs(const char *text, kv_pair_t *pairs, size_t max_pairs)
{
    size_t count = 0;
    const char *p = text;

    while (*p && count < max_pairs) {
        /* Skip whitespace */
        while (*p && isspace((unsigned char)*p)) p++;

        /* Skip comments */
        if (*p == '/' && *(p + 1) == '/') {
            while (*p && *p != '\n') p++;
            continue;
        }
        if (*p == '/' && *(p + 1) == '*') {
            p += 2;
            while (*p && !(*p == '*' && *(p + 1) == '/')) p++;
            if (*p) p += 2;
            continue;
        }

        if (*p == '{' || *p == '}' || *p == ',') {
            p++;
            continue;
        }

        if (!*p) break;

        /* Parse key: unquoted or quoted */
        char key_buf[128] = {0};
        size_t klen = 0;
        char quote = 0;
        if (*p == '"' || *p == '\'') {
            quote = *p++;
            while (*p && *p != quote && klen < sizeof(key_buf) - 1) {
                key_buf[klen++] = *p++;
            }
            if (*p == quote) p++;
        } else {
            while (*p && (isalnum((unsigned char)*p) || *p == '_' || *p == '-') &&
                   klen < sizeof(key_buf) - 1) {
                key_buf[klen++] = *p++;
            }
        }
        key_buf[klen] = '\0';

        /* Expect colon */
        while (*p && (isspace((unsigned char)*p) || *p == ':')) {
            if (*p == ':') {
                p++;
                break;
            }
            p++;
        }

        while (*p && isspace((unsigned char)*p)) p++;

        /* Parse value */
        char val_buf[256] = {0};
        size_t vlen = 0;
        if (*p == '"' || *p == '\'') {
            quote = *p++;
            while (*p && *p != quote && vlen < sizeof(val_buf) - 1) {
                if (*p == '\\' && *(p + 1)) p++;
                val_buf[vlen++] = *p++;
            }
            if (*p == quote) p++;
        } else {
            while (*p && *p != ',' && *p != '}' && *p != '\n' && *p != '\r' &&
                   vlen < sizeof(val_buf) - 1) {
                val_buf[vlen++] = *p++;
            }
            /* Trim trailing whitespace */
            while (vlen > 0 && isspace((unsigned char)val_buf[vlen - 1])) {
                vlen--;
            }
        }
        val_buf[vlen] = '\0';

        if (klen > 0 && vlen > 0) {
            snprintf(pairs[count].key, sizeof(pairs[count].key), "%s", key_buf);
            snprintf(pairs[count].val, sizeof(pairs[count].val), "%s", val_buf);
            count++;
        }
    }

    return count;
}

bool config_import_micro(const char *micro_dir, const char *out_ini_path, FILE *out)
{
    if (out == NULL) out = stdout;

    char in_dir[512] = {0};
    if (micro_dir != NULL && *micro_dir != '\0') {
        strncpy(in_dir, micro_dir, sizeof(in_dir) - 1);
    } else {
        const char *home = getenv("HOME");
        if (home == NULL) home = "";
        snprintf(in_dir, sizeof(in_dir), "%s/.config/micro", home);
    }

    char target_path[512] = {0};
    if (out_ini_path != NULL && *out_ini_path != '\0') {
        strncpy(target_path, out_ini_path, sizeof(target_path) - 1);
    } else {
        char *cfg_dir = get_config_dir(true);
        if (cfg_dir == NULL) {
            fprintf(out, "Failed to determine tam-edit config directory.\n");
            return false;
        }
        snprintf(target_path, sizeof(target_path), "%s/settings.ini", cfg_dir);
        free(cfg_dir);
    }

    fprintf(out, "Importing micro configuration from '%s'...\n", in_dir);

    tam_str_t ini_out = tam_str_new("# tam-edit configuration generated from micro\n\n[settings]\n");
    if (ini_out == NULL) return false;

    /* 1. Import settings.json */
    char settings_json[512];
    snprintf(settings_json, sizeof(settings_json), "%s/settings.json", in_dir);
    size_t s_len = 0;
    char *s_data = tam_fs_read(settings_json, 0, &s_len);
    if (s_data != NULL) {
        kv_pair_t pairs[64];
        size_t n = parse_json5_pairs(s_data, pairs, 64);
        config_t test_cfg;
        config_init_defaults(&test_cfg);

        for (size_t i = 0; i < n; i++) {
            if (config_set(&test_cfg, pairs[i].key, pairs[i].val)) {
                ini_out = tam_str_cat_printf(ini_out, "%s = %s\n", pairs[i].key, pairs[i].val);
                fprintf(out, "  [SETTING] %s = %s\n", pairs[i].key, pairs[i].val);
            } else {
                fprintf(out, "  [IGNORED] unsupported micro setting: %s\n", pairs[i].key);
            }
        }
        free(s_data);
    } else {
        fprintf(out, "  Notice: %s not found.\n", settings_json);
    }

    /* 2. Import bindings.json */
    ini_out = tam_str_cat(ini_out, "\n[bindings]\n");
    char bindings_json[512];
    snprintf(bindings_json, sizeof(bindings_json), "%s/bindings.json", in_dir);
    size_t b_len = 0;
    char *b_data = tam_fs_read(bindings_json, 0, &b_len);
    if (b_data != NULL) {
        kv_pair_t pairs[128];
        size_t n = parse_json5_pairs(b_data, pairs, 128);

        for (size_t i = 0; i < n; i++) {
            const char *action = pairs[i].val;
            /* Strip command: prefix if present */
            if (strncmp(action, "command:", 8) == 0) {
                action += 8;
            }
            ini_out = tam_str_cat_printf(ini_out, "%s = %s\n", pairs[i].key, action);
            fprintf(out, "  [BINDING] %s = %s\n", pairs[i].key, action);
        }
        free(b_data);
    } else {
        fprintf(out, "  Notice: %s not found.\n", bindings_json);
    }

    /* 3. If target exists, backup to target_path.bak */
    char backup_path[540];
    snprintf(backup_path, sizeof(backup_path), "%s.bak", target_path);
    size_t existing_len = 0;
    char *existing = tam_fs_read(target_path, 0, &existing_len);
    if (existing != NULL) {
        (void)tam_fs_write_atomic(backup_path, existing, existing_len, TAM_FS_KEEP_MODE, 0644);
        fprintf(out, "Existing configuration backed up to '%s'.\n", backup_path);
        free(existing);
    }

    /* 4. Write new settings.ini */
    bool ok = tam_fs_write_atomic(target_path, ini_out, tam_str_len(ini_out),
                                  TAM_FS_KEEP_MODE, 0644) >= 0;
    if (ok) {
        fprintf(out, "Successfully wrote '%s'.\n", target_path);
    } else {
        fprintf(out, "Failed to write '%s': %s\n", target_path, strerror(errno));
    }

    tam_str_free(ini_out);
    return ok;
}

static bool binding_keys_equal(const char *left, const char *right)
{
    tam_key_event_t a, b;
    return keybind_parse_string(left, &a.type, &a.codepoint, &a.mods) &&
           keybind_parse_string(right, &b.type, &b.codepoint, &b.mods) &&
           a.type == b.type && a.codepoint == b.codepoint && a.mods == b.mods;
}

static bool ini_update_entry(const char *section, const char *key, const char *value)
{
    char *cfg_dir = get_config_dir(true);
    if (!cfg_dir) return false;

    char ini_path[512];
    snprintf(ini_path, sizeof(ini_path), "%s/settings.ini", cfg_dir);
    free(cfg_dir);

    size_t len = 0;
    char *existing = tam_fs_read(ini_path, 0, &len);
    tam_str_t out = tam_str_new("");
    if (!out) {
        free(existing);
        return false;
    }

    bool in_target_sec = false;
    bool seen_target_sec = false;
    bool wrote_entry = false;

    if (existing) {
        char *p = existing;
        while (*p) {
            char *nl = strchr(p, '\n');
            size_t line_len = nl ? (size_t)(nl - p) : strlen(p);
            char *line = malloc(line_len + 1);
            if (!line) break;
            memcpy(line, p, line_len);
            line[line_len] = '\0';
            if (line_len > 0 && line[line_len - 1] == '\r') {
                line[--line_len] = '\0';
            }

            char copy[512];
            snprintf(copy, sizeof(copy), "%s", line);
            char *t = trim_inplace(copy);

            if (t[0] == '[') {
                if (in_target_sec && !wrote_entry && value != NULL) {
                    out = tam_str_cat_printf(out, "%s = %s\n", key, value);
                    wrote_entry = true;
                }
                char *rb = strchr(t, ']');
                if (rb) *rb = '\0';
                char *sec_name = trim_inplace(t + 1);
                in_target_sec = (strcasecmp(sec_name, section) == 0);
                if (in_target_sec) seen_target_sec = true;
                out = tam_str_cat_printf(out, "%s\n", line);
            } else if (in_target_sec && t[0] != '#' && t[0] != ';' && t[0] != '\0') {
                char *eq = strchr(t, '=');
                if (eq) {
                    *eq = '\0';
                    char *k = trim_inplace(t);
                    bool match = (strcasecmp(section, "bindings") == 0)
                                 ? binding_keys_equal(k, key)
                                 : (strcmp(k, key) == 0);
                    if (match) {
                        if (value != NULL) {
                            if (!wrote_entry) {
                                out = tam_str_cat_printf(out, "%s = %s\n", key, value);
                                wrote_entry = true;
                            }
                        } else {
                            wrote_entry = true; /* omitted */
                        }
                        free(line);
                        p = nl ? (nl + 1) : (p + line_len);
                        continue;
                    }
                }
                out = tam_str_cat_printf(out, "%s\n", line);
            } else {
                out = tam_str_cat_printf(out, "%s\n", line);
            }

            free(line);
            p = nl ? (nl + 1) : (p + line_len);
        }
        free(existing);
    }

    if (value != NULL && !wrote_entry) {
        if (!seen_target_sec) {
            if (tam_str_len(out) > 0 && out[tam_str_len(out) - 1] != '\n') {
                out = tam_str_cat(out, "\n");
            }
            if (tam_str_len(out) > 0) {
                out = tam_str_cat(out, "\n");
            }
            out = tam_str_cat_printf(out, "[%s]\n", section);
        }
        out = tam_str_cat_printf(out, "%s = %s\n", key, value);
    }

    bool ok = tam_fs_write_atomic(ini_path, out, tam_str_len(out), TAM_FS_KEEP_MODE, 0644) >= 0;
    tam_str_free(out);
    return ok;
}

bool config_save_setting(const char *name, const char *value)
{
    if (!name || !name[0] || !value) return false;
    return ini_update_entry("settings", name, value);
}

bool config_save_binding(const char *key_str, const char *action_str)
{
    if (!key_str || !key_str[0] || !action_str) return false;
    return ini_update_entry("bindings", key_str, action_str);
}

bool config_remove_binding(const char *key_str)
{
    if (!key_str || !key_str[0]) return false;
    return ini_update_entry("bindings", key_str, NULL);
}
