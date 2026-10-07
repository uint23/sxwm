/* See LICENSE for more info
  
   A very SeXy Window Manager:
   sxwm is a user-friendly, easily configurable yet powerful
   tiling window manager inspired by window managers such as
   DWM and i3.
  
   The userconfig is designed to be as user-friendly as
   possible, and I hope it is easy to configure even without
   knowledge of C or programming, although most people who
   will use this will probably be programmers :)
  
   (c) uint 2024-2026 */

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

#include <X11/keysym.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xinerama.h>
#include <X11/Xcursor/Xcursor.h>

#include "common.h"
#include "extern.h"
#include "parser.h"

Client *add_client(Window w, Bool floating, int ws);
void apply_fullscreen(Client *c, Bool on);
void centre_client(Client *c);
/* void centre_window(void); */
void change_workspace(int ws);
int clean_mask(int mask);
Bool client_is_floating(Client *c);
/* void close_focused(void); */
void configure_tile(Client *c, int x, int y, int w, int h);
/* void dec_gaps(void); */
Client *find_client(Window w);
Window find_toplevel(Window w);
/* void focus_next(void); */
/* void focus_prev(void); */
/* void focus_next_mon(void); */
/* void focus_prev_mon(void); */
Bool get_cursor_point(Point* p);
Client* get_last_client(Client *root);
int get_monitor_for_point(Point p);
Point get_window_center(Client* c);
WindowType get_window_type(Window w);
int get_workspace_for_window(Window w);
void grab_button(Mask button, Mask mod, Window w, Bool owner_events, Mask masks);
void grab_keys(void);
void hdl_button(XEvent *xev);
void hdl_button_release(XEvent *xev);
void hdl_client_msg(XEvent *xev);
void hdl_config_ntf(XEvent *xev);
void hdl_config_req(XEvent *xev);
void hdl_dummy(XEvent *xev);
void hdl_destroy_ntf(XEvent *xev);
void hdl_keypress(XEvent *xev);
void hdl_mapping_ntf(XEvent *xev);
void hdl_map_req(XEvent *xev);
void hdl_motion(XEvent *xev);
void hdl_property_ntf(XEvent *xev);
void hdl_unmap_ntf(XEvent *xev);
/* void inc_gaps(void); */
void init_defaults(void);
/* void move_master_next(void); */
/* void move_master_prev(void); */
/* void move_next_mon(void); */
/* void move_prev_mon(void); */
void move_to_workspace(int ws);
/* void move_win_down(void); */
/* void move_win_left(void); */
/* void move_win_right(void); */
/* void move_win_up(void); */
void other_wm(void);
int other_wm_err(Display *d, XErrorEvent *ee);
/* long parse_col(const char *hex); */
/* void quit(void); */
/* void reload_config(void); */
/* void resize_master_add(void); */
/* void resize_master_sub(void); */
/* void resize_stack_add(void); */
/* void resize_stack_sub(void); */
/* void resize_win_down(void); */
/* void resize_win_left(void); */
/* void resize_win_right(void); */
/* void resize_win_up(void); */
void run(void);
void reset_opacity(Window w);
void scan_existing_windows(void);
void select_input(Window w, Mask masks);
void send_wm_take_focus(Window w);
void setup(void);
void setup_atoms(void);
void set_frame_extents(Window w);
void set_input_focus(Client *c, Bool raise_win, Bool warp);
void set_opacity(Window w, double opacity);
void set_wm_state(Window w, long state);
int snap_coordinate(int pos, int size, int screen_size, int snap_dist);
void spawn(const char * const *argv);
void startup_exec(void);
/* void switch_previous_workspace(void); */
void switch_client_list(Client *c, Bool floating);
void tile(void);
/* void toggle_floating(void); */
/* void toggle_floating_global(void); */
/* void toggle_fullscreen(void); */
void unlink_client(Client *c);
void update_borders(void);
void update_client_desktop_properties(void);
void update_modifier_masks(void);
void update_mons(void);
void update_net_client_list(void);
void update_struts(void);
void update_workarea(void);
void warp_cursor(Client *c);
Bool window_has_ewmh_state(Window w, Atom state);
Bool window_matches_class(Window w, char ***rules);
void window_set_ewmh_state(Window w, Atom state, Bool add);
Bool window_should_float(Window w);
Bool window_should_start_fullscreen(Window w);
int xerr(Display *d, XErrorEvent *ee);
void xev_case(XEvent *xev);

static Atom atoms[ATOM_COUNT];
static const char *atom_names[ATOM_COUNT] = {
	[ATOM_NET_ACTIVE_WINDOW]                = "_NET_ACTIVE_WINDOW",
	[ATOM_NET_CURRENT_DESKTOP]              = "_NET_CURRENT_DESKTOP",
	[ATOM_NET_SUPPORTED]                    = "_NET_SUPPORTED",
	[ATOM_NET_WM_STATE]                     = "_NET_WM_STATE",
	[ATOM_NET_WM_STATE_FULLSCREEN]          = "_NET_WM_STATE_FULLSCREEN",
	[ATOM_WM_STATE]                         = "WM_STATE",
	[ATOM_NET_WM_WINDOW_TYPE]               = "_NET_WM_WINDOW_TYPE",
	[ATOM_NET_WORKAREA]                     = "_NET_WORKAREA",
	[ATOM_WM_DELETE_WINDOW]                 = "WM_DELETE_WINDOW",
	[ATOM_NET_WM_STRUT]                     = "_NET_WM_STRUT",
	[ATOM_NET_WM_STRUT_PARTIAL]             = "_NET_WM_STRUT_PARTIAL",
	[ATOM_NET_SUPPORTING_WM_CHECK]          = "_NET_SUPPORTING_WM_CHECK",
	[ATOM_NET_WM_NAME]                      = "_NET_WM_NAME",
	[ATOM_UTF8_STRING]                      = "UTF8_STRING",
	[ATOM_NET_WM_DESKTOP]                   = "_NET_WM_DESKTOP",
	[ATOM_NET_CLIENT_LIST]                  = "_NET_CLIENT_LIST",
	[ATOM_NET_FRAME_EXTENTS]                = "_NET_FRAME_EXTENTS",
	[ATOM_NET_NUMBER_OF_DESKTOPS]           = "_NET_NUMBER_OF_DESKTOPS",
	[ATOM_NET_DESKTOP_NAMES]                = "_NET_DESKTOP_NAMES",
	[ATOM_NET_WM_PID]                       = "_NET_WM_PID",
	[ATOM_NET_WM_WINDOW_TYPE_DOCK]          = "_NET_WM_WINDOW_TYPE_DOCK",
	[ATOM_NET_WM_WINDOW_TYPE_UTILITY]       = "_NET_WM_WINDOW_TYPE_UTILITY",
	[ATOM_NET_WM_WINDOW_TYPE_DIALOG]        = "_NET_WM_WINDOW_TYPE_DIALOG",
	[ATOM_NET_WM_WINDOW_TYPE_TOOLBAR]       = "_NET_WM_WINDOW_TYPE_TOOLBAR",
	[ATOM_NET_WM_WINDOW_TYPE_SPLASH]        = "_NET_WM_WINDOW_TYPE_SPLASH",
	[ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU]    = "_NET_WM_WINDOW_TYPE_POPUP_MENU",
	[ATOM_NET_WM_WINDOW_TYPE_MENU]          = "_NET_WM_WINDOW_TYPE_MENU",
	[ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU] = "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
	[ATOM_NET_WM_WINDOW_TYPE_TOOLTIP]       = "_NET_WM_WINDOW_TYPE_TOOLTIP",
	[ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION]  = "_NET_WM_WINDOW_TYPE_NOTIFICATION",
	[ATOM_NET_WM_STATE_MODAL]               = "_NET_WM_STATE_MODAL",
	[ATOM_WM_PROTOCOLS]                     = "WM_PROTOCOLS",
};

struct {
	Cursor normal;
	Cursor move;
	Cursor resize;
} cursors;

struct {
	int type;
	int sx, sy; /* start (x, y) */
	int ox, oy, ow, oh; /* original (x, y), (w, h) */
	Client* c;
} drag;

Workspace workspaces[NUM_WORKSPACES] = {NULL};
Config user_config;
DragMode drag_mode = DRAG_NONE;
Client *drag_client = NULL;
EventHandler evtable[LASTEvent];
Display *dpy;
Window root;
Window wm_check_win;
Monitor *mons = NULL;
int n_mons = 0;
int previous_workspace = 0;
int current_ws = 0;
int current_mon = 0;
long last_motion_time = 0;
Bool global_floating = False;
Bool running = False;

Mask numlock_mask = 0;
Mask mode_switch_mask = 0;

int scr_width;
int scr_height;
int open_windows = 0;

