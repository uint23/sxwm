/* See LICENSE for more information on use */
#define _POSIX_C_SOURCE 200809L /* for strdup */
#include <ctype.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "parser.h"
#define MAX_LINE 512

static const struct { const char *name; void (*fn)(void); } call_table[] = {
	{ "close_window",		close_focused },
	{ "decrease_gaps",		dec_gaps },
	{ "focus_next",			focus_next },
	{ "focus_previous",		focus_prev },
	{ "increase_gaps",		inc_gaps },
	{ "master_next",		move_master_next },
	{ "master_previous",	move_master_prev },
	{ "quit",				quit },
	{ "reload_config",		reload_config },
	{ "master_increase",	resize_master_add },
	{ "master_decrease",	resize_master_sub },
	{ "toggle_floating",	toggle_floating },
	{ "global_floating",	toggle_floating_global },
	{ "fullscreen",			toggle_fullscreen },
	{ NULL, NULL }
};

/* helpers */
static char*
strip(char *s)
{
	while (isspace((unsigned char)*s)) s++;
	if (!*s) return s;
	char *end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
	return s;
}

static char*
strip_quotes(char *s)
{
	size_t len = strlen(s);
	if (len > 0 && s[0] == '"') {
		s++;
		len--;
	}
	if (len > 0 && s[len-1] == '"') {
		s[len-1] = '\0';
	}
	return s;
}

static const char** build_argv(char *cmdline)
{
	char **argv = calloc(MAX_ARGS + 1, sizeof(char *));
	if (!argv) return NULL;

	int argc = 0;
	char *tok = strtok(cmdline, " \t");
	while (tok && argc < MAX_ARGS) {
		argv[argc++] = strdup(tok);
		tok = strtok(NULL, " \t");
	}
	argv[argc] = NULL;
	return (const char **)argv;
}

unsigned int
parse_mods(const char *mods, Config *user_config)
{
	unsigned int m = 0;
	char buf[MAX_LINE];
	strncpy(buf, mods, sizeof(buf)-1);
	buf[sizeof(buf)-1] = '\0';

	for (char *tok = strtok(buf, "+"); tok; tok = strtok(NULL, "+")) {
		for (char *p = tok; *p; p++) *p = tolower((unsigned char)*p);
		if (strcmp(tok, "mod") == 0) m |= user_config->modkey;
		else if (strcmp(tok, "shift") == 0)		m |= ShiftMask;
		else if (strcmp(tok, "ctrl") == 0)		m |= ControlMask;
		else if (strcmp(tok, "alt") == 0) 		m |= Mod1Mask;
		else if (strcmp(tok, "super") == 0) 	m |= Mod4Mask;
	}
	return m;
}

KeySym
parse_keysym(const char *key)
{
	char buf[64];
	size_t len = strlen(key);
	if (len >= sizeof(buf)) len = sizeof(buf) - 1;

	if (len == 1) {
		buf[0] = key[0];
		buf[1] = '\0';
	} else {
		buf[0] = toupper((unsigned char)key[0]);
		for (size_t i = 1; i < len; i++)
			buf[i] = tolower((unsigned char)key[i]);
		buf[len] = '\0';
	}

	KeySym ks = XStringToKeysym(buf);
	if (ks == NoSymbol) {
		for (size_t i = 0; i < len; i++)
			buf[i] = toupper((unsigned char)key[i]);
		buf[len] = '\0';
		ks = XStringToKeysym(buf);
	}
	if (ks == NoSymbol)
		fprintf(stderr, "sxwmrc: unknown keysym '%s'\n", key);
	return ks;
}

