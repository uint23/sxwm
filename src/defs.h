/* See LICENSE for more information on use */
#pragma once
#include <X11/Xlib.h>
#define SXWM_VERSION "sxwm ver. 1.8"
#define SXWM_AUTHOR "(C) Abhinav Prasai 2025"
#define SXWM_LICINFO "See LICENSE for more info"

#define MF_MIN               0.05f
#define MF_MAX               0.95f
#define MAX(a, b)            ((a) > (b) ? (a) : (b))
#define MIN(a, b)            ((a) < (b) ? (a) : (b))
#define UDIST(a, b)          abs((int)(a) - (int)(b))
#define CLAMP(x, lo, hi)     (((x) < (lo)) ? (lo) : ((x) > (hi)) ? (hi) : (x))

#define MAX_MONITORS         32
#define MAX_BINDS            256
#define MAX_CLIENTS          99
#define MAX_SCRATCHPADS      32
#define MAX_ITEMS            256
#define MIN_WINDOW_SIZE      20
#define PATH_MAX             4096

/* workspaces */
#define TYPE_WS_CHANGE       0
#define TYPE_WS_MOVE         1
/* fn/cmd */
#define TYPE_FUNC            2
#define TYPE_CMD             3
/* scratchpads*/
#define TYPE_SP_REMOVE       4
#define TYPE_SP_TOGGLE       5
#define TYPE_SP_CREATE       6

#define NUM_WORKSPACES		 9
#define WORKSPACE_NAMES	\
	"1""\0"\
	"2""\0"\
	"3""\0"\
	"4""\0"\
	"5""\0"\
	"6""\0"\
	"7""\0"\
	"8""\0"\
	"9""\0"


typedef enum { DRAG_NONE, DRAG_MOVE, DRAG_RESIZE, DRAG_SWAP } DragMode;
typedef void (*EventHandler)(XEvent *);

typedef union {
	const char **cmd;
	void (*fn)(void);
	int ws;            /* workspace */
	int sp;            /* scratchpad */
} Action;

typedef struct {
	int mods;
	KeySym keysym;
	KeyCode keycode;
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
	int n_binds;
	int move_window_amt;
	int resize_window_amt;
	Bool new_win_focus;
	Bool warp_cursor;
	Bool floating_on_top;
	Bool new_win_master;
	Binding binds[MAX_ITEMS];
	char **should_float[MAX_ITEMS];
	char **start_fullscreen[MAX_ITEMS];
	char **can_swallow[MAX_ITEMS];
	char **can_be_swallowed[MAX_ITEMS];
	char **scratchpads[MAX_SCRATCHPADS];
	char **open_in_workspace[MAX_ITEMS];
	char *to_run[MAX_ITEMS];
} Config;

typedef struct {
	const char *name;
	void (*fn)(void);
} CommandEntry;

typedef struct {
	int x, y;
	int w, h;
	int reserve_left, reserve_right;
	int reserve_top, reserve_bottom;
} Monitor;

typedef struct {
	Client *client;
	Bool enabled;
} Scratchpad;
