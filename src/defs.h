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
#define BIND(mod, key, cmdstr) { (mod), XK_##key, { cmdstr }, False }
#define CALL(mod, key, fnptr) { (mod), XK_##key, { .fn = fnptr }, True }
#define CMD(name, ...) 						\
	static const char *name[] = { __VA_ARGS__, NULL }

static void change_ws1(void);
static void moveto_ws1(void);
static void change_ws2(void);
static void moveto_ws2(void);
static void change_ws3(void);
static void moveto_ws3(void);
static void change_ws4(void);
static void moveto_ws4(void);
static void change_ws5(void);
static void moveto_ws5(void);
static void change_ws6(void);
static void moveto_ws6(void);
static void change_ws7(void);
static void moveto_ws7(void);
static void change_ws8(void);
static void moveto_ws8(void);
static void change_ws9(void);
static void moveto_ws9(void);

#define INIT_WORKSPACE \
static void change_ws1(void) { change_workspace(0); }	\
static void moveto_ws1(void) { move_to_workspace(0); }	\
static void change_ws2(void) { change_workspace(1); }	\
static void moveto_ws2(void) { move_to_workspace(1); }	\
static void change_ws3(void) { change_workspace(2); }	\
static void moveto_ws3(void) { move_to_workspace(2); }	\
static void change_ws4(void) { change_workspace(3); }	\
static void moveto_ws4(void) { move_to_workspace(3); }	\
static void change_ws5(void) { change_workspace(4); }	\
static void moveto_ws5(void) { move_to_workspace(4); }	\
static void moveto_ws6(void) { move_to_workspace(5); }	\
static void change_ws6(void) { change_workspace(5); }	\
static void moveto_ws7(void) { move_to_workspace(6); }	\
static void change_ws7(void) { change_workspace(6); }	\
static void moveto_ws8(void) { move_to_workspace(7); }	\
static void change_ws8(void) { change_workspace(7); }	\
static void moveto_ws9(void) { move_to_workspace(8); }	\
static void change_ws9(void) { change_workspace(8); }	\

#define UDIST(a,b) abs((int)(a) - (int)(b))
#define MAXCLIENTS	99
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
	Bool is_func;
} Binding;

typedef struct Client{
	Window win;
	uint x, y, h, w;
	Bool floating;
	struct Client *next;
} Client;

#endif
