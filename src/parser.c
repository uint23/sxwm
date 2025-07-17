#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <wordexp.h>
#endif
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include "parser.h"
#include "defs.h"

static const CommandEntry call_table[] = {{"close_window", close_focused},
                                          {"decrease_gaps", dec_gaps},
                                          {"focus_next", focus_next},
                                          {"focus_prev", focus_prev},
                                          {"focus_next_mon", focus_next_mon},
                                          {"focus_prev_mon", focus_prev_mon},
                                          {"increase_gaps", inc_gaps},
                                          {"master_next", move_master_next},
                                          {"master_previous", move_master_prev},
                                          {"move_next_mon", move_next_mon},
                                          {"move_prev_mon", move_prev_mon},
                                          {"quit", quit},
                                          {"reload_config", reload_config},
                                          {"master_increase", resize_master_add},
                                          {"master_decrease", resize_master_sub},
                                          {"stack_increase", resize_stack_add},
                                          {"stack_decrease", resize_stack_sub},
                                          {"toggle_floating", toggle_floating},
                                          {"global_floating", toggle_floating_global},
                                          {"fullscreen", toggle_fullscreen},
                                          {"centre_window", centre_window},
                                          {NULL, NULL}};

static void remap_and_dedupe_binds(Config *cfg)
{
	for (int i = 0; i < cfg->bindsn; i++) {
		for (int j = i + 1; j < cfg->bindsn; j++) {
			if (cfg->binds[i].mods == cfg->binds[j].mods && cfg->binds[i].keysym == cfg->binds[j].keysym) {
				memmove(&cfg->binds[j], &cfg->binds[j + 1], sizeof(Binding) * (cfg->bindsn - j - 1));
				cfg->bindsn--;
				j--;
			}
		}
	}
}

static char *strip(char *s)
{
	while (*s && isspace((unsigned char)*s)) {
		s++;
	}
	if (*s == '\0') {
		return s;
	}

	char *e = s + strlen(s) - 1;
	while (e > s && isspace((unsigned char)*e)) {
		*e-- = '\0';
	}
	return s;
}

static char *strip_quotes(char *s)
{
	size_t L = strlen(s);
	if (L > 0 && s[0] == '"') {
		s++;
		L--;
	}
	if (L > 0 && s[L - 1] == '"') {
		s[L - 1] = '\0';
	}
	return s;
}

static Binding *alloc_bind(Config *cfg, unsigned mods, KeySym ks)
{
	for (int i = 0; i < cfg->bindsn; i++) {
		if (cfg->binds[i].mods == (int)mods && cfg->binds[i].keysym == ks) {
			return &cfg->binds[i];
		}
	}
	if (cfg->bindsn >= MAX_BINDS) {
		return NULL;
	}
	Binding *b = &cfg->binds[cfg->bindsn++];
	b->mods = mods;
	b->keysym = ks;
	return b;
}

KeySym parse_keysym(const char *tok);

