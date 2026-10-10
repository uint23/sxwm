#ifndef COMMON_H
#define COMMON_H

#include <X11/Xlib.h>

#define SXWM_VERSION "sxwm ver. 1.8"
#define SXWM_AUTHOR "(C) uint 2024-2026"
#define SXWM_LICINFO "See LICENSE for more info"

#define MF_MIN           0.05f
#define MF_MAX           0.95f
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define UDIST(a, b)      abs((int)(a) - (int)(b))
#define CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : ((x) > (hi)) ? (hi) : (x))

#define MAX_MONITORS    32
#define MAX_BINDS       256
#define MAX_ITEMS       256
#define MIN_WINDOW_SIZE 20
#define PATH_MAX        4096

#define NUM_WORKSPACES 9
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

enum {
	TYPE_WS_CHANGE = 0,
	TYPE_WS_MOVE = 1,
	TYPE_FUNC = 2,
	TYPE_CMD = 3,
};

typedef enum { DRAG_NONE, DRAG_MOVE, DRAG_RESIZE } DragMode;
typedef enum { LIST_TILED, LIST_FLOATING, LIST_COUNT } ListType;
typedef enum { WINDOW_NORMAL, WINDOW_FLOAT, WINDOW_DOCK } WindowType;
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;
typedef void (*EventHandler)(XEvent*);

typedef union Action Action;
typedef struct Binding Binding;
typedef struct Client Client;
typedef struct ClientList ClientList;
typedef struct Config Config;
typedef struct CommandEntry CommandEntry;
typedef struct Monitor Monitor;
typedef struct Point Point;
typedef struct Workspace Workspace;
typedef struct WorkspaceRule WorkspaceRule;

union Action {
	char* cmd;
	void (*fn)(void);
	int ws;
};

struct Binding {
	int mods;
	KeySym keysym;
	KeyCode keycode;
	Action action;
	int type;
};

struct Client {
	Window win;
	int x, y, w, h, ox, oy, ow, oh, mon;
	Bool fixed, fullscreen, mapped;
	ClientList* list;
	Client* next, *prev;
};

struct ClientList {
	Client* head, *tail;
	Workspace* workspace;
	ListType type;
	unsigned int count;
};

struct WorkspaceRule {
	char* name;
	int workspace;
};

struct Config {
	int modkey, gaps, border_width, motion_throttle, resize_master_amt;
	int snap_distance, n_binds, move_window_amt, resize_window_amt;
	long border_foc_col, border_ufoc_col;
	float master_width[MAX_MONITORS];
	char* should_float[MAX_ITEMS], *start_fullscreen[MAX_ITEMS], *to_run[MAX_ITEMS];
	Bool new_win_focus ,warp_cursor ,floating_on_top ,new_win_master;
	WorkspaceRule open_in_workspace[MAX_ITEMS];
	Binding binds[MAX_ITEMS];
};

struct CommandEntry {
	const char* name;
	void (*fn)(void);
};

struct Monitor {
	int x, y, w, h;
	struct { int left, right, bottom, top; } res;
};

struct Point {
	int x, y;
};

struct Workspace {
	int number;
	Client* focused;
	ClientList lists[LIST_COUNT];
};

typedef enum {
	ATOM_NET_ACTIVE_WINDOW,
	ATOM_NET_CURRENT_DESKTOP,
	ATOM_NET_SUPPORTED,
	ATOM_NET_WM_STATE,
	ATOM_NET_WM_STATE_FULLSCREEN,
	ATOM_WM_STATE,
	ATOM_NET_WM_WINDOW_TYPE,
	ATOM_NET_WORKAREA,
	ATOM_WM_DELETE_WINDOW,
	ATOM_NET_WM_STRUT,
	ATOM_NET_WM_STRUT_PARTIAL,
	ATOM_NET_SUPPORTING_WM_CHECK,
	ATOM_NET_WM_NAME,
	ATOM_UTF8_STRING,
	ATOM_NET_WM_DESKTOP,
	ATOM_NET_CLIENT_LIST,
	ATOM_NET_FRAME_EXTENTS,
	ATOM_NET_NUMBER_OF_DESKTOPS,
	ATOM_NET_DESKTOP_NAMES,
	ATOM_NET_WM_PID,
	ATOM_NET_WM_WINDOW_TYPE_DOCK,
	ATOM_NET_WM_WINDOW_TYPE_UTILITY,
	ATOM_NET_WM_WINDOW_TYPE_DIALOG,
	ATOM_NET_WM_WINDOW_TYPE_TOOLBAR,
	ATOM_NET_WM_WINDOW_TYPE_SPLASH,
	ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU,
	ATOM_NET_WM_WINDOW_TYPE_MENU,
	ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
	ATOM_NET_WM_WINDOW_TYPE_TOOLTIP,
	ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION,
	ATOM_NET_WM_STATE_MODAL,
	ATOM_WM_PROTOCOLS,
	ATOM_COUNT
} AtomType;

#endif /* COMMON_H */

