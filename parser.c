#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "common.h"
#include "extern.h"
#include "parser.h"
#include "utils.h"

typedef enum { OPT_INT, OPT_BOOL, OPT_COLOR } OptionType;

typedef struct {
	const char* name;
	size_t offset;
	OptionType type;
} Option;

static Binding* alloc_bind(Config* cfg, Binding bind);
static int append_rules(char** rules, char* value);
static FILE* open_config(char* path, size_t size);
static void parse_binding(Config* cfg, const char* key, char* value, int line);
static unsigned parse_combo(const char* combo, Config* cfg, KeySym* keysym);
static KeySym parse_keysym(const char* key);
static char* strip(char* s);
static char* strip_comment(char* s);
static char* strip_quotes(char* s);

static const CommandEntry call_table[] = {
	{ "centre_window",             centre_window },
	{ "close_window",              close_focused },
	{ "decrease_gaps",             dec_gaps },
	{ "focus_next",                focus_next },
	{ "focus_prev",                focus_prev },
	{ "focus_next_mon",            focus_next_mon },
	{ "focus_prev_mon",            focus_prev_mon },
	{ "fullscreen",                toggle_fullscreen },
	{ "global_floating",           toggle_floating_global },
	{ "increase_gaps",             inc_gaps },
	{ "master_next",               move_master_next },
	{ "master_prev",               move_master_prev },
	{ "master_increase",           resize_master_add },
	{ "master_decrease",           resize_master_sub },
	{ "move_next_mon",             move_next_mon },
	{ "move_prev_mon",             move_prev_mon },
	{ "move_win_up",               move_win_up },
	{ "move_win_down",             move_win_down },
	{ "move_win_left",             move_win_left },
	{ "move_win_right",            move_win_right },
	{ "quit",                      quit },
	{ "reload_config",             reload_config },
	{ "resize_win_up",             resize_win_up },
	{ "resize_win_down",           resize_win_down },
	{ "resize_win_left",           resize_win_left },
	{ "resize_win_right",          resize_win_right },
	{ "switch_previous_workspace", switch_previous_workspace },
	{ "toggle_floating",           toggle_floating },
	{ NULL, NULL },
};

static const Option options[] = {
	{ "border_width",             offsetof(Config, border_width), OPT_INT },
	{ "floating_on_top",          offsetof(Config, floating_on_top), OPT_BOOL },
	{ "focused_border_colour",    offsetof(Config, border_foc_col), OPT_COLOR },
	{ "gaps",                     offsetof(Config, gaps), OPT_INT },
	{ "motion_throttle",          offsetof(Config, motion_throttle), OPT_INT },
	{ "move_window_amount",       offsetof(Config, move_window_amt), OPT_INT },
	{ "new_win_focus",            offsetof(Config, new_win_focus), OPT_BOOL },
	{ "new_win_master",           offsetof(Config, new_win_master), OPT_BOOL },
	{ "resize_master_amount",     offsetof(Config, resize_master_amt), OPT_INT },
	{ "resize_window_amount",     offsetof(Config, resize_window_amt), OPT_INT },
	{ "snap_distance",            offsetof(Config, snap_distance), OPT_INT },
	{ "unfocused_border_colour",  offsetof(Config, border_ufoc_col), OPT_COLOR },
	{ "warp_cursor",              offsetof(Config, warp_cursor), OPT_BOOL },
};

static Binding* alloc_bind(Config* cfg, Binding bind)
{
	for (int i = 0; i < cfg->n_binds; i++) {
		Binding* b = &cfg->binds[i];
		if (b->mods == bind.mods && b->keysym == bind.keysym) {
			if (b->type == TYPE_CMD)
				free(b->action.cmd);
			*b = bind;
			return b;
		}
	}

	if (cfg->n_binds >= MAX_BINDS)
		return NULL;

	Binding* b = &cfg->binds[cfg->n_binds++];
	*b = bind;
	return b;
}

static int append_rules(char** rules, char* value)
{
	char* save;
	for (char* item = strtok_r(value, ",", &save); item; item = strtok_r(NULL, ",", &save)) {
		item = strip_quotes(strip(item));
		if (!*item)
			continue;

		int i = 0;
		while (i < MAX_ITEMS && rules[i])
			i++;
		if (i == MAX_ITEMS || !(rules[i] = strdup(item)))
			return -1;
	}
	return 0;
}

