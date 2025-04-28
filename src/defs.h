/* See LICENSE for more information on use */
#pragma once
#define SXWM_VERSION	"sxwm ver. 1.2"
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
	const char *name[] = { __VA_ARGS__, NULL }


#include <X11/Xlib.h>
#define INIT_WORKSPACE \
void change_ws1(void);\
void moveto_ws1(void);\
void change_ws2(void);\
void moveto_ws2(void);\
void change_ws3(void);\
void moveto_ws3(void);\
void change_ws4(void);\
void moveto_ws4(void);\
void change_ws5(void);\
void moveto_ws5(void);\
void change_ws6(void);\
void moveto_ws6(void);\
void change_ws7(void);\
void moveto_ws7(void);\
void change_ws8(void);\
void moveto_ws8(void);\
void change_ws9(void);\
void moveto_ws9(void);\
void change_ws1(void) { change_workspace(0); update_net_client_list(); }	\
void moveto_ws1(void) { move_to_workspace(0); update_net_client_list(); }\
void change_ws2(void) { change_workspace(1); update_net_client_list(); }	\
void moveto_ws2(void) { move_to_workspace(1); update_net_client_list(); }\
void change_ws3(void) { change_workspace(2); update_net_client_list(); }	\
void moveto_ws3(void) { move_to_workspace(2); update_net_client_list(); }\
void change_ws4(void) { change_workspace(3); update_net_client_list(); }	\
void moveto_ws4(void) { move_to_workspace(3); update_net_client_list(); }\
void change_ws5(void) { change_workspace(4); update_net_client_list(); }	\
void moveto_ws5(void) { move_to_workspace(4); update_net_client_list(); }\
void change_ws6(void) { change_workspace(5); update_net_client_list(); }	\
void moveto_ws6(void) { move_to_workspace(5); update_net_client_list(); }\
void change_ws7(void) { change_workspace(6); update_net_client_list(); }	\
void moveto_ws7(void) { move_to_workspace(6); update_net_client_list(); }\
void change_ws8(void) { change_workspace(7); update_net_client_list(); }	\
void moveto_ws8(void) { move_to_workspace(7); update_net_client_list(); }\
void change_ws9(void) { change_workspace(8); update_net_client_list(); }	\
void moveto_ws9(void) { move_to_workspace(8); update_net_client_list(); }\

#define UDIST(a,b) abs((int)(a) - (int)(b))
#define MAXCLIENTS	99
#define MAXGAPS		100

typedef enum {
	DRAG_NONE,
	DRAG_MOVE,
	DRAG_RESIZE,
	DRAG_SWAP
} DragMode;

typedef void (*EventHandler)(XEvent *);

typedef union {
	const char **cmd;
	void (*fn)(void);
} Action;

typedef struct {
	int mods;
	KeySym keysym;
	Action action;
	Bool is_func;
} Binding;

typedef struct Client{
	Window win;
	int x, y, h, w;
	int orig_x, orig_y, orig_w, orig_h;
	int mon;
	Bool fixed;
	Bool floating;
	Bool fullscreen;
	struct Client *next;
} Client;

typedef struct {
	int modkey;
	int gaps;
	int border_width;
	long border_foc_col;
	long border_ufoc_col;
	long border_swap_col;
	int master_width;
	int resize_master_amt;
	int snap_distance;
	int bindsn;
	Binding binds[256];
} Config;

typedef struct {
	int x, y;
	int w, h;
} Monitor;

extern void close_focused(void);
extern void dec_gaps(void);
extern void focus_next(void);
extern void focus_prev(void);
extern void inc_gaps(void);
extern void move_master_next(void);
extern void move_master_prev(void);
extern long parse_col(const char *hex);
extern void quit(void);
extern void reload_config(void);
extern void resize_master_add(void);
extern void resize_master_sub(void);
extern void toggle_floating(void);
extern void toggle_floating_global(void);
extern void toggle_fullscreen(void);