Client *add_client(Window w, Bool floating, int ws)
{
	Client *c = malloc(sizeof(Client));
	if (!c) {
		fprintf(stderr, "sxwm: could not alloc memory for client\n");
		return NULL;
	}

	c->win = w;
	c->next = NULL;
	c->prev = NULL;

	/* subscribing to certain events */
	Mask window_masks = EnterWindowMask | LeaveWindowMask | FocusChangeMask | PropertyChangeMask |
	                    StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	select_input(w, window_masks);
	grab_button(Button1, None, w, False, ButtonPressMask);
	grab_button(Button1, user_config.modkey, w, False, ButtonPressMask);
	grab_button(Button3, user_config.modkey, w, False, ButtonPressMask);

	XWindowAttributes wa;
	XGetWindowAttributes(dpy, w, &wa);
	c->x = wa.x;
	c->y = wa.y;
	c->w = wa.width;
	c->h = wa.height;

	/* set monitor based on cursor location */
	int cursor_mon = 0;
	Point cp;
	if (get_cursor_point(&cp))
		cursor_mon = get_monitor_for_point(cp);

	/* set client defaults */
	c->mon = cursor_mon;
	c->fixed = False;
	c->fullscreen = False;
	c->mapped = True;
	c->custom_stack_height = 0;

	Client **head = floating ? &workspaces[ws].floating : &workspaces[ws].tiled;

	if (!*head) {
		*head = c;
	}
	else if (!floating && user_config.new_win_master) {
		c->next = *head;
		(*head)->prev = c;
		*head = c;
	}
	else {
		Client* tail = get_last_client(*head);
		c->prev = tail;
		tail->next = c;
	}

	open_windows++;

	/* remember first created client per workspace as a fallback */
	if (!workspaces[ws].focused)
		workspaces[ws].focused = c;

	if (ws == current_ws && workspaces[ws].focused == c)
		current_mon = c->mon;

	/* associate with workspace ws */
	long desktop = ws;
	XChangeProperty(dpy, w, atoms[ATOM_NET_WM_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&desktop, 1);

	/* graceful exits */
	Atom protos[] = { atoms[ATOM_WM_DELETE_WINDOW] };
	XSetWMProtocols(dpy, w, protos, 1);

	XRaiseWindow(dpy, w);
	return c;
}

void apply_fullscreen(Client *c, Bool on)
{
	if (!c || !c->mapped || c->fullscreen == on)
		return;


	if (on) {
		XWindowAttributes win_attr;
		if (!XGetWindowAttributes(dpy, c->win, &win_attr))
			return;

		c->orig_x = win_attr.x;
		c->orig_y = win_attr.y;
		c->orig_w = win_attr.width;
		c->orig_h = win_attr.height;

		c->fullscreen = True;

		int mon = CLAMP(c->mon, 0, n_mons - 1);
		/* make window fill mon */
		XSetWindowBorderWidth(dpy, c->win, 0);
		XMoveResizeWindow(dpy, c->win, mons[mon].x, mons[mon].y, mons[mon].w, mons[mon].h);

		c->x = mons[mon].x;
		c->y = mons[mon].y;
		c->w = mons[mon].w;
		c->h = mons[mon].h;

		XRaiseWindow(dpy, c->win);
		window_set_ewmh_state(c->win, atoms[ATOM_NET_WM_STATE_FULLSCREEN], True);
	}
	else {
		c->fullscreen = False;

		/* restore win attributes */
		XMoveResizeWindow(dpy, c->win, c->orig_x, c->orig_y, c->orig_w, c->orig_h);
		XSetWindowBorderWidth(dpy, c->win, user_config.border_width);
		window_set_ewmh_state(c->win, atoms[ATOM_NET_WM_STATE_FULLSCREEN], False);

		c->x = c->orig_x;
		c->y = c->orig_y;
		c->w = c->orig_w;
		c->h = c->orig_h;

		if (!c->floating)
			c->mon = get_monitor_for_point(get_window_center(c));
		tile();
		update_borders();
	}
}

void centre_client(Client *c)
{
	if (!c || n_mons < 1)
		return;

	c->x = mons[c->mon].x + (mons[c->mon].w - c->w) / 2 - user_config.border_width;
	c->y = mons[c->mon].y + (mons[c->mon].h - c->h) / 2 - user_config.border_width;
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
}

void centre_window(void)
{
	Client *c = workspaces[current_ws].focused;
	if (!c || !c->mapped || !client_is_floating(c))
		return;

	c->mon = get_monitor_for_point(get_window_center(c));
	centre_client(c);
}

void change_workspace(int ws)
{
	if (ws < 0 || ws >= NUM_WORKSPACES || ws == current_ws)
		return;

	Workspace *oldw = &workspaces[current_ws];
	Workspace *neww = &workspaces[ws];

	previous_workspace = current_ws;
	current_ws = ws;

	Client *oldc[] = { oldw->tiled, oldw->floating };
	Client *newc[] = { neww->tiled, neww->floating };

	/* map current workspace */
	for (int i = 0; i < 2; i++)
		for (Client *c = newc[i]; c; c = c->next)
			if (c->mapped)
				XMapWindow(dpy, c->win);

	/* unmap previous workspace */
	for (int i = 0; i < 2; i++)
		for (Client *c = oldc[i]; c; c = c->next)
			if (c->mapped)
				XUnmapWindow(dpy, c->win);

	/* restore previous focus, else current monitor */
	Client *focused = NULL;
	Client *fallback = NULL;

	for (int i = 0; i < 2; i++) {
		for (Client *c = newc[i]; c; c = c->next) {
			if (!c->mapped)
				continue;

			if (c == neww->focused) {
				focused = c;
				break;
			}

			if (!fallback || (fallback->mon != current_mon && c->mon == current_mon))
				fallback = c;
		}
		if (focused)
			break;
	}

	tile();
	set_input_focus(focused ? focused : fallback, False, True);

	long desktop = current_ws;
	XChangeProperty(dpy, root, atoms[ATOM_NET_CURRENT_DESKTOP], XA_CARDINAL, 32,
	                PropModeReplace, (unsigned char *)&desktop, 1);
	update_client_desktop_properties();
}

int clean_mask(int mask)
{
	return mask & ~(LockMask | numlock_mask | mode_switch_mask);
}

Bool client_is_floating(Client *c)
{
	if (!c || c->ws < 0 || c->ws >= NUM_WORKSPACES)
		return False;

	for (Client *p = workspaces[c->ws].floating; p; p = p->next)
		if (p == c)
			return True;
	return False;
}

void close_focused(void)
{
	if (!focused)
		return;

	Atom *protocols;
	int n_protocols;
	if (XGetWMProtocols(dpy, focused->win, &protocols, &n_protocols) && protocols) {
		for (int i = 0; i < n_protocols; i++) {
			if (protocols[i] == atoms[ATOM_WM_DELETE_WINDOW]) {
				XEvent ev = {.xclient = {
					.type = ClientMessage,
					.window = focused->win,
					.message_type = atoms[ATOM_WM_PROTOCOLS],
					.format = 32}
				};

				ev.xclient.data.l[0] = atoms[ATOM_WM_DELETE_WINDOW];
				ev.xclient.data.l[1] = CurrentTime;
				XSendEvent(dpy, focused->win, False, NoEventMask, &ev);
				XFree(protocols);
				return;
			}
		}
		XUnmapWindow(dpy, focused->win);
		XFree(protocols);
	}
	XUnmapWindow(dpy, focused->win);
	XKillClient(dpy, focused->win);
}

void configure_tile(Client *c, int x, int y, int w, int h)
{
	int bw = 2 * user_config.border_width;
	w = MAX(1, w - bw);
	h = MAX(1, h - bw);

	if (c->x != x || c->y != y || c->w != w || c->h != h) {
		XWindowChanges wc = {
			.x = x, .y = y, .width = w, .height = h,
			.border_width = user_config.border_width
		};
		XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	}

	c->x = x;
	c->y = y;
	c->w = w;
	c->h = h;
}

void dec_gaps(void)
{
	if (user_config.gaps > 0) {
		user_config.gaps--;
		tile();
		update_borders();
	}
}

Client *find_client(Window w)
{
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws].tiled; c; c = c->next)
			if (c->win == w)
				return c;
		for (Client *c = workspaces[ws].floating; c; c = c->next)
			if (c->win == w)
				return c;
	}
	return NULL;
}

Window find_toplevel(Window w)
{
	if (!w || w == None)
		return root;

	Window root_win = None;
	Window parent;
	Window *kids;
	unsigned n_kids;

	while (True) {
		if (w == root_win)
			break;
		if (XQueryTree(dpy, w, &root_win, &parent, &kids, &n_kids) == 0)
			break;
		if (kids)
			XFree(kids);
		if (parent == root_win || parent == None)
			break;
		w = parent;
	}
	return w;
}

void focus_next(void)
{
	if (!workspaces[current_ws])
		return;

	Client *start = focused ? focused : workspaces[current_ws];
	Client *c = start;

	/* loop until we find a mapped client or return to start */
	do
		c = c->next ? c->next : workspaces[current_ws];
	while (( !c->mapped || c->mon != current_mon ) && c != start);

	/* if we return to start: */
	if (!c->mapped || c->mon != current_mon)
		return;

	focused = c;
	current_mon = c->mon;
	set_input_focus(focused, True, True);
}

void focus_prev(void)
{
	if (!workspaces[current_ws])
		return;

	Client *start = focused ? focused : workspaces[current_ws];
	Client *c = start;

	do {
		c = c->prev ? c->prev : get_last_client(workspaces[current_ws]);
	} while ((!c->mapped || c->mon != current_mon) && c != start);

	if (!c->mapped || c->mon != current_mon)
		return;

	focused = c;
	current_mon = c->mon;
	set_input_focus(focused, True, True);
}

void focus_next_mon(void)
{
	if (n_mons <= 1)
		return;

	int target_mon = (current_mon + 1) % n_mons;
	/* find the first window on the target monitor in current workspace */
	Client *target_client = NULL;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mon == target_mon && c->mapped) {
			target_client = c;
			break;
		}
	}

	if (target_client) {
		/* focus the window on target monitor */
		focused = target_client;
		current_mon = target_mon;
		set_input_focus(focused, True, True);
	}
	else {
		/* no windows on target monitor, just move cursor to center and update current_mon */
		current_mon = target_mon;
		int center_x = mons[target_mon].x + mons[target_mon].w / 2;
		int center_y = mons[target_mon].y + mons[target_mon].h / 2;
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, center_x, center_y);
		XSync(dpy, False);
	}
}

void focus_prev_mon(void)
{
	if (n_mons <= 1)
		return; /* only one monitor, nothing to switch to */

	int target_mon = (current_mon - 1 + n_mons) % n_mons;
	/* find the first window on the target monitor in current workspace */
	Client *target_client = NULL;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mon == target_mon && c->mapped) {
			target_client = c;
			break;
		}
	}

	if (target_client) {
		/* focus the window on target monitor */
		focused = target_client;
		current_mon = target_mon;
		set_input_focus(focused, True, True);
	}
	else {
		current_mon = target_mon;
		int center_x = mons[target_mon].x + mons[target_mon].w / 2;
		int center_y = mons[target_mon].y + mons[target_mon].h / 2;
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, center_x, center_y);
		XSync(dpy, False);
	}
}

