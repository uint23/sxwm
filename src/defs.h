#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <X11/Xlib.h>

#define SXWM_VERSION	"sxwm ver. 0.1.3"
#define SXWM_AUTHOR		"(C) Abhinav Prasai 2025"
#define SXWM_LICINFO	"See LICENSE for more info"

#define ALT		Mod1Mask
#define SUPER	Mod4Mask
#define SHIFT	ShiftMask

#define MARGIN (gaps + BORDER_WIDTH)
#define OUT_IN (2 * BORDER_WIDTH)
#define LENGTH(X) (sizeof X / sizeof X[0])
#define BIND(mod, key, cmdstr) { (mod), XK_##key, { cmdstr }, 0 }
#define CALL(mod, key, fnptr) { (mod), XK_##key, { .fn = fnptr }, 1 }
#define CMD(name, ...) \
	static const char *name[] = { __VA_ARGS__, NULL }

#define UDIST(a,b) abs((int)(a) - (int)(b))
#define MAXCLIENTS	64
#define MAXGAPS		100

enum { DRAG_NONE, DRAG_MOVE, DRAG_RESIZE } drag_mode = DRAG_NONE;
typedef void (*EventHandler)(XEvent *);

typedef union {
	const char **cmd;
	void (*fn)(void);
} Action;

typedef struct {
	unsigned int mods;
	KeySym keysym;
	Action action;
	int is_func;
} Binding;

typedef struct Client{
	Window win;
	uint x, y, h, w;
	Bool floating;
	struct Client *next;
} Client;

#endif
