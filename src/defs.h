#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <X11/Xlib.h>

#define SXWM_VERSION	"sxwm ver. 0.1.1"
#define SXWM_AUTHOR		"(C) Abhinav Prasai 2025"
#define SXWM_LICINFO	"See LICENSE for more info"

#define ALT		Mod1Mask
#define SUPER	Mod4Mask
#define SHIFT	ShiftMask

#define LENGTH(X) (sizeof X / sizeof X[0])
#define BIND(mod, key, cmdstr)  { (mod), XK_##key, { cmdstr }, 0 }
#define CALL(mod, key, fnptr)	{ (mod), XK_##key, { .fn = fnptr }, 1 }

#define UDIST(a,b) abs((int)(a) - (int)(b))
#define SNAP_EDGE(pos, size, bound)								\
	do {														\
		if (UDIST(pos, 0) < SNAP_DISTANCE)						\
		pos = 0;												\
		else if (UDIST(pos, (bound) - (size)) < SNAP_DISTANCE)	\
		pos = (bound) - (size);									\
	} while (0)

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