void
handler(char *command, char *arg, int mods, KeySym keysym, Action action,
		Bool is_func, Config *user_config)
{
	if (strcmp(command, "bind") == 0) {
		/* check if binding already exists */
		int slot = user_config->bindsn;
		for (int i = 0; i < user_config->bindsn; i++) {
			if (user_config->binds[i].mods == mods &&
					user_config->binds[i].keysym == keysym) {
				slot = i;
				break;
			}
		}

		if (slot == user_config->bindsn)
			user_config->bindsn++;

		Binding *b = &user_config->binds[slot];
		b->mods = mods;
		b->keysym = keysym;
		b->is_func = is_func;

		if (is_func) {
			for (size_t j = 0; call_table[j].name; j++) {
				if (strcmp(arg, call_table[j].name) == 0) {
					b->action.fn = call_table[j].fn;
					break;
				}
			}
		} else {
			b->action.cmd = action.cmd;
		}
		return;
	}

	if (strcmp(command, "gaps") == 0)							user_config->gaps = atoi(arg);
	else if (strcmp(command, "border_width") == 0)				user_config->border_width = atoi(arg);
	else if (strcmp(command, "focused_border_colour") == 0)		user_config->border_foc_col = parse_col(arg);
	else if (strcmp(command, "unfocused_border_colour") == 0)	user_config->border_ufoc_col = parse_col(arg);
	else if (strcmp(command, "swap_border_colour") == 0)		user_config->border_swap_col = parse_col(arg);
	else if (strcmp(command, "master_coverage") == 0)			user_config->master_width = atoi(arg);
	else if (strcmp(command, "resize_master_amount") == 0)		user_config->resize_master_amt = atoi(arg);
	else if (strcmp(command, "snap_distance") == 0)				user_config->snap_distance = atoi(arg);
	else if (strcmp(command, "mod_key") == 0)					user_config->modkey = parse_mods(arg, user_config);
	else
		fprintf(stderr, "sxwmrc: unknown setting '%s'\n", command);
}
int
parser(Config *user_config)
{
	char *home = getenv("HOME");
	if (!home) {
		fputs("sxwmrc: HOME not set\n", stderr);
		return -1;
	}

	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/.config/sxwmrc", home);
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "sxwmrc: cannot open '%s'\n", path);
		return -1;
	}

	char line[MAX_LINE];
	int lineno = 0;
	while (fgets(line, sizeof(line), f)) {
		lineno++;
		char *s = strip(line);
		if (!*s || *s == '#') continue;

		/* split key : value */
		char *sep = strchr(s, ':');
		if (!sep) {
			fprintf(stderr, "sxwmrc:%d: missing ':'\n", lineno);
			continue;
		}
		*sep = '\0';
		char *key = strip(s);
		char *val = strip(sep + 1);

		if (strcmp(key, "bind") == 0) {
			/* bind parsing */
			char *mid = strchr(val, ':');
			if (!mid) {
				fprintf(stderr, "sxwmrc:%d: bind missing action\n", lineno);
				continue;
			}
			*mid = '\0';
			char *combo = strip(val);
			char *act = strip(mid + 1);

			if (*combo == '[') {
				combo++;
			}
			size_t L = strlen(combo);
			if (L && combo[L-1] == ']') {
				combo[L-1] = '\0';
			}

			/* parse mods & key */
			unsigned int mods = 0;
			char key_part[64] = {0};
			char tokbuf[MAX_LINE];
			strncpy(tokbuf, combo, sizeof(tokbuf)-1);
			tokbuf[sizeof(tokbuf)-1] = '\0';

			for (char *tok = strtok(tokbuf, "+"); tok; tok = strtok(NULL, "+")) {
				char *p = strip(tok);
				for (char *c = p; *c; c++) *c = tolower((unsigned char)*c);
				if (strcmp(p, "mod") == 0) mods |= user_config->modkey;
				else if (strcmp(p, "shift") == 0) mods |= ShiftMask;
				else if (strcmp(p, "ctrl") == 0) mods |= ControlMask;
				else if (strcmp(p, "alt") == 0) mods |= Mod1Mask;
				else if (strcmp(p, "super") == 0) mods |= Mod4Mask;
				else strncpy(key_part, p, sizeof(key_part)-1);
			}

			KeySym ks = parse_keysym(key_part);
			if (!ks) continue;

			Action a = { .cmd = NULL };
			Bool is_fn = False;
			if (*act == '"') {
				char act_buf[MAX_LINE];
				strncpy(act_buf, strip_quotes(act), sizeof(act_buf)-1);
				act_buf[sizeof(act_buf)-1] = '\0';
				a.cmd = build_argv(act_buf);
				fprintf(stderr, "[DEBUG parser] Parsed bind: mods=0x%x, keysym=0x%lx, cmd='%s'\n", mods, ks, act);
			} else {
				is_fn = True;
				fprintf(stderr, "[DEBUG parser] Parsed bind: mods=0x%x, keysym=0x%lx, func='%s'\n", mods, ks, act);
			}

			handler("bind", act, mods, ks, a, is_fn, user_config);

		} else {
			/* normal settings */
			handler(key, val, 0, 0, (Action){ .cmd = NULL }, False, user_config);
		}
	}

	fclose(f);
	return 0;
}