void free_config(Config* cfg)
{
	for (int i = 0; i < cfg->n_binds; i++)
		if (cfg->binds[i].type == TYPE_CMD)
			free(cfg->binds[i].action.cmd);

	for (int i = 0; i < MAX_ITEMS; i++) {
		free(cfg->to_run[i]);
		free(cfg->should_float[i]);
		free(cfg->start_fullscreen[i]);
		free(cfg->open_in_workspace[i].name);
	}
	memset(cfg, 0, sizeof(*cfg));
}

static FILE* open_config(char* path, size_t size)
{
	const char* home = getenv("HOME");
	const char* xdg = getenv("XDG_CONFIG_HOME");
	const char* paths[] = { "%s/sxwmrc", "%s/sxwm/sxwmrc" };

	if (xdg) {
		for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
			snprintf(path, size, paths[i], xdg);
			if (access(path, R_OK) == 0)
				goto found;
		}
	}
	if (!home) {
		wlog("HOME not set");
		return NULL;
	}

	snprintf(path, size, "%s/.config/sxwmrc", home);
	if (access(path, R_OK) == 0)
		goto found;

	snprintf(path, size, "/usr/local/share/sxwmrc");
	if (access(path, R_OK) == 0)
		goto found;

	wlog("No configuration file found");
	return NULL;

found:;
	FILE* f = fopen(path, "r");
	if (!f)
		wlog("Cannot open configuration %s: %s", path, strerror(errno));
	else
		wlog("Using configuration file %s", path);
	return f;
}

static void parse_binding(Config* cfg, const char* key, char* value, int line)
{
	char* action = strchr(value, ':');
	if (!action) {
		wlog("config:%d: %s missing action", line, key);
		return;
	}
	*action++ = '\0';
	action = strip(action);

	KeySym keysym;
	Binding bind = { 0 };
	bind.mods = parse_combo(strip(value), cfg, &keysym);
	if (keysym == NoSymbol) {
		wlog("config:%d: bad key in '%s'", line, value);
		return;
	}
	bind.keysym = keysym;

	if (!strcmp(key, "workspace")) {
		int ws;
		if (sscanf(action, "move %d", &ws) == 1)
			bind.type = TYPE_WS_CHANGE;
		else if (sscanf(action, "swap %d", &ws) == 1)
			bind.type = TYPE_WS_MOVE;
		else
			ws = 0;
		if (ws < 1 || ws > NUM_WORKSPACES) {
			wlog("config:%d: invalid workspace action '%s'", line, action);
			return;
		}
		bind.action.ws = ws - 1;
	}
	else if (!strcmp(key, "bind") && *action == '"') {
		bind.type = TYPE_CMD;
		bind.action.cmd = strdup(strip_quotes(action));
		if (!bind.action.cmd) {
			wlog("config:%d: failed to parse command '%s'", line, action);
			return;
		}
	}
	else {
		bind.type = TYPE_FUNC;
		for (int i = 0; call_table[i].name; i++)
			if (!strcmp(action, call_table[i].name)) {
				bind.action.fn = call_table[i].fn;
				break;
			}
		if (!bind.action.fn) {
			wlog("config:%d: unknown function '%s'", line, action);
			return;
		}
	}

	if (!alloc_bind(cfg, bind)) {
		if (bind.type == TYPE_CMD)
			free(bind.action.cmd);
		wlog("config:%d: too many key bindings", line);
	}
}

static unsigned parse_combo(const char* combo, Config* cfg, KeySym* keysym)
{
	unsigned mods = 0;
	char buf[MAX_ITEMS];
	snprintf(buf, sizeof(buf), "%s", combo);
	for (char* p = buf; *p; p++)
		if (*p == '+' || isspace((unsigned char)*p))
			*p = '+';

	*keysym = NoSymbol;
	char* save;
	for (char* tok = strtok_r(buf, "+", &save); tok; tok = strtok_r(NULL, "+", &save)) {
		if (!strcmp(tok, "mod")) mods |= cfg->modkey;
		else if (!strcmp(tok, "shift")) mods |= ShiftMask;
		else if (!strcmp(tok, "ctrl")) mods |= ControlMask;
		else if (!strcmp(tok, "alt")) mods |= Mod1Mask;
		else if (!strcmp(tok, "super")) mods |= Mod4Mask;
		else *keysym = parse_keysym(tok);
	}
	return mods;
}