Bool get_cursor_point(Point* p)
{
	Window root_ret, child_ret;
	int win_x, win_y;
	unsigned int masks;

	return XQueryPointer(
		dpy, root, &root_ret, &child_ret,
		&p->x, &p->y, &win_x, &win_y, &masks
	);
}

Client* get_last_client(Client *root)
{
	while (root->next)
		root = root->next;
	return root;
}

int get_monitor_for_point(Point p)
{
	for (int m = 0; m < n_mons; m++) {
		if (p.x >= mons[m].x && p.x < mons[m].x + mons[m].w && p.y >= mons[m].y && p.y < mons[m].y + mons[m].h)
			return m;
	}
	return 0;
}

Point get_window_center(Client *c)
{
	return (Point){ c->x + c->w / 2, c->y + c->h / 2 };
}

WindowType get_window_type(Window w)
{
	Atom actual_type;
	int format;
	unsigned long count, remaining;
	Atom *types = NULL;
	WindowType type = WINDOW_NORMAL;

	Bool prop = XGetWindowProperty(
		dpy, w, atoms[ATOM_NET_WM_WINDOW_TYPE], 0, 32, False, XA_ATOM,
		&actual_type, &format, &count, &remaining, (unsigned char **)&types
	) == Success && actual_type == XA_ATOM && format == 32 && types;

	if (prop) {
		for (unsigned long i = 0; i < count; i++) {
			Atom t = types[i];

			if (t == atoms[ATOM_NET_WM_WINDOW_TYPE_DOCK]) {
				type = WINDOW_DOCK;
				break;
			}

			if (t == atoms[ATOM_NET_WM_WINDOW_TYPE_UTILITY] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_DIALOG] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_TOOLBAR] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_SPLASH] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_MENU] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_TOOLTIP] ||
			    t == atoms[ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION])
				type = WINDOW_FLOAT;
		}
	}

	if (types)
		XFree(types);

	return type;
}

int get_workspace_for_window(Window w)
{
	XClassHint ch = {0};
	if (!XGetClassHint(dpy, w, &ch))
		return current_ws;

	for (int i = 0; i < MAX_ITEMS; i++) {
		/* TODO: Add docs for open_in_workspace */
		if (!user_config.open_in_workspace[i])
			break;

		char *rule_class = user_config.open_in_workspace[i][0];
		char *rule_ws = user_config.open_in_workspace[i][1];

		if (rule_class && rule_ws) {
			if ((ch.res_class && strcasecmp(ch.res_class, rule_class) == 0) ||
			    (ch.res_name && strcasecmp(ch.res_name, rule_class) == 0)) {
				XFree(ch.res_class);
				XFree(ch.res_name);
				return atoi(rule_ws);
			}
		}
	}

	XFree(ch.res_class);
	XFree(ch.res_name);

	return current_ws; /* default */
}

void grab_button(Mask button, Mask mod, Window w, Bool owner_events, Mask masks)
{
	Mask guards[] = {
		0,
		LockMask,
		numlock_mask,
		LockMask | numlock_mask,
		mode_switch_mask,
		LockMask | mode_switch_mask,
		numlock_mask | mode_switch_mask,
		LockMask | numlock_mask | mode_switch_mask
	};

	for (size_t i = 0; i < sizeof(guards) / sizeof(guards[0]); i++) {
		XGrabButton(
			dpy, button, mod | guards[i], w, owner_events, masks,
			w == root ? GrabModeAsync : GrabModeSync,
			GrabModeAsync, None, None
		);
	}
}

void grab_keys(void)
{
	Mask guards[] = {
		0, LockMask, numlock_mask, LockMask | numlock_mask, mode_switch_mask,
		LockMask | mode_switch_mask, numlock_mask | mode_switch_mask,
		LockMask | numlock_mask | mode_switch_mask
	};
	XUngrabKey(dpy, AnyKey, AnyModifier, root);

	for (int i = 0; i < user_config.n_binds; i++) {
		Binding *bind = &user_config.binds[i];

		if ((bind->type == TYPE_WS_CHANGE && bind->mods != user_config.modkey) ||
			(bind->type == TYPE_WS_MOVE   && bind->mods != (user_config.modkey | ShiftMask))) {
			continue;
		}

		bind->keycode = XKeysymToKeycode(dpy, bind->keysym);
		if (!bind->keycode)
			continue;

		for (size_t guard = 0; guard < sizeof(guards)/sizeof(guards[0]); guard++) {
			XGrabKey(dpy, bind->keycode, bind->mods | guards[guard],
					root, True, GrabModeAsync, GrabModeAsync);
		}
	}
}

void hdl_button(XEvent *xev)
{
	XButtonEvent *ev = &xev->xbutton;
	Window w = ev->subwindow != None ? ev->subwindow : ev->window;
	Client *c = find_client(find_toplevel(w));
	Bool mod = (clean_mask(ev->state) & user_config.modkey) == user_config.modkey;

	if (!c || c->ws != current_ws) {
		XAllowEvents(dpy, ReplayPointer, ev->time);
		return;
	}

	if (!mod) {
		if (ev->button == Button1)
			set_input_focus(c, True, False);
		XAllowEvents(dpy, ReplayPointer, ev->time);
		return;
	}

	XAllowEvents(dpy, AsyncPointer, ev->time);

	if (ev->button != Button1 && ev->button != Button3)
		return;
	if (c->fixed && ev->button == Button3)
		return;

	set_input_focus(c, True, False);
	if (!client_is_floating(c))
		toggle_floating();

	Cursor cursor = ev->button == Button1 ? cursors.move : cursors.resize;
	if (XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask,
	                 GrabModeAsync, GrabModeAsync, None, cursor, ev->time) != GrabSuccess)
		return;

	drag.c = c;
	drag.mode = ev->button == Button1 ? DRAG_MOVE : DRAG_RESIZE;
	drag.sx = ev->x_root;
	drag.sy = ev->y_root;
	drag.ox = c->x;
	drag.oy = c->y;
	drag.ow = c->w;
	drag.oh = c->h;
}

void hdl_button_release(XEvent *xev)
{
	(void)xev;

	XUngrabPointer(dpy, CurrentTime);

	drag_mode = DRAG_NONE;
	drag_client = NULL;
}

void hdl_client_msg(XEvent *xev)
{
	if (xev->xclient.message_type == atoms[ATOM_NET_CURRENT_DESKTOP]) {
		int ws = (int)xev->xclient.data.l[0];
		change_workspace(ws);
		return;
	}

	if (xev->xclient.message_type == atoms[ATOM_NET_WM_STATE]) {
		XClientMessageEvent *client_msg_ev = &xev->xclient;
		Window w = client_msg_ev->window;
		Client *c = find_client(find_toplevel(w));
		if (!c)
			return;

		/* 0=remove, 1=add, 2=toggle */
		long action = client_msg_ev->data.l[0];
		Atom a1 = (Atom)client_msg_ev->data.l[1];
		Atom a2 = (Atom)client_msg_ev->data.l[2];

		Atom state_atoms[2] = { a1, a2 };
		for (int i = 0; i < 2; i++) {
			if (state_atoms[i] == None)
				continue;

			if (state_atoms[i] == atoms[ATOM_NET_WM_STATE_FULLSCREEN]) {
				Bool want = c->fullscreen;
				if (action == 0)
					want = False;
				else if (action == 1)
					want = True;
				else if (action == 2)
					want = !want;

				apply_fullscreen(c, want);
			}
			/* TODO: other states */
		}
		return;
	}
}

void hdl_config_ntf(XEvent *xev)
{
	if (xev->xconfigure.window == root) {
		update_mons();
		tile();
		update_borders();
	}
}

void hdl_config_req(XEvent *xev)
{
	XConfigureRequestEvent *ev = &xev->xconfigurerequest;
	Client *c = find_client(ev->window);

	if (c && (!client_is_floating(c) || c->fullscreen))
		return;

	/* allow client to configure itself */
	XWindowChanges wc = {
		.x = ev->x,
		.y = ev->y,
		.width = ev->width,
		.height = ev->height,
		.border_width = ev->border_width,
		.sibling = ev->above,
		.stack_mode = ev->detail
	};
	XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
}

void hdl_dummy(XEvent *xev)
{
	(void)xev;
}

void hdl_destroy_ntf(XEvent *xev)
{
	Client *c = find_client(xev->xdestroywindow.window);
	if (!c)
		return;

	int ws = c->ws;
	Bool was_focused = workspaces[ws].focused == c;
	Client *prev = c->prev;
	Client *next = c->next;

	/* unlink from workspace list */
	unlink_client(c);
	if (was_focused)
		workspaces[ws].focused = NULL;
	if (drag_client == c) {
		drag_client = NULL;
		drag_mode = DRAG_NONE;
	}
	free(c);
	open_windows--;
	update_net_client_list();

	if (ws != current_ws)
		return;

	tile();
	update_borders();

	if (!was_focused)
		return;

	/* prefer previous window else next */
	Client *foc_new = NULL;
	if (prev && prev->mapped && prev->mon == current_mon)
		foc_new = prev;
	else if (next && next->mapped && next->mon == current_mon)
		foc_new = next;
	else {
		Client *lists[] = { workspaces[ws].tiled, workspaces[ws].floating };
		for (int i = 0; i < 2 && !foc_new; i++)
			for (Client *p = lists[i]; p; p = p->next)
				if (p->mapped && p->mon == current_mon) {
					foc_new = p;
					break;
				}
	}
	set_input_focus(foc_new, True, True);
}

void hdl_keypress(XEvent *xev)
{
	KeyCode code = xev->xkey.keycode;
	int mods = clean_mask(xev->xkey.state);

	for (int i = 0; i < user_config.n_binds; i++) {
		Binding *bind = &user_config.binds[i];
		if (bind->keycode == code && clean_mask(bind->mods) == mods) {
			switch (bind->type) {
				case TYPE_CMD: spawn(bind->action.cmd); break;
				case TYPE_FUNC: if (bind->action.fn) bind->action.fn(); break;
				case TYPE_WS_CHANGE: change_workspace(bind->action.ws); update_net_client_list(); break;
				case TYPE_WS_MOVE: move_to_workspace(bind->action.ws); update_net_client_list(); break;
			}
			return;
		}
	}
}