static unsigned parse_combo(const char *combo, Config *cfg, KeySym *out_ks)
{
	unsigned m = 0;
	KeySym ks = NoSymbol;
	char buf[256];

	strncpy(buf, combo, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	for (char *p = buf; *p; p++) {
		if (*p == '+' || isspace((unsigned char)*p)) {
			*p = '+';
		}
	}

	for (char *tok = strtok(buf, "+"); tok; tok = strtok(NULL, "+")) {
		if (!strcmp(tok, "mod")) {
			m |= cfg->modkey;
		}
		else if (!strcmp(tok, "shift")) {
			m |= ShiftMask;
		}
		else if (!strcmp(tok, "ctrl")) {
			m |= ControlMask;
		}
		else if (!strcmp(tok, "alt")) {
			m |= Mod1Mask;
		}
		else if (!strcmp(tok, "super")) {
			m |= Mod4Mask;
		}
		else {
			ks = XStringToKeysym(tok);
			if (ks == NoSymbol) {
				ks = parse_keysym(tok);
			}
		}
	}

	*out_ks = ks;
	return m;
}

int parser(Config *cfg)
{
	char path[PATH_MAX];
	const char *home = getenv("HOME");
	if (!home) {
		fputs("sxwmrc: HOME not set\n", stderr);
		return -1;
	}

	/* determine config file path */
	const char *xdg_config_home = getenv("XDG_CONFIG_HOME");
	if (xdg_config_home) {
		snprintf(path, sizeof path, "%s/sxwmrc", xdg_config_home);
		if (access(path, R_OK) == 0) {
			goto found;
		}

		snprintf(path, sizeof path, "%s/sxwm/sxwmrc", xdg_config_home);
		if (access(path, R_OK) == 0) {
			goto found;
		}
	}

	snprintf(path, sizeof path, "%s/.config/sxwmrc", home);
	if (access(path, R_OK) == 0) {
		goto found;
	}

	snprintf(path, sizeof path, "/usr/local/share/sxwmrc");
	if (access(path, R_OK) == 0) {
		goto found;
	}

	/* Nothing found */
	fprintf(stderr, "sxwmrc: no configuration file found\n");
	return -1;

found:
	printf("sxwmrc: using configuration file %s\n", path);
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "sxwmrc: cannot open %s\n", path);
		return -1;
	}

	char line[512];
	int lineno = 0;
	int should_floatn = 0;
	int torun = 0;

	/* Initialize should_float matrix */
	for (int j = 0; j < 256; j++) {
		cfg->should_float[j] = calloc(2, sizeof(char *));
		if (!cfg->should_float[j]) {
			goto cleanup_file;
		}
	}

	while (fgets(line, sizeof line, f)) {
		lineno++;
		char *s = strip(line);
		if (!*s || *s == '#') {
			continue;
		}

		char *sep = strchr(s, ':');
		if (!sep) {
			fprintf(stderr, "sxwmrc:%d: missing ':'\n", lineno);
			continue;
		}

		*sep = '\0';
		char *key = strip(s);
		char *rest = strip(sep + 1);

		if (!strcmp(key, "mod_key")) {
			unsigned m = parse_mods(rest, cfg);
			if (m & (Mod1Mask | Mod4Mask | ShiftMask | ControlMask)) {
				cfg->modkey = m;
			}
			else {
				fprintf(stderr, "sxwmrc:%d: unknown mod_key '%s'\n", lineno, rest);
			}
		}
		else if (!strcmp(key, "gaps")) {
			cfg->gaps = atoi(rest);
		}
		else if (!strcmp(key, "border_width")) {
			cfg->border_width = atoi(rest);
		}
		else if (!strcmp(key, "focused_border_colour")) {
			cfg->border_foc_col = parse_col(rest);
		}
		else if (!strcmp(key, "unfocused_border_colour")) {
			cfg->border_ufoc_col = parse_col(rest);
		}
		else if (!strcmp(key, "swap_border_colour")) {
			cfg->border_swap_col = parse_col(rest);
		}
		else if (!strcmp(key, "new_win_focus")) {
			cfg->new_win_focus = !strcmp(rest, "true") ? True : False;
		}
		else if (!strcmp(key, "warp_cursor")) {
			cfg->warp_cursor = !strcmp(rest, "true") ? True : False;
		}
		else if (!strcmp(key, "master_width")) {
			float mf = (float)atoi(rest) / 100.0f;
			for (int i = 0; i < MAX_MONITORS; i++) {
				cfg->master_width[i] = mf;
			}
		}
		else if (!strcmp(key, "motion_throttle")) {
			cfg->motion_throttle = atoi(rest);
		}
		else if (!strcmp(key, "resize_master_amount")) {
			cfg->resize_master_amt = atoi(rest);
		}
		else if (!strcmp(key, "resize_stack_amount")) {
			cfg->resize_stack_amt = atoi(rest);
		}
		else if (!strcmp(key, "snap_distance")) {
			cfg->snap_distance = atoi(rest);
		}
		else if (!strcmp(key, "should_float")) {
			if (should_floatn >= 256) {
				fprintf(stderr, "sxwmrc:%d: too many should_float entries\n", lineno);
				continue;
			}

			char *comment = strchr(rest, '#');
			size_t len = comment ? (size_t)(comment - rest) : strlen(rest);
			if (len >= sizeof(line)) {
				len = sizeof(line) - 1;
			}

			char win[sizeof(line)];
			snprintf(win, sizeof(win), "%.*s", (int)len, rest);

			char *final = strip(win);
			char *comma_ptr;
			char *comma = strtok_r(final, ",", &comma_ptr);

			/* store each comma separated value in a seperate row */
			while (comma && should_floatn < 256) {
				comma = strip(comma);
				if (*comma == '"') {
					comma++;
				}
				char *end = comma + strlen(comma) - 1;
				if (*end == '"') {
					*end = '\0';
				}

				char *dup = strdup(comma);
				if (!dup) {
					fprintf(stderr, "sxwmrc:%d: failed to allocate memory\n", lineno);
					goto cleanup_file;
				}

				/* store each programs name in its own row at index 0 */
				cfg->should_float[should_floatn][0] = dup;
				should_floatn++;
				comma = strtok_r(NULL, ",", &comma_ptr);
			}
		}
		else if (!strcmp(key, "call") || !strcmp(key, "bind")) {
			char *mid = strchr(rest, ':');
			if (!mid) {
				fprintf(stderr, "sxwmrc:%d: '%s' missing action\n", lineno, key);
				continue;
			}
			*mid = '\0';
			char *combo = strip(rest);
			char *act = strip(mid + 1);

			KeySym ks;
			unsigned mods = parse_combo(combo, cfg, &ks);
			if (ks == NoSymbol) {
				fprintf(stderr, "sxwmrc:%d: bad key in '%s'\n", lineno, combo);
				continue;
			}

			Binding *b = alloc_bind(cfg, mods, ks);
			if (!b) {
				fputs("sxwm: too many binds\n", stderr);
				goto cleanup_file;
			}

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
				Bool found = False;
				for (int i = 0; call_table[i].name; i++) {
					if (!strcmp(act, call_table[i].name)) {
						b->action.fn = call_table[i].fn;
						found = True;
						break;
					}
				}
				if (!found) {
					fprintf(stderr, "sxwmrc:%d: unknown function '%s'\n", lineno, act);
				}
			}
		}
		else if (!strcmp(key, "workspace")) {
			char *mid = strchr(rest, ':');
			if (!mid) {
				fprintf(stderr, "sxwmrc:%d: workspace missing action\n", lineno);
				continue;
			}
			*mid = '\0';
			char *combo = strip(rest);
			char *act = strip(mid + 1);

			KeySym ks;
			unsigned mods = parse_combo(combo, cfg, &ks);
			if (ks == NoSymbol) {
				fprintf(stderr, "sxwmrc:%d: bad key in '%s'\n", lineno, combo);
				continue;
			}

			Binding *b = alloc_bind(cfg, mods, ks);
			if (!b) {
				fputs("sxwm: too many binds\n", stderr);
				break;
			}

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
		else if (!strcmp(key, "scratchpad")) {
			char *mid = strchr(rest, ':');
			if (!mid) {
				fprintf(stderr, "sxwmrc:%d: scratchpad missing action\n", lineno);
				continue;
			}
			*mid = '\0';
			char *combo = strip(rest);
			char *act = strip(mid + 1);

			KeySym ks;
			unsigned mods = parse_combo(combo, cfg, &ks);
			if (ks == NoSymbol) {
				fprintf(stderr, "sxwmrc:%d: bad key in '%s'\n", lineno, combo);
				continue;
			}

			Binding *b = alloc_bind(cfg, mods, ks);
			if (!b) {
				fputs("sxwm: too many binds\n", stderr);
				goto cleanup_file;
			}

			int padnum = -1;

			if (sscanf(act, "create %d", &padnum) == 1 && padnum >= 1 && padnum <= MAX_SCRATCHPADS) {
				b->type = TYPE_SP_CREATE;
				b->action.sp = padnum - 1;
			}
			else if (sscanf(act, "toggle %d", &padnum) == 1 && padnum >= 1 && padnum <= MAX_SCRATCHPADS) {
				b->type = TYPE_SP_TOGGLE;
				b->action.sp = padnum - 1;
			}
			else if (sscanf(act, "remove %d", &padnum) == 1 && padnum >= 1 && padnum <= MAX_SCRATCHPADS) {
				b->type = TYPE_SP_REMOVE;
				b->action.sp = padnum - 1;
			}
			else {
				fprintf(stderr, "sxwmrc:%d: invalid scratchpad action '%s'\n", lineno, act);
			}
		}
		else if (!strcmp(key, "exec")) {
			if (torun >= 256) {
				fprintf(stderr, "sxwmrc:%d: too many exec commands\n", lineno);
				continue;
			}

			char *comment = strchr(rest, '#');
			if (comment) {
				*comment = '\0';
			}

			char *cmd = strip(rest);
			cmd = strip_quotes(cmd);

			if (!*cmd) {
				fprintf(stderr, "sxwmrc:%d: empty exec command\n", lineno);
				continue;
			}

			cfg->torun[torun] = strdup(cmd);
			if (!cfg->torun[torun]) {
				fprintf(stderr, "sxwmrc:%d: failed to allocate memory for exec command\n", lineno);
				goto cleanup_file;
			}
			torun++;
		}
		else if (!strcmp(key, "can_swallow")) {
			char *token = strtok(rest, ",");
			int i = 0;
			while (token && i < 256) {
				char *item = strip_quotes(strip(token));
				if (*item) {
					cfg->can_swallow[i] = malloc(2 * sizeof(char *));
					if (!cfg->can_swallow[i]) {
						fprintf(stderr, "sxwmrc:%d: malloc failed\n", lineno);
						break;
					}
					cfg->can_swallow[i][0] = strdup(item);
					cfg->can_swallow[i][1] = NULL;
					i++;
				}
				token = strtok(NULL, ",");
			}
		}
		else if (!strcmp(key, "can_be_swallowed")) {
			char *token = strtok(rest, ",");
			int i = 0;
			while (token && i < 256) {
				char *item = strip_quotes(strip(token));
				if (*item) {
					cfg->can_be_swallowed[i] = malloc(2 * sizeof(char *));
					if (!cfg->can_be_swallowed[i]) {
						break;
					}
					cfg->can_be_swallowed[i][0] = strdup(item);
					cfg->can_be_swallowed[i][1] = NULL;
					i++;
				}
				token = strtok(NULL, ",");
			}
		}
		else if (!strcmp(key, "new_win_master")) {
			cfg->new_win_master = !strcmp(rest, "true") ? True : False;
		}
		else if (!strcmp(key, "open_in_workspace")) {
			char *mid = strchr(rest, ':');
			if (!mid) {
				fprintf(stderr, "sxwmrc:%d: open_in_workspace missing workspace number\n", lineno);
				continue;
			}
			*mid = '\0';
			char *class_name = strip(rest);
			char *ws_str = strip(mid + 1);

			class_name = strip_quotes(class_name);

			int ws = atoi(ws_str);
			if (ws < 1 || ws > NUM_WORKSPACES) {
				fprintf(stderr, "sxwmrc:%d: invalid workspace number %d\n", lineno, ws);
				continue;
			}

			/* find free slot in open_in_workspace */
			int slot = -1;
			for (int i = 0; i < 256; i++) {
				if (!cfg->open_in_workspace[i]) {
					slot = i;
					break;
				}
			}

			if (slot >= 0) {
				cfg->open_in_workspace[slot] = malloc(2 * sizeof(char *));
				if (cfg->open_in_workspace[slot]) {
					cfg->open_in_workspace[slot][0] = strdup(class_name); /* class name */
					char ws_buf[16];
					snprintf(ws_buf, sizeof(ws_buf), "%d", ws - 1);   /* 0-indexed workspace */
					cfg->open_in_workspace[slot][1] = strdup(ws_buf); /* workspace number */
				}
			}
		}
		else if (!strcmp(key, "start_fullscreen")) {
			char *comment = strchr(rest, '#');
			size_t len = comment ? (size_t)(comment - rest) : strlen(rest);
			if (len >= sizeof(line)) {
				len = sizeof(line) - 1;
			}

			char win[sizeof(line)];
			snprintf(win, sizeof(win), "%.*s", (int)len, rest);

			char *final = strip(win);
			char *comma_ptr;
			char *comma = strtok_r(final, ",", &comma_ptr);

			int start_fullscreen_idx = 0;
			/* find first empty slot */
			for (int i = 0; i < 256; i++) {
				if (!cfg->start_fullscreen[i]) {
					start_fullscreen_idx = i;
					break;
				}
			}

			/* store each comma separated value in a separate row */
			while (comma && start_fullscreen_idx < 256) {
				comma = strip(comma);
				if (*comma == '"') {
					comma++;
				}
				char *end = comma + strlen(comma) - 1;
				if (*end == '"') {
					*end = '\0';
				}

				char *dup = strdup(comma);
				if (!dup) {
					fprintf(stderr, "sxwmrc:%d: failed to allocate memory\n", lineno);
					goto cleanup_file;
				}

				cfg->start_fullscreen[start_fullscreen_idx] = malloc(2 * sizeof(char *));
				if (!cfg->start_fullscreen[start_fullscreen_idx]) {
					free(dup);
					fprintf(stderr, "sxwmrc:%d: failed to allocate memory\n", lineno);
					goto cleanup_file;
				}

				/* store each program's name in its own row at index 0 */
				cfg->start_fullscreen[start_fullscreen_idx][0] = dup;
				cfg->start_fullscreen[start_fullscreen_idx][1] = NULL;
				start_fullscreen_idx++;
				comma = strtok_r(NULL, ",", &comma_ptr);
			}
		}
		else {
			fprintf(stderr, "sxwmrc:%d: unknown option '%s'\n", lineno, key);
		}
	}

	fclose(f);
	remap_and_dedupe_binds(cfg);
	return 0;

