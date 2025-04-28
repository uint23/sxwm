#include <ctype.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "parser.h"

static struct { const char *n; void (*fn)(void); } call_table[] = {
	{"close_window",		close_focused},
	{"decrease_gaps",		dec_gaps},
	{"focus_next",			focus_next},
	{"focus_previous",		focus_prev},
	{"increase_gaps",		inc_gaps},
	{"master_next",			move_master_next},
	{"master_previous",		move_master_prev},
	{"quit",				quit},
	{"reload_config",		reload_config},
	{"master_increase",		resize_master_add},
	{"master_decrease",		resize_master_sub},
	{"floating",			toggle_floating},
	{"global_floating",		toggle_floating_global},
	{"fullscreen",			toggle_fullscreen},
	{ NULL, NULL }
};

void
handler(char *command, char *arg,
		unsigned int mods, KeySym keysym,
		Action action, Bool is_func,
		Config *user_config)
{
	if (strcmp(command, "bind") == 0) {
		if (is_func) {
			/* look up function by name */
			for (unsigned long i = 0; i < LENGTH(call_table); ++i) {
				if (strcmp(arg, call_table[i].n) == 0) {
					user_config->binds[user_config->bindsn].action.fn = call_table[i].fn;
					user_config->binds[user_config->bindsn].is_func = True;
					user_config->binds[user_config->bindsn].mods = mods;
					user_config->binds[user_config->bindsn].keysym = keysym;
					++user_config->bindsn;
					return;
				}
			}
			fprintf(stderr, "sxwmrc: unknown function call: %s\n", arg);
		} else {
			user_config->binds[user_config->bindsn].action.cmd = action.cmd;
			user_config->binds[user_config->bindsn].is_func = False;
			user_config->binds[user_config->bindsn].mods = mods;
			user_config->binds[user_config->bindsn].keysym = keysym;
			++user_config->bindsn;
		}
		return;
	}

	/* TODO USE A TABLE */
	if (strcmp(command, "gaps") == 0) user_config->gaps = atoi(arg);
	else if (strcmp(command, "border_width") == 0) user_config->border_width = atoi(arg);
	else if (strcmp(command, "focus_border_colour") == 0) user_config->border_foc_col = parse_col(arg);
	else if (strcmp(command, "unfocus_border_colour")== 0) user_config->border_ufoc_col = parse_col(arg);
	else if (strcmp(command, "swap_border_colour") == 0) user_config->border_swap_col = parse_col(arg);
	else if (strcmp(command, "master_coverage") == 0) user_config->master_width = atoi(arg);
	else if (strcmp(command, "resize_master_amount") == 0) user_config->resize_master_amt = atoi(arg);
	else if (strcmp(command, "snap_distance") == 0) user_config->snap_distance = atoi(arg);
	else
		fprintf(stderr, "sxwmrc: unknown command: %s\n", command);
}

void
parser(Config *user_config)
{
	char *home = getenv("HOME");
	if (!home) {
		fputs("sxwmrc: HOME environment variable not set\n", stderr);
		return;
	}
	char sxwmrc[PATH_MAX];
	snprintf(sxwmrc, sizeof(sxwmrc), "%s/.config/sxwmrc", home);

	FILE *f = fopen(sxwmrc, "r");
	if (!f) {
		fprintf(stderr, "sxwmrc: file not found: %s\n", sxwmrc);
		return;
	}

	char line[512];
	while (fgets(line, sizeof(line), f)) {
		char *s = strip(line);
		if (!*s || *s == '#' || *s == '\n')
			continue;

		char *c = strchr(s, ':');
		if (!c) {
			fprintf(stderr, "sxwmrc: invalid line (no colon): %s\n", s);
			continue;
		}
		*c = '\0';
		char *key = strip(s);
		char *val = strip(c+1);

		if (strcmp(key, "bind") == 0) {
			char *d = strchr(val, ':');
			if (!d) {
				fprintf(stderr, "sxwmrc: invalid bind (no action): %s\n", val);
				continue;
			}
			*d = '\0';
			char *combo = strip(val);
			char *act = strip(d+1);

			if (*combo == '[') combo++;
			size_t L = strlen(combo);
			if (L && combo[L-1] == ']') combo[L-1] = '\0';

			unsigned int mods = 0;
			KeySym ks = 0;
			char part[64];
			char *tok = strtok(combo, "+");
			while (tok) {
				char *p = strip(tok);
				for (size_t i=0; p[i]; i++) p[i] = tolower((unsigned char)p[i]);
				if (strcmp(p,"mod") == 0) mods |= user_config->modkey;
				else if (strcmp(p,"shift")==0) mods |= ShiftMask;
				else if (strcmp(p,"ctrl")==0) mods |= ControlMask;
				else if (strcmp(p,"alt")==0) mods |= Mod1Mask;
				else {
					strncpy(part, tok, sizeof(part)-1);
					part[sizeof(part)-1] = '\0';
					ks = parse_keysym(strip(part));
				}
				tok = strtok(NULL, "+");
			}

			Action a = { .cmd = NULL };
			Bool is_fn = False;
			if (*act == '"') {
				char *cmdstr = strip_quotes(act);
				a.cmd = build_argv(cmdstr);
			} else {
				is_fn = True;
			}
			handler(key, act, mods, ks, a, is_fn, user_config);

		} else {
			handler(key, val, 0, 0, (Action){0}, False, user_config);
		}
	}

	fclose(f);
}

char *
strip(char *str)
{
	while (*str == ' ' || *str == '\t') str++;
	char *end = str + strlen(str) - 1;
	while (end > str && (*end==' '||*end=='\t'||*end=='\r'||*end=='\n'))
		*end-- = '\0';
	return str;
}

	char *
strip_quotes(char *s)
{
	if (*s == '"') s++;
	size_t n = strlen(s);
	if (n && s[n-1] == '"') s[n-1] = '\0';
	return s;
}

unsigned int
parse_mods(const char *mods, Config *user_config)
{
	unsigned int m = 0;
	if (strstr(mods, "mod")) m |= user_config->modkey;
	if (strstr(mods, "shift")) m |= ShiftMask;
	if (strstr(mods, "ctrl")) m |= ControlMask;
	if (strstr(mods, "alt")) m |= Mod1Mask;
	return m;
}

KeySym
parse_keysym(const char *key)
{
	KeySym ks = XStringToKeysym(key);
	if (ks == NoSymbol) {
		fprintf(stderr, "sxwmrc: unknown keysym: %s\n", key);
		return 0;
	}
	return ks;
}

static const char **
build_argv(char *cmdline)
{
	char **argv = calloc(MAX_ARGS+1, sizeof(char *));
	if (!argv) return NULL;

	int argc = 0;
	char *tok = strtok(cmdline, " \t");
	while (tok && argc < MAX_ARGS) {
		argv[argc++] = tok;
		tok = strtok(NULL, " \t");
	}
	argv[argc] = NULL;
	return (const char **)argv;
}