void hdl_mapping_ntf(XEvent *xev)
{
	XRefreshKeyboardMapping(&xev->xmapping);
	update_modifier_masks();
	grab_keys();
}

void hdl_map_req(XEvent *xev)
{
	Window w = xev->xmaprequest.window;
	XWindowAttributes wa;

	if (!XGetWindowAttributes(dpy, w, &wa))
		return;

	if (wa.override_redirect || wa.width <= 0 || wa.height <= 0) {
		XMapWindow(dpy, w);
		return;
	}

	/* already managed */
	Client *c = find_client(w);
	if (c) {
		if (c->ws != current_ws)
			return;

		if (!c->mapped) {
			XMapWindow(dpy, w);
			c->mapped = True;
		}

		if (user_config.new_win_focus)
			set_input_focus(c, True, True);
		else
			update_borders();
		return;
	}

	WindowType type = get_window_type(w);
	if (type == WINDOW_DOCK) {
		XMapWindow(dpy, w);
		return;
	}

	if (open_windows >= MAX_CLIENTS) {
		fprintf(stderr, "sxwm: max clients reached, ignoring map request\n");
		return;
	}

	/* classify window before adding */
	Bool fullscreen = window_should_start_fullscreen(w) || window_has_ewmh_state(w, atoms[ATOM_NET_WM_STATE_FULLSCREEN]);

	XSizeHints hints;
	long supplied;
	Bool fixed = XGetWMNormalHints(dpy, w, &hints, &supplied) &&
	             (hints.flags & PMinSize) && (hints.flags & PMaxSize) &&
	             hints.min_width == hints.max_width && hints.min_height == hints.max_height;

	Bool floating = type == WINDOW_FLOAT || global_floating || fixed ||
	                window_should_float(w) || window_has_ewmh_state(w, atoms[ATOM_NET_WM_STATE_MODAL]);

	Window transient;
	if (!floating && XGetTransientForHint(dpy, w, &transient))
		floating = True;

	if (fullscreen)
		floating = False;

	int ws = get_workspace_for_window(w);
	c = add_client(w, floating, ws);
	if (!c)
		return;

	c->ws = ws;
	c->fixed = fixed;
	set_wm_state(w, NormalState);

	/* position floating windows */
	if (floating) {
		c->w = MAX(c->w, 64);
		c->h = MAX(c->h, 64);
		centre_client(c);
		XSetWindowBorderWidth(dpy, w, user_config.border_width);
	}

	/* initialise fullscreen through the existing helper */
	if (fullscreen)
		apply_fullscreen(c, True);

	update_net_client_list();

	if (ws != current_ws)
		return;

	XMapWindow(dpy, w);
	c->mapped = True;

	if (!floating && !fullscreen)
		tile();
	else if (floating)
		XRaiseWindow(dpy, w);

	set_frame_extents(w);

	if (user_config.new_win_focus)
		set_input_focus(c, True, True);
	else
		update_borders();
}

void hdl_motion(XEvent *xev)
{
	XMotionEvent *motion_ev = &xev->xmotion;

	if ((drag_mode == DRAG_NONE || !drag_client) ||
		(motion_ev->time - last_motion_time <= (1000 / (Time)user_config.motion_throttle)))
		return;
	last_motion_time = motion_ev->time;

	/* figure out which monitor the pointer is in right now */
	int mon = 0;
	for (int i = 0; i < n_mons; i++) {
		Bool is_current_mon =
			motion_ev->x_root >= mons[i].x &&
			motion_ev->x_root < mons[i].x + mons[i].w &&
			motion_ev->y_root >= mons[i].y &&
			motion_ev->y_root < mons[i].y + mons[i].h;

		if (is_current_mon) {
			mon = i;
			break;
		}
	}
	Monitor *current_mon_motion = &mons[mon];

	if (drag_mode == DRAG_MOVE) {
		int dx = motion_ev->x_root - drag_start_x;
		int dy = motion_ev->y_root - drag_start_y;
		int nx = drag_orig_x + dx;
		int ny = drag_orig_y + dy;

		int outer_w = drag_client->w + 2 * user_config.border_width;
		int outer_h = drag_client->h + 2 * user_config.border_width;

		/* snap relative to this mons bounds: */
		int rel_x = nx - current_mon_motion->x;
		int rel_y = ny - current_mon_motion->y;

		rel_x = snap_coordinate(rel_x, outer_w, current_mon_motion->w, user_config.snap_distance);
		rel_y = snap_coordinate(rel_y, outer_h, current_mon_motion->h, user_config.snap_distance);

		nx = current_mon_motion->x + rel_x;
		ny = current_mon_motion->y + rel_y;

		if (!drag_client->floating && (UDIST(nx, drag_client->x) > user_config.snap_distance ||
			UDIST(ny, drag_client->y) > user_config.snap_distance)) {
			toggle_floating();
		}

		XMoveWindow(dpy, drag_client->win, nx, ny);
		drag_client->x = nx;
		drag_client->y = ny;
	}
	else if (drag_mode == DRAG_RESIZE) {
		int dx = motion_ev->x_root - drag_start_x;
		int dy = motion_ev->y_root - drag_start_y;
		int nw = drag_orig_w + dx;
		int nh = drag_orig_h + dy;

		/* clamp relative to this mon */
		int max_w = (current_mon_motion->w - (drag_client->x - current_mon_motion->x));
		int max_h = (current_mon_motion->h - (drag_client->y - current_mon_motion->y));

		drag_client->w = CLAMP(nw, MIN_WINDOW_SIZE, max_w);
		drag_client->h = CLAMP(nh, MIN_WINDOW_SIZE, max_h);

		XResizeWindow(dpy, drag_client->win, drag_client->w, drag_client->h);
	}
}

void hdl_property_ntf(XEvent *xev)
{
	XPropertyEvent *property_ev = &xev->xproperty;

	if (property_ev->window == root) {
		if (property_ev->atom == atoms[ATOM_NET_CURRENT_DESKTOP]) {
			long *val = NULL;
			Atom actual;
			int fmt;
			unsigned long n;
			unsigned long after;
			if (XGetWindowProperty(dpy, root, atoms[ATOM_NET_CURRENT_DESKTOP], 0, 1, False, XA_CARDINAL, &actual,
						           &fmt, &n, &after, (unsigned char **)&val) == Success && val) {
				change_workspace((int)val[0]);
				XFree(val);
			}
		}
		else if (property_ev->atom == atoms[ATOM_NET_WM_STRUT_PARTIAL]) {
			update_struts();
			tile();
			update_borders();
		}
	}

	/* client window properties */
	if (property_ev->atom == atoms[ATOM_NET_WM_STATE]) {
		Client *c = find_client(find_toplevel(property_ev->window));
		if (!c)
			return;

		Bool want = window_has_ewmh_state(c->win, atoms[ATOM_NET_WM_STATE_FULLSCREEN]);
		if (want != c->fullscreen)
			apply_fullscreen(c, want);
	}
}

void hdl_unmap_ntf(XEvent *xev)
{
	Client *c = find_client(xev->xunmap.window);
	if (!c || c->ws != current_ws || !c->mapped)
		return;

	c->mapped = False;
	tile();

	if (workspaces[current_ws].focused == c)
		set_input_focus(NULL, False, False);
	else
		update_borders();
}

void inc_gaps(void)
{
	user_config.gaps++;
	tile();
	update_borders();
}

void init_defaults(void)
{
	user_config.modkey = Mod4Mask;
	user_config.gaps = 10;
	user_config.border_width = 1;
	user_config.border_foc_col = parse_col("#c0cbff");
	user_config.border_ufoc_col = parse_col("#555555");
	user_config.move_window_amt = 10;
	user_config.resize_window_amt = 10;

	for (int i = 0; i < MAX_MONITORS; i++)
		user_config.master_width[i] = 50 / 100.0f;

	for (int i = 0; i < MAX_ITEMS; i++) {
		user_config.open_in_workspace[i] = NULL;
		user_config.start_fullscreen[i] = NULL;
	}

	user_config.motion_throttle = 60;
	user_config.resize_master_amt = 5;
	user_config.resize_stack_amt = 20;
	user_config.snap_distance = 5;
	user_config.n_binds = 0;
	user_config.new_win_focus = True;
	user_config.warp_cursor = True;
	user_config.new_win_master = False;
	user_config.floating_on_top = True;
}

void move_master_next(void)
{
	Client **head = &workspaces[current_ws].tiled;
	if (!*head || !(*head)->next)
		return;

	Client *first = *head;
	Client *old_focused = workspaces[current_ws].focused;

	*head = first->next;
	(*head)->prev = NULL;

	Client *tail = get_last_client(*head);
	tail->next = first;
	first->prev = tail;
	first->next = NULL;

	tile();

	if (user_config.warp_cursor && old_focused)
		warp_cursor(old_focused);
	if (old_focused)
		send_wm_take_focus(old_focused->win);

	update_borders();
}

void move_master_prev(void)
{
	Client **head = &workspaces[current_ws].tiled;
	if (!*head || !(*head)->next)
		return;

	Client *last = get_last_client(*head);
	Client *old_focused = workspaces[current_ws].focused;

	last->prev->next = NULL;
	last->prev = NULL;
	last->next = *head;
	(*head)->prev = last;
	*head = last;

	tile();

	if (user_config.warp_cursor && old_focused)
		warp_cursor(old_focused);
	if (old_focused)
		send_wm_take_focus(old_focused->win);

	update_borders();
}