cleanup_file:
	if (f) {
		fclose(f);
	}
	for (int j = 0; j < 256; j++) {
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
	for (int i = 0; i < torun; i++) {
		free(cfg->torun[i]);
	}
	return -1;
}

int parse_mods(const char *mods, Config *cfg)
{
	KeySym dummy;
	return parse_combo(mods, cfg, &dummy);
}

KeySym parse_keysym(const char *key)
{
	KeySym ks = XStringToKeysym(key);
	if (ks != NoSymbol) {
		return ks;
	}

	char buf[64];
	size_t n = strlen(key);
	if (n >= sizeof buf) {
		n = sizeof buf - 1;
	}

	buf[0] = toupper((unsigned char)key[0]);
	for (size_t i = 1; i < n; i++) {
		buf[i] = tolower((unsigned char)key[i]);
	}
	buf[n] = '\0';
	ks = XStringToKeysym(buf);
	if (ks != NoSymbol) {
		return ks;
	}

	for (size_t i = 0; i < n; i++) {
		buf[i] = toupper((unsigned char)key[i]);
	}
	buf[n] = '\0';
	ks = XStringToKeysym(buf);
	if (ks != NoSymbol) {
		return ks;
	}

	fprintf(stderr, "sxwmrc: unknown keysym '%s'\n", key);
	return NoSymbol;
}

