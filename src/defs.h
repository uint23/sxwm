/* See LICENSE for more information on use */

#ifndef DEFS_H
#define DEFS_H

#include <X11/Xlib.h>

#define uint unsigned int
#define ulong unsigned long
#define u_char unsigned char

#define SXWM_VERSION	"sxwm ver. 0.1.6"
#define SXWM_AUTHOR		"(C) Abhinav Prasai 2025"
#define SXWM_LICINFO	"See LICENSE for more info"

#define ALT		Mod1Mask
#define SUPER	Mod4Mask
#define SHIFT	ShiftMask

#define MARGIN (gaps + BORDER_WIDTH)
#define OUT_IN (2 * BORDER_WIDTH)
#define MF_MIN 0.05f
#define MF_MAX 0.95f
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
static void change_ws1(void) { change_workspace(0); update_net_client_list(); }	\
static void moveto_ws1(void) { move_to_workspace(0); update_net_client_list(); }\
static void change_ws2(void) { change_workspace(1); update_net_client_list(); }	\
static void moveto_ws2(void) { move_to_workspace(1); update_net_client_list(); }\
static void change_ws3(void) { change_workspace(2); update_net_client_list(); }	\
static void moveto_ws3(void) { move_to_workspace(2); update_net_client_list(); }\
static void change_ws4(void) { change_workspace(3); update_net_client_list(); }	\
static void moveto_ws4(void) { move_to_workspace(3); update_net_client_list(); }\
static void change_ws5(void) { change_workspace(4); update_net_client_list(); }	\
static void moveto_ws5(void) { move_to_workspace(4); update_net_client_list(); }\
static void change_ws6(void) { change_workspace(5); update_net_client_list(); }	\
static void moveto_ws6(void) { move_to_workspace(5); update_net_client_list(); }\
static void change_ws7(void) { change_workspace(6); update_net_client_list(); }	\
static void moveto_ws7(void) { move_to_workspace(6); update_net_client_list(); }\
static void change_ws8(void) { change_workspace(7); update_net_client_list(); }	\
static void moveto_ws8(void) { move_to_workspace(7); update_net_client_list(); }\
static void change_ws9(void) { change_workspace(8); update_net_client_list(); }	\
static void moveto_ws9(void) { move_to_workspace(8); update_net_client_list(); }\

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
	uint orig_x, orig_y, orig_w, orig_h;
	Bool fixed;
	Bool floating;
	Bool fullscreen;
	struct Client *next;
} Client;

#endif
