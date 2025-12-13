#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib.h>

#include "defs.h"
#include "extern.h"
#include "parser.h"

static Binding *alloc_bind(Config *cfg, unsigned mods, KeySym ks);
static char **alloc_str_pair(const char *a, const char *b);
static void dedupe_binds(Config *cfg);
static int find_free_slot(char **arr[], int max);
static FILE *open_config(char *path, size_t pathsz);
static Binding *parse_bind_line(Config *cfg, char *rest, int lineno, const char *ctx, char **out_act);
static unsigned parse_combo(const char *combo, Config *cfg, KeySym *out_ks);
static int parse_csv_to_array(char *rest, char **arr[], int *idx, int max, int alloc_pair);
static char **split_cmd(const char *cmd, int *out_argc);
static char *strip(char *s);
static char *strip_comment(char *s);
static char *strip_quotes(char *s);

static const CommandEntry call_table[] = {
	{"centre_window",             centre_window},
	{"close_window",              close_focused},
	{"decrease_gaps",             dec_gaps},
	{"focus_next",                focus_next},
	{"focus_prev",                focus_prev},
	{"focus_next_mon",            focus_next_mon},
	{"focus_prev_mon",            focus_prev_mon},
	{"fullscreen",                toggle_fullscreen},
	{"global_floating",           toggle_floating_global},
	{"increase_gaps",             inc_gaps},
	{"master_next",               move_master_next},
	{"master_prev",               move_master_prev},
	{"master_increase",           resize_master_add},
	{"master_decrease",           resize_master_sub},
	{"move_next_mon",             move_next_mon},
	{"move_prev_mon",             move_prev_mon},
	{"move_win_up",               move_win_up},
	{"move_win_down",             move_win_down},
	{"move_win_left",             move_win_left},
	{"move_win_right",            move_win_right},
	{"quit",                      quit},
	{"reload_config",             reload_config},
	{"resize_win_up",             resize_win_up},
	{"resize_win_down",           resize_win_down},
	{"resize_win_left",           resize_win_left},
	{"resize_win_right",          resize_win_right},
	{"stack_increase",            resize_stack_add},
	{"stack_decrease",            resize_stack_sub},
	{"switch_previous_workspace", switch_previous_workspace},
	{"toggle_floating",           toggle_floating},
	{"toggle_monocle",            toggle_monocle},
	{NULL, NULL},
};

static Binding *alloc_bind(Config *cfg, unsigned mods, KeySym ks)
{
	for (int i = 0; i < cfg->n_binds; i++) {
		if (cfg->binds[i].mods == (int)mods && cfg->binds[i].keysym == ks)
			return &cfg->binds[i];
	}

	if (cfg->n_binds >= MAX_BINDS)
		return NULL;

	Binding *b = &cfg->binds[cfg->n_binds++];
	b->mods = mods;
	b->keysym = ks;
	return b;
}

static char **alloc_str_pair(const char *a, const char *b)
{
	char **p = malloc(2 * sizeof(char *));
	if (!p)
		return NULL;

	p[0] = a ? strdup(a) : NULL;
	p[1] = b ? strdup(b) : NULL;
	return p;
}

const char **build_argv(const char *cmd)
{
	int argc = 0;
	char **tmp = split_cmd(cmd, &argc);
	if (!tmp)
		return NULL;

	return (const char **)tmp;
}

static void dedupe_binds(Config *cfg)
{
	for (int i = 0; i < cfg->n_binds; i++) {
		for (int j = i + 1; j < cfg->n_binds; j++) {
			Bool dup = cfg->binds[i].mods == cfg->binds[j].mods &&
			           cfg->binds[i].keysym == cfg->binds[j].keysym;
			if (dup) {
				memmove(
					&cfg->binds[j], &cfg->binds[j + 1],
					sizeof(Binding) * (cfg->n_binds - j - 1)
				);
				cfg->n_binds--;
				j--;
			}
		}
	}
}

static int find_free_slot(char **arr[], int max)
{
	for (int i = 0; i < max; i++) {
		if (!arr[i])
			return i;
	}

	return -1;
}