#ifndef __linux__
char **split_cmd(const char *cmd, int *out_argc)
{
	enum { NORMAL, IN_QUOTE } state = NORMAL;
	const char *p = cmd;
	size_t cap = 8, argc = 0, toklen = 0;
	char *token = malloc(strlen(cmd) + 1);
	char **argv = malloc(cap * sizeof *argv);
	if (!token || !argv) {
		goto err;
	}

	while (*p) {
		if (state == NORMAL && isspace((unsigned char)*p)) {
			if (toklen) {
				token[toklen] = '\0';
				if (argc + 1 >= cap) {
					cap *= 2;
					char **new_argv = realloc(argv, cap * sizeof *argv);
					if (!new_argv) {
						goto err;
					}
					argv = new_argv;
				}
				argv[argc++] = strdup(token);
				toklen = 0;
			}
		}
		else if (*p == '"') {
			state = (state == NORMAL) ? IN_QUOTE : NORMAL;
		}
		else {
			token[toklen++] = *p;
		}
		p++;
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
	if (token) {
		free(token);
	}
	if (argv) {
		for (size_t i = 0; i < argc; i++) {
			free(argv[i]);
		}
		free(argv);
	}
	return NULL;
}
#endif

const char **build_argv(const char *cmd)
{
#ifdef __linux__
	wordexp_t p;
	if (wordexp(cmd, &p, 0) != 0 || p.we_wordc == 0) {
		fprintf(stderr, "sxwm: wordexp failed for cmd: '%s'\n", cmd);
		return NULL;
	}

	const char **argv = malloc((p.we_wordc + 1) * sizeof *argv);
	if (!argv) {
		wordfree(&p);
		return NULL;
	}

	for (size_t i = 0; i < p.we_wordc; i++) {
		argv[i] = strdup(p.we_wordv[i]);
	}
	argv[p.we_wordc] = NULL;
	wordfree(&p);
	return argv;

#else
	int argc = 0;
	char **tmp = split_cmd(cmd, &argc);
	if (!tmp) {
		return NULL;
	}

	const char **argv = malloc((argc + 1) * sizeof *argv);
	if (!argv) {
		for (int i = 0; i < argc; i++) {
			free(tmp[i]);
		}
		free(tmp);
		return NULL;
	}

	for (int i = 0; i < argc; i++) {
		argv[i] = tmp[i];
	}
	argv[argc] = NULL;
	free(tmp);
	return argv;
#endif
}