void move_next_mon(void)
{
	if (!focused || n_mons <= 1)
		return; /* no focused window or only one monitor */

	int target_mon = (focused->mon + 1) % n_mons;

	/* update window's monitor assignment */
	focused->mon = target_mon;
	current_mon = target_mon;

	/* if window is floating, center it on the target monitor */
	if (focused->floating) {
		int mx = mons[target_mon].x, my = mons[target_mon].y;
		int mw = mons[target_mon].w, mh = mons[target_mon].h;
		int x = mx + (mw - focused->w) / 2;
		int y = my + (mh - focused->h) / 2;

		/* ensure window stays within monitor bounds */
		if (x < mx)
			x = mx;
		if (y < my)
			y = my;
		if (x + focused->w > mx + mw)
			x = mx + mw - focused->w;
		if (y + focused->h > my + mh)
			y = my + mh - focused->h;

		focused->x = x;
		focused->y = y;
		XMoveWindow(dpy, focused->win, x, y);
	}

	/* retile to update layouts on both monitors */
	tile();

	/* follow the window with cursor if enabled */
	if (user_config.warp_cursor)
		warp_cursor(focused);

	update_borders();
}

void move_prev_mon(void)
{
	if (!focused || n_mons <= 1)
		return; /* no focused window or only one monitor */

	int target_mon = (focused->mon - 1 + n_mons) % n_mons;

	/* update window's monitor assignment */
	focused->mon = target_mon;
	current_mon = target_mon;

	/* if window is floating, center it on the target monitor */
	if (focused->floating) {
		int mx = mons[target_mon].x, my = mons[target_mon].y;
		int mw = mons[target_mon].w, mh = mons[target_mon].h;
		int x = mx + (mw - focused->w) / 2;
		int y = my + (mh - focused->h) / 2;

		/* ensure window stays within monitor bounds */
		if (x < mx)
			x = mx;
		if (y < my)
			y = my;
		if (x + focused->w > mx + mw)
			x = mx + mw - focused->w;
		if (y + focused->h > my + mh)
			y = my + mh - focused->h;

		focused->x = x;
		focused->y = y;
		XMoveWindow(dpy, focused->win, x, y);
	}

	/* retile to update layouts on both monitors */
	tile();

	/* follow the window with cursor if enabled */
	if (user_config.warp_cursor)
		warp_cursor(focused);

	update_borders();
}

void move_to_workspace(int ws)
{
	if (!workspaces[current_ws].focused || ws < 0 || ws >= NUM_WORKSPACES || ws == current_ws)
		return;

	Client *moved = workspaces[current_ws].focused;
	int from_ws = current_ws;

	XUnmapWindow(dpy, moved->win);

	/* remove from current list */
	Bool floating = client_is_floating(moved);
	unlink_client(moved);

	/* push to target list */
	Client **head = floating ? &workspaces[ws].floating : &workspaces[ws].tiled;
	moved->next = *head;
	if (*head)
		(*head)->prev = moved;
	*head = moved;
	moved->ws = ws;
	long desktop = ws;
	XChangeProperty(dpy, moved->win, atoms[ATOM_NET_WM_DESKTOP], XA_CARDINAL, 32,
		        PropModeReplace, (unsigned char *)&desktop, 1);

	/* remember it as last-focused for the target workspace */
	workspaces[ws].focused = moved;

	/* retile current workspace and pick a new focus there */
	workspaces[from_ws].focused = workspaces[from_ws].tiled ? workspaces[from_ws].tiled : workspaces[from_ws].floating;
	tile();
	set_input_focus(workspaces[from_ws].focused, False, False);
}

void move_win_down(void)
{
	if (!focused || !focused->floating)
		return;

	focused->y += user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void move_win_left(void)
{
	if (!focused || !focused->floating)
		return;

	focused->x -= user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void move_win_right(void)
{
	if (!focused || !focused->floating)
		return;
	focused->x += user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void move_win_up(void)
{
	if (!focused || !focused->floating)
		return;

	focused->y -= user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void other_wm(void)
{
	XSetErrorHandler(other_wm_err);
	XChangeWindowAttributes(dpy, root, CWEventMask, &(XSetWindowAttributes){.event_mask = SubstructureRedirectMask});
	XSync(dpy, False);
	XSetErrorHandler(xerr);
	XChangeWindowAttributes(dpy, root, CWEventMask, &(XSetWindowAttributes){.event_mask = 0});
	XSync(dpy, False);
}

int other_wm_err(Display *d, XErrorEvent *ee)
{
	fprintf(stderr, "can't start because another window manager is already running");
	exit(EXIT_FAILURE);
	return 0;
	(void)d;
	(void)ee;
}

long parse_col(const char *hex)
{
	XColor col;
	Colormap cmap = DefaultColormap(dpy, DefaultScreen(dpy));

	if (!XParseColor(dpy, cmap, hex, &col)) {
		fprintf(stderr, "sxwm: cannot parse color %s\n", hex);
		return WhitePixel(dpy, DefaultScreen(dpy));
	}

	if (!XAllocColor(dpy, cmap, &col)) {
		fprintf(stderr, "sxwm: cannot allocate color %s\n", hex);
		return WhitePixel(dpy, DefaultScreen(dpy));
	}

	/* possibly unsafe BUT i dont think it can cause any problems.
	 * used to make sure borders are opaque with compositor like picom */
	return ((long)col.pixel) | (0xffL << 24);
}

void quit(void)
{
	/* Kill all clients on exit...

	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			XUnmapWindow(dpy, c->win);
			XKillClient(dpy, c->win);
		}
	}
	*/

	XSync(dpy, False);
	XFreeCursor(dpy, cursors.move);
	XFreeCursor(dpy, cursors.normal);
	XFreeCursor(dpy, cursors.resize);
	XCloseDisplay(dpy);
	puts("quitting...");
	running = False;
}

void reload_config(void)
{
	puts("sxwm: reloading config...");

	/* free binding commands without */
	for (int i = 0; i < user_config.n_binds; i++) {
		if (user_config.binds[i].type == TYPE_CMD && user_config.binds[i].action.cmd)
			free(user_config.binds[i].action.cmd);
		user_config.binds[i].action.cmd = NULL;
		user_config.binds[i].action.fn = NULL;
		user_config.binds[i].type = -1;
		user_config.binds[i].keysym = 0;
		user_config.binds[i].mods = 0;
	}

	for (int i = 0; i < MAX_ITEMS; i++) {
		if (user_config.open_in_workspace[i]) {
			if (user_config.open_in_workspace[i][0])
				free(user_config.open_in_workspace[i][0]);
			if (user_config.open_in_workspace[i][1])
				free(user_config.open_in_workspace[i][1]);
			free(user_config.open_in_workspace[i]);
			user_config.open_in_workspace[i] = NULL;
		}
		if (user_config.start_fullscreen[i]) {
			if (user_config.start_fullscreen[i][0])
				free(user_config.start_fullscreen[i][0]);
			free(user_config.start_fullscreen[i]);
			user_config.start_fullscreen[i] = NULL;
		}
	}

	/* free should_float arrays */
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (user_config.should_float[i]) {
			if (user_config.should_float[i][0])
				free(user_config.should_float[i][0]);
			free(user_config.should_float[i]);
			user_config.should_float[i] = NULL;
		}
	}

	/* free any exec strings */
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (user_config.to_run[i]) {
			free(user_config.to_run[i]);
			user_config.to_run[i] = NULL;
		}
	}

	/* wipe everything else */
	memset(&user_config, 0, sizeof(user_config));
	init_defaults();
	if (parser(&user_config)) {
		fprintf(stderr, "sxwmrc: error parsing config file\n");
		init_defaults();
	}

	/* regrab all key/button bindings */
	grab_keys();
	XUngrabButton(dpy, AnyButton, AnyModifier, root);
	for (int ws = 0; ws < NUM_WORKSPACES; ws++)
		for (Client *c = workspaces[ws]; c; c = c->next)
			XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	Mask root_click_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	Mask root_resize_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	grab_button(Button1, user_config.modkey, root, True, root_click_masks);
	grab_button(Button3, user_config.modkey, root, True, root_resize_masks);

	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			grab_button(Button1, None, c->win, False, ButtonPressMask);
			grab_button(Button1, user_config.modkey, c->win, False, ButtonPressMask);
			grab_button(Button2, user_config.modkey, c->win, False, ButtonPressMask);
		}
	}

	update_client_desktop_properties();
	update_net_client_list();
	XSync(dpy, False);

	tile();
	update_borders();
}

void resize_master_add(void)
{
	/* pick the monitor of the focused window (or 0 if none) */
	int m = focused ? focused->mon : 0;
	float *mw = &user_config.master_width[m];

	if (*mw < MF_MAX - 0.001f)
		*mw += ((float)user_config.resize_master_amt / 100);

	tile();
	update_borders();
}

void resize_master_sub(void)
{
	/* pick the monitor of the focused window (or 0 if none) */
	int m = focused ? focused->mon : 0;
	float *mw = &user_config.master_width[m];

	if (*mw > MF_MIN + 0.001f)
		*mw -= ((float)user_config.resize_master_amt / 100);

	tile();
	update_borders();
}

void resize_stack_add(void)
{
	if (!focused || focused->floating || focused == workspaces[current_ws])
		return;

	int bw2 = 2 * user_config.border_width;
	int raw_cur = (focused->custom_stack_height > 0) ? focused->custom_stack_height : (focused->h + bw2);

	int raw_new = raw_cur + user_config.resize_stack_amt;

	/* Calculate maximum allowed height to prevent extending off-screen */
	int mon = CLAMP(focused->mon, 0, n_mons - 1);
	int mon_height = MAX(1, mons[mon].h - mons[mon].res.top - mons[mon].res.bottom);
	int gaps = user_config.gaps;
	int tile_height = MAX(1, mon_height - 2 * gaps);

	/* Count stack windows (excluding master) */
	int n_stack = 0;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped && !c->floating && !c->fullscreen && c->mon == mon && c != workspaces[current_ws])
			n_stack++;
	}

	/* Maximum height: tile_height minus space for other stack windows (min_height + gap each) */
	int min_stack_height = bw2 + 1;
	int other_stack_space = (n_stack > 1) ? (n_stack - 1) * (min_stack_height + gaps) : 0;
	int max_raw = tile_height - other_stack_space;

	if (raw_new > max_raw)
		raw_new = max_raw;

	focused->custom_stack_height = raw_new;
	tile();
}