static KeySym parse_keysym(const char* key)
{
	KeySym ks = XStringToKeysym(key);
	if (ks != NoSymbol || !*key)
		return ks;

	char buf[64];
	size_t n = strlen(key);
	if (n >= sizeof(buf))
		n = sizeof(buf) - 1;

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

	wlog("Unknown keysym '%s'", key);
	return NoSymbol;
}

int parse(Config* cfg)
{
	char path[PATH_MAX];
	FILE* f = open_config(path, sizeof(path));
	if (!f)
		return -1;

	char line[512];
	int lineno = 0, n_exec = 0;
	while (fgets(line, sizeof(line), f)) {
		lineno++;
		char* s = strip(line);
		if (!*s || *s == '#')
			continue;

		char* sep = strchr(s, ':');
		if (!sep) {
			wlog("config:%d: missing ':'", lineno);
			continue;
		}
		*sep++ = '\0';
		char* key = strip(s);
		char* value = strip(sep);

		char** rules = NULL;
		if (!strcmp(key, "should_float")) rules = cfg->should_float;
		else if (!strcmp(key, "start_fullscreen")) rules = cfg->start_fullscreen;

		if (rules) {
			if (append_rules(rules, strip_comment(value)) < 0)
				goto error;
		}
		else if (!strcmp(key, "bind") || !strcmp(key, "call") || !strcmp(key, "workspace"))
			parse_binding(cfg, key, value, lineno);
		else if (!strcmp(key, "exec")) {
			if (n_exec >= MAX_ITEMS) {
				wlog("config:%d: too many exec commands", lineno);
				continue;
			}
			value = strip_quotes(strip_comment(value));
			if (!*value) {
				wlog("config:%d: empty exec command", lineno);
				continue;
			}
			if (!(cfg->to_run[n_exec++] = strdup(value)))
				goto error;
		}
		else if (!strcmp(key, "open_in_workspace")) {
			char* mid = strchr(value, ':');
			if (!mid) {
				wlog("config:%d: open_in_workspace missing workspace", lineno);
				continue;
			}
			*mid++ = '\0';
			int ws = atoi(strip(mid));
			if (ws < 1 || ws > NUM_WORKSPACES) {
				wlog("config:%d: invalid workspace number %d", lineno, ws);
				continue;
			}
			int slot = 0;
			while (slot < MAX_ITEMS && cfg->open_in_workspace[slot].name)
				slot++;
			if (slot == MAX_ITEMS || !(cfg->open_in_workspace[slot].name =
			    strdup(strip_quotes(strip(value)))))
				goto error;
			cfg->open_in_workspace[slot].workspace = ws - 1;
		}
		else if (!strcmp(key, "master_width")) {
			float fraction = (float)atoi(value) / 100.0f;
			for (int i = 0; i < MAX_MONITORS; i++)
				cfg->master_width[i] = fraction;
		}
		else if (!strcmp(key, "mod_key")) {
			KeySym sym;
			unsigned mods = parse_combo(value, cfg, &sym);
			if (mods & (Mod1Mask | Mod4Mask | ShiftMask | ControlMask))
				cfg->modkey = mods;
			else
				wlog("config:%d: unknown mod_key '%s'", lineno, value);
		}
		else {
			size_t i;
			for (i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
				if (strcmp(key, options[i].name))
					continue;
				char* field = (char*)cfg + options[i].offset;
				switch (options[i].type) {
				case OPT_INT: *(int*)field = atoi(value); break;
				case OPT_BOOL: *(Bool*)field = !strcmp(value, "true"); break;
				case OPT_COLOR: *(long*)field = parse_col(value); break;
				}
				break;
			}
			if (i == sizeof(options) / sizeof(options[0]))
				wlog("config:%d: unknown option '%s'", lineno, key);
		}
	}
	fclose(f);
	return 0;

error:
	wlog("config:%d: out of memory or rule limit exceeded", lineno);
	fclose(f);
	free_config(cfg);
	return -1;
}

static char* strip(char* s)
{
	while (*s && isspace((unsigned char)*s))
		s++;
	if (!*s)
		return s;
	char* end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end))
		*end-- = '\0';
	return s;
}

static char* strip_comment(char* s)
{
	char* c = strchr(s, '#');
	if (c)
		*c = '\0';
	return strip(s);
}

static char* strip_quotes(char* s)
{
	size_t len = strlen(s);
	if (len && s[0] == '"') {
		s++;
		len--;
	}
	if (len && s[len - 1] == '"')
		s[len - 1] = '\0';
	return s;
}