static FILE *open_config(char *path, size_t pathsz)
{
	const char *home = getenv("HOME");
	if (!home) {
		fputs("sxwmrc: HOME not set\n", stderr);
		return NULL;
	}

	const char *xdg = getenv("XDG_CONFIG_HOME");
	const char *paths[] = {
		"%s/sxwmrc",
		"%s/sxwm/sxwmrc",
	};

	if (xdg) {
		for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
			snprintf(path, pathsz, paths[i], xdg);
			if (access(path, R_OK) == 0)
				goto found;
		}
	}

	snprintf(path, pathsz, "%s/.config/sxwmrc", home);
	if (access(path, R_OK) == 0)
		goto found;

	snprintf(path, pathsz, "/usr/local/share/sxwmrc");
	if (access(path, R_OK) == 0)
		goto found;

	fprintf(stderr, "sxwmrc: no configuration file found\n");
	return NULL;

found:
	printf("sxwmrc: using configuration file %s\n", path);
	FILE *f = fopen(path, "r");
	if (!f)
		fprintf(stderr, "sxwmrc: cannot open %s\n", path);

	return f;
}

static Binding *parse_bind_line(Config *cfg, char *rest, int lineno, const char *ctx, char **out_act)
{
	char *mid = strchr(rest, ':');
	if (!mid) {
		fprintf(stderr, "sxwmrc:%d: %s missing action\n", lineno, ctx);
		return NULL;
	}
	*mid = '\0';
	*out_act = strip(mid + 1);

	KeySym ks;
	unsigned mods = parse_combo(strip(rest), cfg, &ks);
	if (ks == NoSymbol) {
		fprintf(stderr, "sxwmrc:%d: bad key in '%s'\n", lineno, rest);
		return NULL;
	}

	Binding *b = alloc_bind(cfg, mods, ks);
	if (!b)
		fputs("sxwm: too many binds\n", stderr);

	return b;
}