void resize_stack_sub(void)
{
	if (!focused || focused->floating || focused == workspaces[current_ws])
		return;

	int bw2 = 2 * user_config.border_width;
	int raw_cur = (focused->custom_stack_height > 0) ? focused->custom_stack_height : (focused->h + bw2);

	int raw_new = raw_cur - user_config.resize_stack_amt;
	int min_raw = bw2 + 1;

	if (raw_new < min_raw)
		raw_new = min_raw;

	focused->custom_stack_height = raw_new;
	tile();
}

void resize_win_down(void)
{
	if (!focused || !focused->floating)
		return;

	int new_h = focused->h + user_config.resize_window_amt;
	int max_h = mons[focused->mon].h - (focused->y - mons[focused->mon].y);
	focused->h = CLAMP(new_h, MIN_WINDOW_SIZE, max_h);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void resize_win_up(void)
{
	if (!focused || !focused->floating)
		return;

	int new_h = focused->h - user_config.resize_window_amt;
	focused->h = CLAMP(new_h, MIN_WINDOW_SIZE, focused->h);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void resize_win_right(void)
{
	if (!focused || !focused->floating)
		return;

	int new_w = focused->w + user_config.resize_window_amt;
	int max_w = mons[focused->mon].w - (focused->x - mons[focused->mon].x);
	focused->w = CLAMP(new_w, MIN_WINDOW_SIZE, max_w);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void resize_win_left(void)
{
	if (!focused || !focused->floating)
		return;

	int new_w = focused->w - user_config.resize_window_amt;
	focused->w = CLAMP(new_w, MIN_WINDOW_SIZE, focused->w);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void run(void)
{
	running = True;
	XEvent xev;
	while (running) {
		XNextEvent(dpy, &xev);
		xev_case(&xev);
	}
}

void scan_existing_windows(void)
{
	Window root_return;
	Window parent_return;
	Window *children;
	unsigned int n_children;

	if (XQueryTree(dpy, root, &root_return, &parent_return, &children, &n_children)) {
		for (unsigned int i = 0; i < n_children; i++) {
			XWindowAttributes wa;
			if (!XGetWindowAttributes(dpy, children[i], &wa)
				|| wa.override_redirect || wa.map_state != IsViewable)
				continue;

			XEvent fake_event = {None};
			fake_event.type = MapRequest;
			fake_event.xmaprequest.window = children[i];
			hdl_map_req(&fake_event);
		}
		if (children)
			XFree(children);
	}
}

void select_input(Window w, Mask masks)
{
	XSelectInput(dpy, w, masks);
}

void send_wm_take_focus(Window w)
{
	Atom wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	Atom wm_take_focus = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	Atom *protos;
	int n;

	if (XGetWMProtocols(dpy, w, &protos, &n)) {
		for (int i = 0; i < n; i++) {
			if (protos[i] == wm_take_focus) {
				XEvent ev = {
				    .xclient = {
						.type = ClientMessage,
						.window = w,
						.message_type = wm_protocols,
						.format = 32}
				};
				ev.xclient.data.l[0] = wm_take_focus;
				ev.xclient.data.l[1] = CurrentTime;
				XSendEvent(dpy, w, False, NoEventMask, &ev);
			}
		}
		XFree(protos);
	}
}

void setup(void)
{
	if ((dpy = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "can't open display.\nquitting...");
		exit(EXIT_FAILURE);
	}
	root = XDefaultRootWindow(dpy);

	setup_atoms();
	other_wm();
	init_defaults();
	if (parser(&user_config)) {
		fprintf(stderr, "sxwmrc: error parsing config file\n");
		init_defaults();
	}
	update_modifier_masks();
	grab_keys();
	startup_exec();

	cursors.normal = XcursorLibraryLoadCursor(dpy, "left_ptr");
	cursors.move = XcursorLibraryLoadCursor(dpy, "fleur");
	cursors.resize = XcursorLibraryLoadCursor(dpy, "bottom_right_corner");
	XDefineCursor(dpy, root, cursors.normal);

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));

	update_mons();

	/* select events wm should look for on root */
	Mask wm_masks = StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask |
	                KeyPressMask | PropertyChangeMask;
	select_input(root, wm_masks);

	/* grab mouse button events on root window */
	Mask root_click_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	Mask root_resize_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	grab_button(Button1, user_config.modkey, root, True, root_click_masks);
	grab_button(Button3, user_config.modkey, root, True, root_resize_masks);
	XSync(dpy, False);

	for (int i = 0; i < LASTEvent; i++)
		evtable[i] = hdl_dummy;
	evtable[ButtonPress] = hdl_button;
	evtable[ButtonRelease] = hdl_button_release;
	evtable[ClientMessage] = hdl_client_msg;
	evtable[ConfigureNotify] = hdl_config_ntf;
	evtable[ConfigureRequest] = hdl_config_req;
	evtable[DestroyNotify] = hdl_destroy_ntf;
	evtable[KeyPress] = hdl_keypress;
	evtable[MappingNotify] = hdl_mapping_ntf;
	evtable[MapRequest] = hdl_map_req;
	evtable[MotionNotify] = hdl_motion;
	evtable[PropertyNotify] = hdl_property_ntf;
	evtable[UnmapNotify] = hdl_unmap_ntf;
	scan_existing_windows();

	/* prevent child processes from becoming zombies */
	signal(SIGCHLD, SIG_IGN);
}

void setup_atoms(void)
{
	for (int i = 0; i < ATOM_COUNT; i++)
		atoms[i] = XInternAtom(dpy, atom_names[i], False);

	/* checking window */
	wm_check_win = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	/* root property -> child window */
	XChangeProperty(dpy, root, atoms[ATOM_NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32,
			        PropModeReplace, (unsigned char *)&wm_check_win, 1);
	/* child window -> child window */
	XChangeProperty(dpy, wm_check_win, atoms[ATOM_NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32,
			        PropModeReplace, (unsigned char *)&wm_check_win, 1);
	/* name the wm */
	const char *wmname = "sxwm";
	XChangeProperty(dpy, wm_check_win, atoms[ATOM_NET_WM_NAME], atoms[ATOM_UTF8_STRING], 8,
			        PropModeReplace, (const unsigned char *)wmname, strlen(wmname));

	/* workspace setup */
	long num_workspaces = NUM_WORKSPACES;
	XChangeProperty(dpy, root, atoms[ATOM_NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32,
			        PropModeReplace, (const unsigned char *)&num_workspaces, 1);

	const char workspace_names[] = WORKSPACE_NAMES;
	int names_len = sizeof(workspace_names);
	XChangeProperty(dpy, root, atoms[ATOM_NET_DESKTOP_NAMES], atoms[ATOM_UTF8_STRING], 8,
			        PropModeReplace, (const unsigned char *)workspace_names, names_len);

	XChangeProperty(dpy, root, atoms[ATOM_NET_CURRENT_DESKTOP], XA_CARDINAL, 32,
			        PropModeReplace, (const unsigned char *)&current_ws, 1);

	/* load supported list */
	XChangeProperty(dpy, root, atoms[ATOM_NET_SUPPORTED], XA_ATOM, 32,
			        PropModeReplace, (const unsigned char *)atoms, ATOM_COUNT);

	update_workarea();
}

void set_frame_extents(Window w)
{
	long extents[4] = {
		user_config.border_width,
		user_config.border_width,
		user_config.border_width,
		user_config.border_width
	};
	XChangeProperty(dpy, w, atoms[ATOM_NET_FRAME_EXTENTS], XA_CARDINAL, 32,
			        PropModeReplace, (unsigned char *)extents, 4);
}

void set_input_focus(Client *c, Bool raise_win, Bool warp)
{
	if (c && c->ws != current_ws)
		return;

	workspaces[current_ws].focused = (c && c->mapped) ? c : NULL;

	if (workspaces[current_ws].focused) {
		current_mon = CLAMP(c->mon, 0, n_mons - 1);
		Window w = find_toplevel(c->win);

		XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
		send_wm_take_focus(w);

		if (raise_win && (client_is_floating(c) || !user_config.floating_on_top))
			XRaiseWindow(dpy, w);

		/* EWMH focus hint */
		XChangeProperty(dpy, root, atoms[ATOM_NET_ACTIVE_WINDOW], XA_WINDOW, 32,
		                PropModeReplace, (unsigned char *)&w, 1);

		if (warp && user_config.warp_cursor)
			warp_cursor(c);
	}
	else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, atoms[ATOM_NET_ACTIVE_WINDOW]);
	}

	update_borders();
	XFlush(dpy);
}

void reset_opacity(Window w)
{
	Atom atom = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	XDeleteProperty(dpy, w, atom);
}


void set_opacity(Window w, double opacity)
{
	if (opacity < 0.0)
		opacity = 0.0;

	if (opacity > 1.0)
		opacity = 1.0;

	unsigned long op = (unsigned long)(opacity * 0xFFFFFFFFu);
	Atom atom = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	XChangeProperty(dpy, w, atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&op, 1);
}

void set_wm_state(Window w, long state)
{
	long data[2] = { state, None }; /* state, icon window */
	XChangeProperty(dpy, w, atoms[ATOM_WM_STATE], atoms[ATOM_WM_STATE], 32,
			        PropModeReplace, (unsigned char *)data, 2);
}

int snap_coordinate(int pos, int size, int screen_size, int snap_dist)
{
	if (UDIST(pos, 0) <= snap_dist)
		return 0;
	if (UDIST(pos + size, screen_size) <= snap_dist)
		return screen_size - size;
	return pos;
}

void spawn(const char * const *argv)
{
	int argc = 0;
	while (argv[argc])
		argc++;

	int cmd_count = 1;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "|") == 0)
			cmd_count++;
	}

	const char ***commands = malloc(cmd_count * sizeof(char **)); /* *** bruh */
	if (!commands) {
		perror("malloc commands");
		return;
	}

	/* initialize all command pointers to NULL for safe cleanup */
	for (int i = 0; i < cmd_count; i++)
		commands[i] = NULL;

	int cmd_idx = 0;
	int arg_start = 0;
	for (int i = 0; i <= argc; i++) {
		if (!argv[i] || strcmp(argv[i], "|") == 0) {
			int len = i - arg_start;
			const char **cmd_args = malloc((len + 1) * sizeof(char *));

			if (!cmd_args) {
				perror("malloc cmd_args");

				for (int j = 0; j < cmd_idx; j++)
					free(commands[j]);

				free(commands);
				return;
			}

			for (int j = 0; j < len; j++)
				cmd_args[j] = argv[arg_start + j];

			cmd_args[len] = NULL;
			commands[cmd_idx++] = cmd_args;
			arg_start = i + 1;
		}
	}

	int (*pipes)[2] = malloc(sizeof(int[2]) * (cmd_count - 1));
	if (!pipes) {
		perror("malloc pipes");

		for (int j = 0; j < cmd_count; j++)
			free(commands[j]);

		free(commands);
		return;
	}

	for (int i = 0; i < cmd_count - 1; i++) {
		if (pipe(pipes[i]) == -1) {
			perror("pipe");

			for (int j = 0; j < cmd_count; j++)
				free(commands[j]);

			free(commands);
			free(pipes);
			return;
		}
	}

	for (int i = 0; i < cmd_count; i++) {
		if (!commands[i] || !commands[i][0])
			continue;

		pid_t pid = fork();
		if (pid < 0) {
			perror("fork");

			for (int k = 0; k < cmd_count - 1; k++) {
				close(pipes[k][0]);
				close(pipes[k][1]);
			}

			for (int j = 0; j < cmd_count; j++)
				free(commands[j]);

			free(commands);
			free(pipes);
			return;
		}
		if (pid == 0) {
			close(ConnectionNumber(dpy));

			if (i > 0)
				dup2(pipes[i - 1][0], STDIN_FILENO);

			if (i < cmd_count - 1)
				dup2(pipes[i][1], STDOUT_FILENO);

			for (int k = 0; k < cmd_count - 1; k++) {
				close(pipes[k][0]);
				close(pipes[k][1]);
			}

			execvp(commands[i][0], (char* const*)(void*)commands[i]);
			fprintf(stderr, "sxwm: execvp '%s' failed\n", commands[i][0]);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < cmd_count - 1; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

	for (int i = 0; i < cmd_count; i++)
		free(commands[i]);

	free(commands);
	free(pipes);
}

void startup_exec(void)
{
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (user_config.to_run[i]) {
			const char **argv = build_argv(user_config.to_run[i]);
			if (argv) {
				spawn(argv);
				for (int j = 0; argv[j]; j++)
					free((void*)(uintptr_t)argv[j]);

				free(argv);
			}
		}
	}
}

void switch_previous_workspace(void)
{
	change_workspace(previous_workspace);
}

void switch_client_list(Client *c, Bool floating)
{
	if (!c || client_is_floating(c) == floating)
		return;

	if (c->fullscreen)
		apply_fullscreen(c, False);

	unlink_client(c);
	Client **head = floating ? &workspaces[c->ws].floating : &workspaces[c->ws].tiled;
	c->next = *head;
	if (*head)
		(*head)->prev = c;
	*head = c;

	if (floating) {
		XWindowAttributes wa;
		if (XGetWindowAttributes(dpy, c->win, &wa)) {
			c->x = wa.x;
			c->y = wa.y;
			c->w = wa.width;
			c->h = wa.height;
		}
		XRaiseWindow(dpy, c->win);
	}
	else
		c->mon = get_monitor_for_point(get_window_center(c));
}

void tile(void)
{
	update_struts();

	for (int m = 0; m < n_mons; m++) {
		Client *clients[MAX_CLIENTS];
		int n = 0;

		for (Client *c = workspaces[current_ws].tiled; c && n < MAX_CLIENTS; c = c->next)
			if (c->mapped && !c->fullscreen && c->mon == m)
				clients[n++] = c;

		if (!n)
			continue;

		Monitor *mon = &mons[m];
		int gaps = user_config.gaps;
		int x = mon->x + mon->res.left + gaps;
		int y = mon->y + mon->res.top + gaps;
		int w = MAX(1, mon->w - mon->res.left - mon->res.right - 2 * gaps);
		int h = MAX(1, mon->h - mon->res.top - mon->res.bottom - 2 * gaps);
		int master_w = n > 1 ? (int)(w * CLAMP(user_config.master_width[m], MF_MIN, MF_MAX)) : w;

		/* master */
		configure_tile(clients[0], x, y, master_w, h);

		if (n == 1)
			continue;

		/* stack */
		int stack_x = x + master_w + gaps;
		int stack_w = w - master_w - gaps;
		int n_stack = n - 1;
		int min_h = 2 * user_config.border_width + 1;
		int heights[MAX_CLIENTS] = {0};
		int available = h - (n_stack - 1) * gaps;
		int n_auto = 0;

		for (int i = 1; i < n; i++) {
			if (clients[i]->custom_stack_height > 0)
				available -= clients[i]->custom_stack_height;
			else
				n_auto++;
		}

		Bool enough = n_auto && available >= n_auto * min_h;
		int auto_h = enough ? available / n_auto : min_h;
		int auto_left = available;
		int auto_count = n_auto;

		for (int i = 1; i < n; i++) {
			if (clients[i]->custom_stack_height > 0) {
				heights[i] = clients[i]->custom_stack_height;
				continue;
			}

			heights[i] = enough && auto_count == 1 ? auto_left : auto_h;
			auto_left -= heights[i];
			auto_count--;
		}

		/* calculate occupied height */
		int occupied = (n_stack - 1) * gaps;
		for (int i = 1; i < n; i++)
			occupied += heights[i];

		/* shrink overflowing windows */
		for (int i = 1; i < n - 1 && occupied > h; i++) {
			int shrink = MIN(occupied - h, MAX(0, heights[i] - min_h));
			heights[i] -= shrink;
			occupied -= shrink;
		}

		/* bottom window takes unused space */
		if (occupied < h)
			heights[n - 1] += h - occupied;

		int stack_y = y;
		for (int i = 1; i < n; i++) {
			configure_tile(clients[i], stack_x, stack_y, stack_w, heights[i]);
			stack_y += heights[i] + gaps;
		}
	}

	update_borders();
}

void toggle_floating(void)
{
	Client *c = workspaces[current_ws].focused;
	if (!c)
		return;

	Bool floating = !client_is_floating(c);
	switch_client_list(c, floating);

	tile();
	update_borders();
	if (floating)
		set_input_focus(c, True, False);
}

void toggle_floating_global(void)
{
	global_floating = !global_floating;
	Bool floating = workspaces[current_ws].tiled != NULL;
	Client **source = floating ? &workspaces[current_ws].tiled : &workspaces[current_ws].floating;

	while (*source)
		switch_client_list(*source, floating);

	tile();
	update_borders();
}

void toggle_fullscreen(void)
{
	if (!focused)
		return;

	apply_fullscreen(focused, !focused->fullscreen);
}

void unlink_client(Client *c)
{
	if (!c || c->ws < 0 || c->ws >= NUM_WORKSPACES)
		return;

	Workspace *ws = &workspaces[c->ws];
	Client **head = client_is_floating(c) ? &ws->floating : &ws->tiled;

	if (c->prev)
		c->prev->next = c->next;
	else if (*head == c)
		*head = c->next;

	if (c->next)
		c->next->prev = c->prev;

	c->next = NULL;
	c->prev = NULL;
}

void update_borders(void)
{
	Client *focused = workspaces[current_ws].focused;

	for (Client *c = workspaces[current_ws].tiled; c; c = c->next)
		XSetWindowBorder(dpy, c->win, c == focused ? user_config.border_foc_col : user_config.border_ufoc_col);

	for (Client *c = workspaces[current_ws].floating; c; c = c->next)
		XSetWindowBorder(dpy, c->win, c == focused ? user_config.border_foc_col : user_config.border_ufoc_col);
}

void update_client_desktop_properties(void)
{
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			long desktop = ws;
			XChangeProperty(dpy, c->win, atoms[ATOM_NET_WM_DESKTOP], XA_CARDINAL, 32,
					        PropModeReplace, (unsigned char *)&desktop, 1);
		}
	}
}

