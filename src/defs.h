/* See LICENSE for more information on use */
#pragma once
#include <X11/Xlib.h>
#define SXWM_VERSION "sxwm ver. 1.6"
#define SXWM_AUTHOR "(C) Abhinav Prasai 2025"
#define SXWM_LICINFO "See LICENSE for more info"

#define ALT Mod1Mask
#define SUPER Mod4Mask
#define SHIFT ShiftMask

#define MARGIN (gaps + BORDER_WIDTH)
#define OUT_IN (2 * BORDER_WIDTH)
#define MF_MIN 0.05f
#define MF_MAX 0.95f
#define MAX_MONITORS 32
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define UDIST(a, b) abs((int)(a) - (int)(b))
#define CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : ((x) > (hi)) ? (hi) : (x))
#define MAXCLIENTS 99
#define MAX_SCRATCHPADS 20
#define MIN_WINDOW_SIZE 20
#define BIND(mod, key, cmdstr) {(mod), XK_##key, {cmdstr}, False}
#define CALL(mod, key, fnptr) {(mod), XK_##key, {.fn = fnptr}, True}
#define CMD(name, ...) const char *name[] = {__VA_ARGS__, NULL}

/* workspaces */
#define TYPE_FUNC 2
#define TYPE_WS_CHANGE 0
#define TYPE_WS_MOVE 1
/* fn/cmd */
#define TYPE_FUNC 2
#define TYPE_CMD 3
/* scratchpads*/
#define TYPE_SP_REMOVE 4
#define TYPE_SP_TOGGLE 5
#define TYPE_SP_CREATE 6

#define NUM_WORKSPACES		9
#define WORKSPACE_NAMES		\
	"1"					"\0"\
	"2"					"\0"\
	"3"					"\0"\
	"4"					"\0"\
	"5"					"\0"\
	"6"					"\0"\
	"7"					"\0"\
	"8"					"\0"\
	"9"					"\0"

#define MAX_BINDS 256

typedef enum { DRAG_NONE, DRAG_MOVE, DRAG_RESIZE, DRAG_SWAP } DragMode;
typedef void (*EventHandler)(XEvent *);

typedef union {
	const char **cmd;
	void (*fn)(void);
	int ws; /* workspace */
	int sp; /* scratchpad */
} Action;

typedef struct {
	int mods;
	KeySym keysym;
	Action action;
	int type;
} Binding;

typedef struct Client {
	Window win;
	int x, y, h, w;
	int orig_x, orig_y, orig_w, orig_h;
	int custom_stack_height;
	int mon;
	int ws;
	Bool fixed;
	Bool floating;
	Bool fullscreen;
	Bool mapped;
	pid_t pid;
	struct Client *next;
	struct Client *swallowed;
	struct Client *swallower;
} Client;

typedef struct {
	int modkey;
	int gaps;
	int border_width;
	long border_foc_col;
	long border_ufoc_col;
	long border_swap_col;
	float master_width[MAX_MONITORS];
	int motion_throttle;
	int resize_master_amt;
	int resize_stack_amt;
	int snap_distance;
	int bindsn;
	Bool new_win_focus;
	Bool warp_cursor;
	Bool new_win_master;
	Binding binds[256];
	char **should_float[256];
	char **start_fullscreen[256];
	char **can_swallow[256];
    char **can_be_swallowed[256];
	char **scratchpads[32];
	char **open_in_workspace[256];
	char *torun[256];
} Config;

typedef struct {
	int x, y;
	int w, h;
	int reserve_left, reserve_right,
		reserve_top, reserve_bottom;
} Monitor;

typedef struct {
	Client *client;
	Bool enabled;
} Scratchpad;

typedef struct {
	const char *name;
	void (*fn)(void);
} CommandEntry;

extern void centre_window();
extern void close_focused(void);
extern void dec_gaps(void);
extern void focus_next(void);
extern void focus_prev(void);
extern void focus_next_mon(void);
extern void focus_prev_mon(void);
extern void move_next_mon(void);
extern void move_prev_mon(void);
extern void inc_gaps(void);
extern void move_master_next(void);
extern void move_master_prev(void);
extern long parse_col(const char *hex);
extern void quit(void);
extern void reload_config(void);
extern void resize_master_add(void);
extern void resize_master_sub(void);
extern void resize_stack_add(void);
extern void resize_stack_sub(void);
extern void toggle_floating(void);
extern void toggle_floating_global(void);
extern void toggle_fullscreen(void);