static unsigned parse_combo(const char *combo, Config *cfg, KeySym *out_ks)
{
	unsigned m = 0;
	KeySym ks = NoSymbol;
	char buf[MAX_ITEMS];

	strncpy(buf, combo, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	for (char *p = buf; *p; p++) {
		if (*p == '+' || isspace((unsigned char)*p))
			*p = '+';
	}

	for (char *tok = strtok(buf, "+"); tok; tok = strtok(NULL, "+")) {
		if (!strcmp(tok, "mod")) m |= cfg->modkey;
		else if (!strcmp(tok, "shift")) m |= ShiftMask;
		else if (!strcmp(tok, "ctrl")) m |= ControlMask;
		else if (!strcmp(tok, "alt")) m |= Mod1Mask;
		else if (!strcmp(tok, "super")) m |= Mod4Mask;
		else {
			ks = XStringToKeysym(tok);
			if (ks == NoSymbol)
				ks = parse_keysym(tok);
		}
	}
	*out_ks = ks;
	return m;
}

static int parse_csv_to_array(char *rest, char **arr[], int *idx, int max, int alloc_pair)
{
	char *save, *tok;
	for (tok = strtok_r(rest, ",", &save); tok && *idx < max; tok = strtok_r(NULL, ",", &save)) {
		char *item = strip_quotes(strip(tok));
		if (!*item)
			continue;

		if (alloc_pair) {
			arr[*idx] = alloc_str_pair(item, NULL);
			if (!arr[*idx])
				return -1;
		}
		else {
			if (!arr[*idx])
				arr[*idx] = calloc(2, sizeof(char *));
			if (!arr[*idx])
				return -1;
			arr[*idx][0] = strdup(item);
		}
		(*idx)++;
	}
	return 0;
}

KeySym parse_keysym(const char *key)
{
	KeySym ks = XStringToKeysym(key);
	if (ks != NoSymbol)
		return ks;

	char buf[64];
	size_t n = strlen(key);
	if (n >= sizeof buf)
		n = sizeof buf - 1;

	/* try Capitalized */
	buf[0] = toupper((unsigned char)key[0]);
	for (size_t i = 1; i < n; i++)
		buf[i] = tolower((unsigned char)key[i]);
	buf[n] = '\0';
	if ((ks = XStringToKeysym(buf)) != NoSymbol)
		return ks;

	/* try UPPERCASE */
	for (size_t i = 0; i < n; i++)
		buf[i] = toupper((unsigned char)key[i]);
	buf[n] = '\0';
	if ((ks = XStringToKeysym(buf)) != NoSymbol)
		return ks;

	fprintf(stderr, "sxwmrc: unknown keysym '%s'\n", key);
	return NoSymbol;
}

int parse_mods(const char *mods, Config *cfg)
{
	KeySym dummy;
	return parse_combo(mods, cfg, &dummy);
}

int parser(Config *cfg)
{
	char path[PATH_MAX];
	FILE *f = open_config(path, sizeof(path));
	if (!f)
		return -1;

	char line[512];
	int lineno = 0, should_floatn = 0, to_run = 0;

	for (int j = 0; j < MAX_ITEMS; j++) {
		cfg->should_float[j] = calloc(2, sizeof(char *));
		if (!cfg->should_float[j])
			goto cleanup;
	}

	while (fgets(line, sizeof line, f)) {
		lineno++;
		char *s = strip(line);
		if (!*s || *s == '#')
			continue;

		char *sep = strchr(s, ':');
		if (!sep) {
			fprintf(stderr, "sxwmrc:%d: missing ':'\n", lineno);
			continue;
		}
		*sep = '\0';
		char *key = strip(s);
		char *rest = strip(sep + 1);

		if (!strcmp(key, "border_width"))
			cfg->border_width = atoi(rest);
		else if (!strcmp(key, "call") || !strcmp(key, "bind")) {
			char *act;
			Binding *b = parse_bind_line(cfg, rest, lineno, key, &act);
			if (!b)
				continue;

			if (*act == '"' && !strcmp(key, "bind")) {
				b->type = TYPE_CMD;
				b->action.cmd = build_argv(strip_quotes(act));
				if (!b->action.cmd) {
					fprintf(stderr, "sxwmrc:%d: failed to parse command: %s\n", lineno, act);
					b->type = -1;
				}
			}
			else {
				b->type = TYPE_FUNC;
				b->action.fn = NULL;
				for (int i = 0; call_table[i].name; i++) {
					if (!strcmp(act, call_table[i].name)) {
						b->action.fn = call_table[i].fn;
						break;
					}
				}
				if (!b->action.fn)
					fprintf(stderr, "sxwmrc:%d: unknown function '%s'\n", lineno, act);
			}
		}
		else if (!strcmp(key, "can_be_swallowed")) {
			int idx = find_free_slot(cfg->can_be_swallowed, MAX_ITEMS);
			if (idx < 0)
				idx = 0;
			parse_csv_to_array(rest, cfg->can_be_swallowed, &idx, MAX_ITEMS, 1);
		}
		else if (!strcmp(key, "can_swallow")) {
			int idx = find_free_slot(cfg->can_swallow, MAX_ITEMS);
			if (idx < 0)
				idx = 0;
			parse_csv_to_array(rest, cfg->can_swallow, &idx, MAX_ITEMS, 1);
		}
		else if (!strcmp(key, "exec")) {
			if (to_run >= MAX_ITEMS) {
				fprintf(stderr, "sxwmrc:%d: too many exec commands\n", lineno);
				continue;
			}
			char *cmd = strip_quotes(strip_comment(rest));
			if (!*cmd) {
				fprintf(stderr, "sxwmrc:%d: empty exec command\n", lineno);
				continue;
			}
			cfg->to_run[to_run] = strdup(cmd);
			if (!cfg->to_run[to_run])
				goto cleanup;
			to_run++;
		}
		else if (!strcmp(key, "floating_on_top"))
			cfg->floating_on_top = !strcmp(rest, "true");
		else if (!strcmp(key, "focused_border_colour"))
			cfg->border_foc_col = parse_col(rest);
		else if (!strcmp(key, "gaps"))
			cfg->gaps = atoi(rest);
		else if (!strcmp(key, "master_width")) {
			float mf = (float)atoi(rest) / 100.0f;
			for (int i = 0; i < MAX_MONITORS; i++)
				cfg->master_width[i] = mf;
		}
		else if (!strcmp(key, "mod_key")) {
			unsigned m = parse_mods(rest, cfg);
			if (m & (Mod1Mask | Mod4Mask | ShiftMask | ControlMask))
				cfg->modkey = m;
			else
				fprintf(stderr, "sxwmrc:%d: unknown mod_key '%s'\n", lineno, rest);
		}
		else if (!strcmp(key, "motion_throttle"))
			cfg->motion_throttle = atoi(rest);
		else if (!strcmp(key, "move_window_amount"))
			cfg->move_window_amt = atoi(rest);
		else if (!strcmp(key, "new_win_focus"))
			cfg->new_win_focus = !strcmp(rest, "true");
		else if (!strcmp(key, "new_win_master"))
			cfg->new_win_master = !strcmp(rest, "true");
		else if (!strcmp(key, "open_in_workspace")) {
			char *mid = strchr(rest, ':');
			if (!mid) {
				fprintf(stderr, "sxwmrc:%d: open_in_workspace missing workspace\n", lineno);
				continue;
			}
			*mid = '\0';
			char *cls = strip_quotes(strip(rest));
			int ws = atoi(strip(mid + 1));
			if (ws < 1 || ws > NUM_WORKSPACES) {
				fprintf(stderr, "sxwmrc:%d: invalid workspace number %d\n", lineno, ws);
				continue;
			}
			int slot = find_free_slot(cfg->open_in_workspace, MAX_ITEMS);
			if (slot >= 0) {
				char ws_buf[16];
				snprintf(ws_buf, sizeof(ws_buf), "%d", ws - 1);
				cfg->open_in_workspace[slot] = alloc_str_pair(cls, ws_buf);
			}
		}
		else if (!strcmp(key, "resize_master_amount"))
			cfg->resize_master_amt = atoi(rest);
		else if (!strcmp(key, "resize_stack_amount"))
			cfg->resize_stack_amt = atoi(rest);
		else if (!strcmp(key, "resize_window_amount"))
			cfg->resize_window_amt = atoi(rest);
		else if (!strcmp(key, "scratchpad")) {
			char *act;
			Binding *b = parse_bind_line(cfg, rest, lineno, "scratchpad", &act);
			if (!b)
				goto cleanup;

			int n, found = 0;
			static const struct { const char *fmt; int type; } sp_acts[] = {
				{"create %d", TYPE_SP_CREATE},
				{"toggle %d", TYPE_SP_TOGGLE},
				{"remove %d", TYPE_SP_REMOVE},
			};
			for (size_t i = 0; i < sizeof(sp_acts) / sizeof(sp_acts[0]); i++) {
				if (sscanf(act, sp_acts[i].fmt, &n) == 1 && n >= 1 && n <= MAX_SCRATCHPADS) {
					b->type = sp_acts[i].type;
					b->action.sp = n - 1;
					found = 1;
					break;
				}
			}
			if (!found)
				fprintf(stderr, "sxwmrc:%d: invalid scratchpad action '%s'\n", lineno, act);
		}
		else if (!strcmp(key, "should_float")) {
			char *clean = strip_comment(rest);
			if (parse_csv_to_array(clean, cfg->should_float, &should_floatn, MAX_ITEMS, 0) < 0)
				goto cleanup;
		}
		else if (!strcmp(key, "snap_distance"))
			cfg->snap_distance = atoi(rest);
		else if (!strcmp(key, "start_fullscreen")) {
			int idx = find_free_slot(cfg->start_fullscreen, MAX_ITEMS);
			if (idx < 0)
				idx = 0;

			char *clean = strip_comment(rest);
			if (parse_csv_to_array(clean, cfg->start_fullscreen, &idx, MAX_ITEMS, 1) < 0)
				goto cleanup;
		}
		else if (!strcmp(key, "swap_border_colour"))
			cfg->border_swap_col = parse_col(rest);
		else if (!strcmp(key, "unfocused_border_colour"))
			cfg->border_ufoc_col = parse_col(rest);
		else if (!strcmp(key, "warp_cursor"))
			cfg->warp_cursor = !strcmp(rest, "true");
		else if (!strcmp(key, "workspace")) {
			char *act;
			Binding *b = parse_bind_line(cfg, rest, lineno, "workspace", &act);
			if (!b)
				continue;

			int n;
			if (sscanf(act, "move %d", &n) == 1 && n >= 1 && n <= NUM_WORKSPACES) {
				b->type = TYPE_WS_CHANGE;
				b->action.ws = n - 1;
			}
			else if (sscanf(act, "swap %d", &n) == 1 && n >= 1 && n <= NUM_WORKSPACES) {
				b->type = TYPE_WS_MOVE;
				b->action.ws = n - 1;
			}
			else {
				fprintf(stderr, "sxwmrc:%d: invalid workspace action '%s'\n", lineno, act);
			}
		}
		else {
			fprintf(stderr, "sxwmrc:%d: unknown option '%s'\n", lineno, key);
		}
	}

	fclose(f);
	dedupe_binds(cfg);
	return 0;

cleanup:
	fclose(f);
	for (int j = 0; j < MAX_ITEMS; j++) {
		if (cfg->should_float[j]) {
			free(cfg->should_float[j][0]);
			free(cfg->should_float[j]);
		}

		if (cfg->can_swallow[j]) {
			free(cfg->can_swallow[j][0]);
			free(cfg->can_swallow[j]);
		}

		if (cfg->can_be_swallowed[j]) {
			free(cfg->can_be_swallowed[j][0]);
			free(cfg->can_be_swallowed[j]);
		}

		if (cfg->open_in_workspace[j]) {
			free(cfg->open_in_workspace[j][0]);
			free(cfg->open_in_workspace[j][1]);
			free(cfg->open_in_workspace[j]);
		}
	}
	for (int i = 0; i < to_run; i++)
		free(cfg->to_run[i]);

	return -1;
}

static char **split_cmd(const char *cmd, int *out_argc)
{
	enum { NORMAL, IN_QUOTE } state = NORMAL;
	size_t cap = 8, argc = 0, toklen = 0;
	char *token = malloc(strlen(cmd) + 1);
	char **argv = malloc(cap * sizeof *argv);

	if (!token || !argv)
		goto err;

	for (const char *p = cmd; *p; p++) {
		if (state == NORMAL && isspace((unsigned char)*p)) {
			if (toklen) {
				token[toklen] = '\0';
				if (argc + 1 >= cap) {
					cap *= 2;
					char **tmp = realloc(argv, cap * sizeof *argv);
					if (!tmp)
						goto err;

					argv = tmp;
				}
				argv[argc++] = strdup(token);
				toklen = 0;
			}
		}
		else if (*p == '"')
			state = (state == NORMAL) ? IN_QUOTE : NORMAL;
		else
			token[toklen++] = *p;
	}

	if (toklen) {
		token[toklen] = '\0';
		argv[argc++] = strdup(token);
	}
	argv[argc] = NULL;
	*out_argc = argc;
	free(token);
	return argv;

err:
	free(token);
	if (argv) {
		for (size_t i = 0; i < argc; i++)
			free(argv[i]);
		free(argv);
	}
	return NULL;
}

static char *strip(char *s)
{
	while (*s && isspace((unsigned char)*s))
		s++;
	if (!*s)
		return s;
	char *e = s + strlen(s) - 1;
	while (e > s && isspace((unsigned char)*e))
		*e-- = '\0';
	return s;
}

static char *strip_comment(char *s)
{
	char *c = strchr(s, '#');
	if (c)
		*c = '\0';
	return strip(s);
}

static char *strip_quotes(char *s)
{
	size_t len = strlen(s);
	if (len > 0 && s[0] == '"') {
		s++;
		len--;
	}
	if (len > 0 && s[len - 1] == '"')
		s[len - 1] = '\0';
	return s;
}