void update_modifier_masks(void)
{
	XModifierKeymap *mod_mapping = XGetModifierMapping(dpy);
	KeyCode num = XKeysymToKeycode(dpy, XK_Num_Lock);
	KeyCode mode = XKeysymToKeycode(dpy, XK_Mode_switch);
	numlock_mask = 0;
	mode_switch_mask = 0;

	int n_masks = 8;
	for (int i = 0; i < n_masks; i++) {
		for (int j = 0; j < mod_mapping->max_keypermod; j++) {
			/* keycode at mod[i][j] */
			KeyCode keycode = mod_mapping->modifiermap[i * mod_mapping->max_keypermod + j];
			if (keycode == num)
				numlock_mask = (1u << i); /* which mod bit == NumLock key */
			if (keycode == mode)
				mode_switch_mask = (1u << i); /* which mod bit == Mode_switch key */
		}
	}
	XFreeModifiermap(mod_mapping);
}

void update_mons(void)
{
	XineramaScreenInfo *info;
	Monitor *old = mons;

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));

	for (int s = 0; s < ScreenCount(dpy); s++) {
		Window scr_root = RootWindow(dpy, s);
		XDefineCursor(dpy, scr_root, cursors.normal);
	}

	if (XineramaIsActive(dpy)) {
		info = XineramaQueryScreens(dpy, &n_mons);
		mons = malloc(sizeof *mons * n_mons);
		if (!mons) {
			fputs("sxwm: failed to allocate monitors\n", stderr);
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < n_mons; i++) {
			mons[i].x = info[i].x_org;
			mons[i].y = info[i].y_org;
			mons[i].w = info[i].width;
			mons[i].h = info[i].height;
		}
		XFree(info);
	}
	else {
		n_mons = 1;
		mons = malloc(sizeof *mons);
		if (!mons) {
			fputs("sxwm: failed to allocate monitor\n", stderr);
			exit(EXIT_FAILURE);
		}
		mons[0].x = 0;
		mons[0].y = 0;
		mons[0].w = scr_width;
		mons[0].h = scr_height;
	}

	free(old);
}

void update_net_client_list(void)
{
	Window wins[MAX_CLIENTS];
	int n = 0;
	for (int ws = 0; ws < NUM_WORKSPACES; ws++)
		for (Client *c = workspaces[ws]; c; c = c->next)
			wins[n++] = c->win;

	XChangeProperty(dpy, root, atoms[ATOM_NET_CLIENT_LIST], XA_WINDOW, 32, PropModeReplace, (unsigned char *)wins, n);
}

void update_struts(void)
{
	/* reset all reserves */
	for (int i = 0; i < n_mons; i++) {
		mons[i].res.left   = 0;
		mons[i].res.right  = 0;
		mons[i].res.top    = 0;
		mons[i].res.bottom = 0;
	}

	Window root_ret;
	Window parent_ret;
	Window *children = NULL;
	unsigned int n_children = 0;

	if (!XQueryTree(dpy, root, &root_ret, &parent_ret, &children, &n_children))
		return;

	int screen_w = scr_width;
	int screen_h = scr_height;

	for (unsigned int i = 0; i < n_children; i++) {
		Window w = children[i];

		if (get_window_type(w) != WINDOW_DOCK)
			continue;

		long *str = NULL;
		Atom actual;
		int sfmt;
		unsigned long len;
		unsigned long rem;

		if (XGetWindowProperty(dpy, w, atoms[ATOM_NET_WM_STRUT_PARTIAL], 0, 12, False, XA_CARDINAL,
					&actual, &sfmt, &len, &rem,
					(unsigned char **)&str) == Success && str && len >= 12) {

			/*
			 ewmh:
			 [0] left, [1] right, [2] top, [3] bottom
			 
			 [4] left_start_y,   [5] left_end_y
			 [6] right_start_y,  [7] right_end_y
			 [8] top_start_x,    [9] top_end_x
			 [10] bottom_start_x,[11] bottom_end_x
			 
			 all coords are in root space.
			 */
			long left = str[0];
			long right = str[1];
			long top = str[2];
			long bottom = str[3];
			long left_start_y = str[4];
			long left_end_y = str[5];
			long right_start_y = str[6];
			long right_end_y = str[7];
			long top_start_x = str[8];
			long top_end_x = str[9];
			long bot_start_x = str[10];
			long bot_end_x = str[11];

			XFree(str);

			/* skip empty struts */
			if (!left && !right && !top && !bottom)
				continue;

			for (int m = 0; m < n_mons; m++) {
				int mx = mons[m].x;
				int my = mons[m].y;
				int mw = mons[m].w;
				int mh = mons[m].h;

				/* strip monitors whose vertical span dostn intersect */
				if (left > 0) {
					long span_start = left_start_y;
					long span_end   = left_end_y;
					if (span_end >= my && span_start <= my + mh - 1) {
						/*
						 left is distance from root left edge to reserved area
						 to map to mon, the portion is:
						     reserve_left = MAX(0, left - mx)
						 */
						int reserve = (int)MAX(0, left - mx);
						if (reserve > 0)
							mons[m].res.left = MAX(mons[m].res.left, reserve);
					}
				}

				if (right > 0) {
					long span_start = right_start_y;
					long span_end   = right_end_y;
					if (span_end >= my && span_start <= my + mh - 1) {
						/*
						 right is distance from root right edge to reserved area:
						     right edge = screen_w
							 mons right edge = mx + mw
							 amount that cuts into monitor = MAX(0, (screen_w - right) - mx)
						 */
						int global_reserved_left = screen_w - (int)right;
						int overlap = (mx + mw) - global_reserved_left;
						int reserve = MAX(0, overlap);
						if (reserve > 0)
							mons[m].res.right = MAX(mons[m].res.right, reserve);
					}
				}

				if (top > 0) {
					long span_start = top_start_x;
					long span_end   = top_end_x;
					if (span_end >= mx && span_start <= mx + mw - 1) {
						/*
						 top is distance from root top to reserved area
							 mons top is at my, amount eaten:
							 reserve_top = MAX(0, top - my)
						 */
						int reserve = (int)MAX(0, top - my);
						if (reserve > 0)
							mons[m].res.top = MAX(mons[m].res.top, reserve);
					}
				}

				if (bottom > 0) {
					long span_start = bot_start_x;
					long span_end   = bot_end_x;
					if (span_end >= mx && span_start <= mx + mw - 1) {
						/*
						 bottom is distance from root bottom to reserved area
						 global_reserved_top = screen_h - bottom;
						 overlap to mon:
						   overlap = (my + mh) - global_reserved_top;
						   reserve_bottom = MAX(0, overlap)
						 */
						int global_reserved_top = screen_h - (int)bottom;
						int overlap = (my + mh) - global_reserved_top;
						int reserve = MAX(0, overlap);
						if (reserve > 0)
							mons[m].res.bottom = MAX(mons[m].res.bottom, reserve);
					}
				}
			}
		}
	}

	if (children)
		XFree(children);

	update_workarea();
}

void update_workarea(void)
{
	long workarea[4 * MAX_MONITORS];

	for (int i = 0; i < n_mons && i < MAX_MONITORS; i++) {
		workarea[i * 4 + 0] = mons[i].x + mons[i].res.left;
		workarea[i * 4 + 1] = mons[i].y + mons[i].res.top;
		workarea[i * 4 + 2] = mons[i].w - mons[i].res.left - mons[i].res.right;
		workarea[i * 4 + 3] = mons[i].h - mons[i].res.top - mons[i].res.bottom;
	}

	XChangeProperty(dpy, root, atoms[ATOM_NET_WORKAREA], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)workarea, n_mons * 4);
}

void warp_cursor(Client *c)
{
	if (!c)
		return;

	int center_x = c->x + (c->w / 2);
	int center_y = c->y + (c->h / 2);

	XWarpPointer(dpy, None, root, 0, 0, 0, 0, center_x, center_y);
	XSync(dpy, False);
}

Bool window_has_ewmh_state(Window w, Atom state)
{
	Atom type;
	int format;
	unsigned long n_atoms = 0;
	unsigned long unread = 0;
	Atom *found_atoms = NULL;

	if (XGetWindowProperty(dpy, w, atoms[ATOM_NET_WM_STATE], 0, 1024, False, XA_ATOM, &type,
		&format, &n_atoms, &unread, (unsigned char**)&found_atoms) == Success && found_atoms) {

		for (unsigned long i = 0; i < n_atoms; i++) {
			if (found_atoms[i] == state) {
				XFree(found_atoms);
				return True;
			}
		}
		XFree(found_atoms);
	}
	return False;
}

Bool window_matches_class(Window w, char ***rules)
{
	XClassHint ch = { 0 };
	if (!XGetClassHint(dpy, w, &ch))
		return False;

	Bool matched = False;
	for (int i = 0; i < MAX_ITEMS && rules[i] && rules[i][0]; i++) {
		const char *name = rules[i][0];
		if ((ch.res_class && !strcmp(ch.res_class, name)) || (ch.res_name && !strcmp(ch.res_name, name))) {
			matched = True;
			break;
		}
	}

	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);

	return matched;
}

void window_set_ewmh_state(Window w, Atom state, Bool add)
{
	Atom type;
	int format;
	unsigned long n_atoms = 0;
	unsigned long unread = 0;
	Atom *found_atoms = NULL;

	if (XGetWindowProperty(dpy, w, atoms[ATOM_NET_WM_STATE], 0, 1024, False, XA_ATOM, &type,
		&format, &n_atoms, &unread, (unsigned char**)&found_atoms) != Success) {
		found_atoms = NULL;
		n_atoms = 0;
	}

	/* build new list */
	Atom buf[16];
	Atom *list = buf;
	unsigned long list_len = 0;

	if (found_atoms) {
		for (unsigned long i = 0; i < n_atoms; i++) {
			if (found_atoms[i] != state)
				list[list_len++] = found_atoms[i];	
		}
	}
	if (add && list_len < 16)
		list[list_len++] = state;

	if (list_len == 0)
		XDeleteProperty(dpy, w, atoms[ATOM_NET_WM_STATE]);
	else
		XChangeProperty(dpy, w, atoms[ATOM_NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (unsigned char*)list, list_len);

	if (found_atoms)
		XFree(found_atoms);	
}

Bool window_should_float(Window w)
{
	return window_matches_class(w, user_config.should_float);
}

Bool window_should_start_fullscreen(Window w)
{
	return window_matches_class(w, user_config.start_fullscreen);
}

int xerr(Display *d, XErrorEvent *ee)
{
	/* ignore noise & non fatal errors */
	const struct {
		int req, code;
	} ignore[] = {
		{0, BadWindow},
		{X_GetGeometry, BadDrawable},
		{X_SetInputFocus, BadMatch},
		{X_ConfigureWindow, BadMatch},
	};

	for (size_t i = 0; i < sizeof(ignore) / sizeof(ignore[0]); i++) {
		if ((ignore[i].req == 0 || ignore[i].req == ee->request_code) && (ignore[i].code == ee->error_code))
			return 0;
	}

	return 0;
	(void)d;
	(void)ee;
}

void xev_case(XEvent *xev)
{
	if (xev->type >= 0 && xev->type < LASTEvent)
		evtable[xev->type](xev);
	else
		fprintf(stderr, "sxwm: invalid event type: %d\n", xev->type);
}

int main(int ac, char **av)
{
	if (ac > 1) {
		if (strcmp(av[1], "-v") == 0 || strcmp(av[1], "--version") == 0) {
			printf("%s\n%s\n%s\n", SXWM_VERSION, SXWM_AUTHOR, SXWM_LICINFO);
			return EXIT_SUCCESS;
		}
		else {
			printf("usage:\n");
			printf("\t[-v || --version]: See the version of sxwm\n");
			return EXIT_SUCCESS;
		}
	}
	setup();
	puts("sxwm: starting...");
	run();
	return EXIT_SUCCESS;
}

