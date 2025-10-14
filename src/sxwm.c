/*  See LICENSE for more info
 *
 *  simple xorg window manager:
 *  sxwm is a user-friendly, easily configurable yet powerful
 *  tiling window manager inspired by window managers such as
 *  DWM and i3.
 *
 *  The userconfig is designed to be as user-friendly as
 *  possible, and I hope it is easy to configure even without
 *  knowledge of C or programming, although most people who
 *  will use this will probably be programmers :)
 *
 *  (C) Abhinav Prasai 2025
*/

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

#include "defs.h"
#include "extern.h"
#include "parser.h"

Client *add_client(Window w, int ws);
void apply_fullscreen(Client *c, Bool on);
/* void centre_window(void); */
void change_workspace(int ws);
int check_parent(pid_t p, pid_t c);
int clean_mask(int mask);
/* void close_focused(void); */
/* void dec_gaps(void); */
Client *find_client(Window w);
Window find_toplevel(Window w);
/* void focus_next(void); */
/* void focus_prev(void); */
/* void focus_next_mon(void); */
/* void focus_prev_mon(void); */
int get_monitor_for(Client *c);
pid_t get_parent_process(pid_t c);
pid_t get_pid(Window w);
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
Bool is_child_proc(pid_t pid1, pid_t pid2);
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
void remove_scratchpad(int n);
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
void set_win_scratchpad(int n);
void set_wm_state(Window w, long state);
int snap_coordinate(int pos, int size, int screen_size, int snap_dist);
void spawn(const char * const *argv);
void startup_exec(void);
void swallow_window(Client *swallower, Client *swallowed);
void swap_clients(Client *a, Client *b);
/* void switch_previous_workspace(void); */
void tile(void);
/* void toggle_floating(void); */
/* void toggle_floating_global(void); */
/* void toggle_fullscreen(void); */
/* void toggle_monocle(void); */
void toggle_scratchpad(int n);
void unswallow_window(Client *c);
void update_borders(void);
void update_client_desktop_properties(void);
void update_modifier_masks(void);
void update_mons(void);
void update_net_client_list(void);
void update_struts(void);
void update_workarea(void);
void warp_cursor(Client *c);
Bool window_has_ewmh_state(Window w, Atom state);
void window_set_ewmh_state(Window w, Atom state, Bool add);
Bool window_should_float(Window w);
Bool window_should_start_fullscreen(Window w);
int xerr(Display *d, XErrorEvent *ee);
void xev_case(XEvent *xev);

Atom _NET_ACTIVE_WINDOW;
Atom _NET_CURRENT_DESKTOP;
Atom _NET_SUPPORTED;
Atom _NET_WM_STATE;
Atom _NET_WM_STATE_FULLSCREEN;
Atom WM_STATE;
Atom _NET_WM_WINDOW_TYPE;
Atom _NET_WORKAREA;
Atom WM_DELETE_WINDOW;
Atom _NET_WM_STRUT;
Atom _NET_WM_STRUT_PARTIAL;
Atom _NET_SUPPORTING_WM_CHECK;
Atom _NET_WM_NAME;
Atom UTF8_STRING;

Atom _NET_WM_DESKTOP;
Atom _NET_CLIENT_LIST;
Atom _NET_FRAME_EXTENTS;
Atom _NET_NUMBER_OF_DESKTOPS;
Atom _NET_DESKTOP_NAMES;
Atom _NET_WM_PID;

Atom _NET_WM_WINDOW_TYPE_DOCK;
Atom _NET_WM_WINDOW_TYPE_UTILITY;
Atom _NET_WM_WINDOW_TYPE_DIALOG;
Atom _NET_WM_WINDOW_TYPE_TOOLBAR;
Atom _NET_WM_WINDOW_TYPE_SPLASH;
Atom _NET_WM_WINDOW_TYPE_POPUP_MENU;
Atom _NET_WM_WINDOW_TYPE_MENU;
Atom _NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
Atom _NET_WM_WINDOW_TYPE_TOOLTIP;
Atom _NET_WM_WINDOW_TYPE_NOTIFICATION;
Atom _NET_WM_STATE_MODAL;

Cursor cursor_normal;
Cursor cursor_move;
Cursor cursor_resize;

Client *workspaces[NUM_WORKSPACES] = {NULL};
Config user_config;
DragMode drag_mode = DRAG_NONE;
Client *drag_client = NULL;
Client *swap_target = NULL;
Client *focused = NULL;
EventHandler evtable[LASTEvent];
Display *dpy;
Window root;
Window wm_check_win;
Monitor *mons = NULL;
Scratchpad scratchpads[MAX_SCRATCHPADS];
int scratchpad_count = 0;
int current_scratchpad = 0;
int n_mons = 0;
int previous_workspace = 0;
int current_ws = 0;
int current_mon = 0;
long last_motion_time = 0;
Bool global_floating = False;
Bool in_ws_switch = False;
Bool running = False;
Bool monocle = False;

Mask numlock_mask = 0;
Mask mode_switch_mask = 0;

int scr_width;
int scr_height;
int open_windows = 0;
int drag_start_x, drag_start_y;
int drag_orig_x, drag_orig_y, drag_orig_w, drag_orig_h;

int reserve_left = 0;
int reserve_right = 0;
int reserve_top = 0;
int reserve_bottom = 0;

Client *add_client(Window w, int ws)
{
	Client *c = malloc(sizeof(Client));
	if (!c) {
		fprintf(stderr, "sxwm: could not alloc memory for client\n");
		return NULL;
	}

	c->win = w;
	c->next = NULL;
	c->ws = ws;
	c->pid = get_pid(w);
	c->swallowed = NULL;
	c->swallower = NULL;

	if (!workspaces[ws]) {
		workspaces[ws] = c;
	}
	else {
		if (user_config.new_win_master) {
			c->next = workspaces[ws];
			workspaces[ws] = c;
		}
		else {
			Client *tail = workspaces[ws];
			while (tail->next) {
				tail = tail->next;
			}
			tail->next = c;
		}
	}
	open_windows++;

	/* subscribing to certain events */
	Mask window_masks = EnterWindowMask | LeaveWindowMask | FocusChangeMask | PropertyChangeMask |
		                StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	select_input(w, window_masks);
	grab_button(Button1, None, w, False, ButtonPressMask);
	grab_button(Button1, user_config.modkey, w, False, ButtonPressMask);
	grab_button(Button1, user_config.modkey | ShiftMask, w, False, ButtonPressMask);
	grab_button(Button3, user_config.modkey, w, False, ButtonPressMask);

	/* allow for more graceful exitting */
	Atom protos[] = {WM_DELETE_WINDOW};
	XSetWMProtocols(dpy, w, protos, 1);

	XWindowAttributes wa;
	XGetWindowAttributes(dpy, w, &wa);
	c->x = wa.x;
	c->y = wa.y;
	c->w = wa.width;
	c->h = wa.height;

	/* set monitor based on cursor location */
	Window root_ret, child_ret;
	int root_x, root_y,
		win_x, win_y;
	unsigned int masks;
	int cursor_mon = 0;

	if (XQueryPointer(dpy, root, &root_ret, &child_ret, &root_x, &root_y, &win_x, &win_y, &masks)) {
		for (int i = 0; i < n_mons; i++) {
			Bool in_mon = root_x >= mons[i].x &&
				          root_x < mons[i].x + mons[i].w &&
				          root_y >= mons[i].y &&
			              root_y < mons[i].y + mons[i].h;
			if (in_mon) {
				cursor_mon = i;
				break;
			}
		}
	}

	/* set client defaults */
	c->mon = cursor_mon;
	c->fixed = False;
	c->floating = False;
	c->fullscreen = False;
	c->mapped = True;
	c->custom_stack_height = 0;

	if (global_floating) {
		c->floating = True;
	}

	if (ws == current_ws && !focused) {
		focused = c;
		current_mon = c->mon;
	}

	/* associate client with workspace n */
	long desktop = ws;
	XChangeProperty(dpy, w, _NET_WM_DESKTOP, XA_CARDINAL, 32,
			        PropModeReplace, (unsigned char *)&desktop, 1);
	XRaiseWindow(dpy, w);
	return c;
}

void apply_fullscreen(Client *c, Bool on)
{
	if (!c || !c->mapped || c->fullscreen == on) {
		return;
	}


	if (on) {
		XWindowAttributes win_attr;
		XGetWindowAttributes(dpy, c->win, &win_attr);

		c->orig_x = win_attr.x;
		c->orig_y = win_attr.y;
		c->orig_w = win_attr.width;
		c->orig_h = win_attr.height;

		c->fullscreen = True;

		int mon = c->mon;
		/* make window fill mon */
		XSetWindowBorderWidth(dpy, c->win, 0);
		XMoveResizeWindow(dpy, c->win, mons[mon].x, mons[mon].y, mons[mon].w, mons[mon].h);
		XRaiseWindow(dpy, c->win);
		window_set_ewmh_state(c->win, _NET_WM_STATE_FULLSCREEN, True);
	}
	else {
		c->fullscreen = False;

		/* restore win attributes */
		XMoveResizeWindow(dpy, c->win, c->orig_x, c->orig_y, c->orig_w, c->orig_h);
		XSetWindowBorderWidth(dpy, c->win, user_config.border_width);
		window_set_ewmh_state(c->win, _NET_WM_STATE_FULLSCREEN, False);

		if (!c->floating) {
			c->mon = get_monitor_for(c);
		}
		tile();
		update_borders();
	}
}

void centre_window(void)
{
	if (!focused || !focused->mapped || !focused->floating) {
		return;
	}

	focused->mon = get_monitor_for(focused);
	int x = mons[focused->mon].x + (mons[focused->mon].w - focused->w) / 2;
	int y = mons[focused->mon].y + (mons[focused->mon].h - focused->h) / 2;
	x -= user_config.border_width;
	y -= user_config.border_width;

	focused->x = x;
	focused->y = y;
	XMoveWindow(dpy, focused->win, x, y);
}

void change_workspace(int ws)
{
	if (ws >= NUM_WORKSPACES || ws == current_ws) {
		return;
	}

	in_ws_switch = True;
	XGrabServer(dpy); /* freeze rendering for tearless switching */

	/* scratchpads stay visible */
	Bool visible_scratchpads[MAX_SCRATCHPADS] = {False};
	for (int i = 0; i < MAX_SCRATCHPADS; i++) {
		if (scratchpads[i].client && scratchpads[i].enabled) {
			visible_scratchpads[i] = True;
			XUnmapWindow(dpy, scratchpads[i].client->win);
			scratchpads[i].client->mapped = False;
		}
	}

	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped) {
			/* TODO: Turn into helper */
			Bool is_scratchpad = False;
			for (int i = 0; i < MAX_SCRATCHPADS; i++) {
				if (scratchpads[i].client == c) {
					is_scratchpad = True;
					break;
				}
			}
			if (!is_scratchpad) {
				XUnmapWindow(dpy, c->win);
			}
		}
	}

	previous_workspace = current_ws;
	current_ws = ws;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped) {
			/* TODO: Turn into helper */
			Bool is_scratchpad = False;
			for (int i = 0; i < MAX_SCRATCHPADS; i++) {
				if (scratchpads[i].client == c) {
					is_scratchpad = True;
					break;
				}
			}
			if (!is_scratchpad) {
				XMapWindow(dpy, c->win);
			}
		}
	}

	/* move visible scratchpads to new workspace and map them */
	for (int i = 0; i < MAX_SCRATCHPADS; i++) {
		if (visible_scratchpads[i] && scratchpads[i].client) {
			Client *c = scratchpads[i].client;

			/* remove from old workspace */
			Client **pp = &workspaces[c->ws];
			while (*pp && *pp != c) {
				pp = &(*pp)->next;
			}
			if (*pp) {
				*pp = c->next;
			}

			/* add to new workspace */
			c->next = workspaces[current_ws];
			workspaces[current_ws] = c;
			c->ws = current_ws;

			XMapWindow(dpy, c->win);
			c->mapped = True;
			XRaiseWindow(dpy, c->win);

			/* Update desktop property */
			long desktop = current_ws;
			XChangeProperty(dpy, c->win, _NET_WM_DESKTOP, XA_CARDINAL, 32,
					        PropModeReplace, (unsigned char *)&desktop, 1);
		}
	}

	tile();
	focused = NULL;

	for (int i = 0; i < MAX_SCRATCHPADS; i++) {
		if (visible_scratchpads[i] && scratchpads[i].client) {
			focused = scratchpads[i].client;
			break;
		}
	}

	/* if no scratchpad found focus regular window */
	if (!focused && workspaces[current_ws]) {
		for (Client *c = workspaces[current_ws]; c; c = c->next) {
			if (c->mon == current_mon) {
				focused = c;
				current_mon = c->mon;
				break;
			}
			focused = c;
			current_mon = c->mon;
		}
	}
	set_input_focus(focused, False, True);

	long current_desktop = current_ws;
	XChangeProperty(dpy, root, _NET_CURRENT_DESKTOP, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *)&current_desktop, 1);
	update_client_desktop_properties();

	XUngrabServer(dpy);
	XSync(dpy, False);
	in_ws_switch = False;
}

int check_parent(pid_t p, pid_t c)
{
	while (p != c && c != 0) { /* walk proc tree until parent found */
		c = get_parent_process(c);
	}
	return (int)c;
}

int clean_mask(int mask)
{
	return mask & ~(LockMask | numlock_mask | mode_switch_mask);
}

void close_focused(void)
{
	if (!focused) {
		return;
	}

	for (int i = 0; i < MAX_SCRATCHPADS; i++) {
		if (scratchpads[i].client == focused) {
			scratchpads[i].client = NULL;
			scratchpads[i].enabled = False;
			break;
		}
	}

	Atom *protocols;
	int n_protocols;
	/* get number of protocols a window possesses and check if any == WM_DELETE_WINDOW (supports it) */
	if (XGetWMProtocols(dpy, focused->win, &protocols, &n_protocols) && protocols) {
		for (int i = 0; i < n_protocols; i++) {
			if (protocols[i] == WM_DELETE_WINDOW) {
				Atom WM_PROTOCOLS =  XInternAtom(dpy, "WM_PROTOCOLS", False);
				XEvent ev = {.xclient = {
					.type = ClientMessage,
					.window = focused->win,
					.message_type = WM_PROTOCOLS,
					.format = 32}};

				ev.xclient.data.l[0] = WM_DELETE_WINDOW;
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
		for (Client *c = workspaces[ws]; c; c = c->next) {
			if (c->win == w) {
				return c;
			}
		}
	}
	return NULL;
}

Window find_toplevel(Window w)
{
	Window root_win = None;
	Window parent;
	Window *kids;
	unsigned n_kids;

	while (True) {
		if (w == root_win) {
			break;
		}
		if (XQueryTree(dpy, w, &root_win, &parent, &kids, &n_kids) == 0) {
			break;
		}
		XFree(kids);
		if (parent == root_win || parent == None) {
			break;
		}
		w = parent;
	}
	return w;
}

void focus_next(void)
{
	if (!workspaces[current_ws]) {
		return;
	}

	Client *start = focused ? focused : workspaces[current_ws];
	Client *c = start;

	/* loop until we find a mapped client or return to start */
	do {
		c = c->next ? c->next : workspaces[current_ws];
	} while (( !c->mapped || c->mon != current_mon ) && c != start);

	/* if we return to start: */
	if (!c->mapped || c->mon != current_mon) {
		return;
	}

	focused = c;
	current_mon = c->mon;
	set_input_focus(focused, True, True);
}

void focus_prev(void)
{
	if (!workspaces[current_ws]) {
		return;
	}

	Client *start = focused ? focused : workspaces[current_ws];
	Client *c = start;

	/* loop until we find a mapped client or return to starting point */
	do {
		Client *p = workspaces[current_ws];
		Client *prev = NULL;
		while (p && p != c) {
			prev = p;
			p = p->next;
		}

		if (prev) {
			c = prev;
		}
		else {
			/* wrap to tail */
			p = workspaces[current_ws];
			while (p->next)
				p = p->next;
			c = p;
		}
	} while (( !c->mapped || c->mon != current_mon ) && c != start);

	/* this stops invisible windows being detected or focused */
	if (!c->mapped || c->mon != current_mon) {
		return;
	}

	focused = c;
	current_mon = c->mon;
	set_input_focus(focused, True, True);
}

void focus_next_mon(void)
{
	if (n_mons <= 1) {
		return;
	}

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
	if (n_mons <= 1) {
		return; /* only one monitor, nothing to switch to */
	}

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

int get_monitor_for(Client *c)
{
	int cx = c->x + c->w / 2;
	int cy = c->y + c->h / 2;
	for (int i = 0; i < n_mons; i++) {
		Bool in_mon_bounds =
			cx >= mons[i].x &&
			cx < mons[i].x + mons[i].w &&
			cy >= (int)mons[i].y &&
			cy < mons[i].y + mons[i].h;

		if (in_mon_bounds) {
			return i;
		}
	}
	return 0;
}

pid_t get_parent_process(pid_t c)
{
	pid_t v = -1;
	FILE *f;
	char buf[256];

	snprintf(buf, sizeof(buf), "/proc/%u/stat", (unsigned)c);
	if (!(f = fopen(buf, "r"))) {
		return 0;
	}

	int no_error = fscanf(f, "%*u %*s %*c %u", &v);
	(void)no_error;
	fclose(f);
	return (pid_t)v;
}

pid_t get_pid(Window w)
{
	pid_t pid = 0;
	Atom actual_type;
	int actual_format;
	unsigned long n_items, bytes_after;
	unsigned char *prop = NULL;

	if (XGetWindowProperty(dpy, w, _NET_WM_PID, 0, 1, False, XA_CARDINAL, &actual_type,
				           &actual_format, &n_items, &bytes_after, &prop) == Success && prop) {
		if (actual_format == 32 && n_items == 1) {
			pid = *(pid_t *)prop;
		}
		XFree(prop);
	}
	return pid;
}

int get_workspace_for_window(Window w)
{
	XClassHint ch = {0};
	if (!XGetClassHint(dpy, w, &ch)) {
		return current_ws;
	}

	for (int i = 0; i < MAX_ITEMS; i++) {
		/* TODO: Add docs for open_in_workspace */
		if (!user_config.open_in_workspace[i]) {
			break;
		}

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
	if (w == root) { /* grabbing for wm */
		XGrabButton(dpy, button, mod, w, owner_events, masks, GrabModeAsync, GrabModeAsync, None, None);
	}
	else { /* grabbing for windows */
		XGrabButton(dpy, button, mod, w, owner_events, masks, GrabModeSync, GrabModeAsync, None, None);
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
		if (!bind->keycode) {
			continue;
		}

		for (size_t guard = 0; guard < sizeof(guards)/sizeof(guards[0]); guard++) {
			XGrabKey(dpy, bind->keycode, bind->mods | guards[guard],
					root, True, GrabModeAsync, GrabModeAsync);
		}
	}
}

void hdl_button(XEvent *xev)
{
	XButtonEvent *xbutton = &xev->xbutton;
	Window w = (xbutton->subwindow != None) ? xbutton->subwindow : xbutton->window;
	w = find_toplevel(w);

	Mask left_click = Button1;
	Mask right_click = Button3;

	XAllowEvents(dpy, ReplayPointer, xbutton->time);
	if (!w) {
		return;
	}

	Client *head = workspaces[current_ws];
	for (Client *c = head; c; c = c->next) {
		if (c->win != w) {
			continue;
		}

		Bool is_swap_mode =
			(xbutton->state & user_config.modkey) &&
			(xbutton->state & ShiftMask) &&
			xbutton->button == left_click && !c->floating;
		if (is_swap_mode) {
			drag_client = c;
			drag_start_x = xbutton->x_root;
			drag_start_y = xbutton->y_root;
			drag_orig_x = c->x;
			drag_orig_y = c->y;
			drag_orig_w = c->w;
			drag_orig_h = c->h;
			drag_mode = DRAG_SWAP;
			XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask,
					     GrabModeAsync, GrabModeAsync, None, cursor_move, CurrentTime);
			focused = c;
			set_input_focus(focused, False, False);
			XSetWindowBorder(dpy, c->win, user_config.border_swap_col);
			return;
		}

		Bool is_move_resize =
			(xbutton->state & user_config.modkey) &&
			(xbutton->button == left_click ||
			 xbutton->button == right_click) && !c->floating;
		if (is_move_resize) {
			focused = c;
			toggle_floating();
		}

		Bool is_single_click = 
			!(xbutton->state & user_config.modkey) &&
			xbutton->button == left_click;
		if (is_single_click) {
			focused = c;
			set_input_focus(focused, True, False);
			return;
		}

		if (!c->floating) {
			return;
		}

		if (c->fixed && xbutton->button == right_click) {
			return;
		}

		Cursor cursor = (xbutton->button == left_click) ? cursor_move : cursor_resize;
		XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask,
				     GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);

		drag_client = c;
		drag_start_x = xbutton->x_root;
		drag_start_y = xbutton->y_root;
		drag_orig_x = c->x;
		drag_orig_y = c->y;
		drag_orig_w = c->w;
		drag_orig_h = c->h;
		drag_mode = (xbutton->button == left_click) ? DRAG_MOVE : DRAG_RESIZE;
		focused = c;

		set_input_focus(focused, True, False);
		return;
	}
}

void hdl_button_release(XEvent *xev)
{
	(void)xev;

	if (drag_mode == DRAG_SWAP) {
		if (swap_target) {
			XSetWindowBorder(dpy, swap_target->win, (swap_target == focused ?
						     user_config.border_foc_col : user_config.border_ufoc_col));
			swap_clients(drag_client, swap_target);
		}
		tile();
		update_borders();
	}

	XUngrabPointer(dpy, CurrentTime);

	drag_mode = DRAG_NONE;
	drag_client = NULL;
	swap_target = NULL;
}

void hdl_client_msg(XEvent *xev)
{
	if (xev->xclient.message_type == _NET_CURRENT_DESKTOP) {
		int ws = (int)xev->xclient.data.l[0];
		change_workspace(ws);
		return;
	}

	if (xev->xclient.message_type == _NET_WM_STATE) {
		XClientMessageEvent *client_msg_ev = &xev->xclient;
		Window w = client_msg_ev->window;
		Client *c = find_client(find_toplevel(w));
		if (!c) {
			return;
		}

		/* 0=remove, 1=add, 2=toggle */
		long action = client_msg_ev->data.l[0];
		Atom a1 = (Atom)client_msg_ev->data.l[1];
		Atom a2 = (Atom)client_msg_ev->data.l[2];

		Atom atoms[2] = { a1, a2 };
		for (int i = 0; i < 2; i++) {
			if (atoms[i] == None) {
				continue;
			}

			if (atoms[i] == _NET_WM_STATE_FULLSCREEN) {
				Bool want = c->fullscreen;
				if (action == 0) {
					want = False;
				}
				else if (action == 1) {
					want = True;
				}
				else if (action == 2) {
					want = !want;
				}

				apply_fullscreen(c, want);

				if (want) {
					XRaiseWindow(dpy, c->win);
				}
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
	XConfigureRequestEvent *config_ev = &xev->xconfigurerequest;
	Client *c = NULL;

	for (int i = 0; i < NUM_WORKSPACES && !c; i++) {
		for (c = workspaces[i]; c; c = c->next) {
			if (c->win == config_ev->window) {
				break;
			}
		}
	}

	if (!c || c->floating || c->fullscreen) {
		/* allow client to configure itself */
		XWindowChanges wc = {
			.x = config_ev->x,
			.y = config_ev->y,
			.width = config_ev->width,
			.height = config_ev->height,
			.border_width = config_ev->border_width,
			.sibling = config_ev->above,
			.stack_mode = config_ev->detail
		};
		XConfigureWindow(dpy, config_ev->window, config_ev->value_mask, &wc);
		return;
	}
}

void hdl_dummy(XEvent *xev)
{
	(void)xev;
}

void hdl_destroy_ntf(XEvent *xev)
{
	Window w = xev->xdestroywindow.window;

	for (int i = 0; i < NUM_WORKSPACES; i++) {
		Client *prev = NULL, *c = workspaces[i];
		while (c && c->win != w) {
			prev = c;
			c = c->next;
		}
		if (c) {
			if (c->swallower) {
				unswallow_window(c);
			}

			if (c->swallowed) {
				Client *swallowed = c->swallowed;
				c->swallowed = NULL;
				swallowed->swallower = NULL;

				/* show swallowed window */
				XMapWindow(dpy, swallowed->win);
				swallowed->mapped = True;
				focused = swallowed;
			}

			if (focused == c) {
				/* first try next on same monitor */
				Client *tmp = c->next;
				while (tmp && tmp->mon != current_mon) {
					tmp = tmp->next;
				}

				if (tmp) {
					focused = tmp;
				}
				else {
					/* then try previous on same monitor */
					tmp = prev;
					while (tmp && tmp->mon != current_mon) {
						/* walk backwards */
						Client *p = workspaces[i];
						Client *p_prev = NULL;
						while (p && p != tmp) {
							p_prev = p;
							p = p->next;
						}
						tmp = p_prev;
					}
					focused = tmp;
				}

				/* if nothing found on same monitor, unfocus */
				if (!focused || focused->mon != current_mon) {
					focused = NULL;
				}
			}

			if (!prev) {
				workspaces[i] = c->next;
			}
			else {
				prev->next = c->next;
			}

			free(c);
			update_net_client_list();
			open_windows--;

			if (i == current_ws) {
				tile();
				update_borders();

				if (focused) {
					set_input_focus(focused, True, True);
				}
			}
			return;
		}
	}
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
				case TYPE_SP_REMOVE: remove_scratchpad(bind->action.sp); break;
				case TYPE_SP_TOGGLE: toggle_scratchpad(bind->action.sp); break;
				case TYPE_SP_CREATE: set_win_scratchpad(bind->action.sp); break;
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
	XWindowAttributes win_attr;

	if (!XGetWindowAttributes(dpy, w, &win_attr)) {
		return;
	}

	/* skips invisible windows */
	if (win_attr.override_redirect || win_attr.width <= 0 || win_attr.height <= 0) {
		XMapWindow(dpy, w);
		return;
	}

	/* check if this window is already managed on any workspace */
	Client *c = find_client(w);
	if (c) {
		if (c->ws == current_ws) {
			if (!c->mapped) {
				XMapWindow(dpy, w);
				c->mapped = True;
			}
			if (user_config.new_win_focus) {
				focused = c;
				set_input_focus(c, True, True);
				return; /* set_input_focus already calls update_borders */
			}
			update_borders();
		}
		return;
	}


	Atom type;
	int format;
	unsigned long n_items, after;
	Atom *types = NULL;
	Bool should_float = False;

	if (XGetWindowProperty(dpy, w, _NET_WM_WINDOW_TYPE, 0, 8, False, XA_ATOM, &type, &format,
				           &n_items, &after, (unsigned char **)&types) == Success && types) {

		for (unsigned long i = 0; i < n_items; i++) {
			if (types[i] == _NET_WM_WINDOW_TYPE_DOCK) {
				XFree(types);
				XMapWindow(dpy, w);
				return;
			}

			if (types[i] == _NET_WM_WINDOW_TYPE_UTILITY ||
				types[i] == _NET_WM_WINDOW_TYPE_DIALOG  ||
				types[i] == _NET_WM_WINDOW_TYPE_TOOLBAR ||
				types[i] == _NET_WM_WINDOW_TYPE_SPLASH  ||
				types[i] == _NET_WM_WINDOW_TYPE_POPUP_MENU ||
				types[i] == _NET_WM_WINDOW_TYPE_DROPDOWN_MENU ||
				types[i] == _NET_WM_WINDOW_TYPE_MENU ||
				types[i] == _NET_WM_WINDOW_TYPE_TOOLTIP ||
				types[i] == _NET_WM_WINDOW_TYPE_NOTIFICATION) {
				should_float = True;
				break;
			}
		}
		XFree(types);
	}

	if (!should_float) {
		should_float = window_should_float(w);
	}

	if (!should_float) {
		Atom state_type;
		Atom *state_atoms = NULL;
		int state_format;
		unsigned long bytes_after;
		n_items = 0;

		if (XGetWindowProperty(dpy, w, _NET_WM_STATE, 0, 8, False, XA_ATOM, &state_type, &state_format, &n_items,
					           &bytes_after, (unsigned char**)&state_atoms) == Success && state_atoms) {
			for (unsigned long i = 0; i < n_items; i++) {
				if (state_atoms[i] == _NET_WM_STATE_MODAL) {
					should_float = True;
					break;
				}
			}
			XFree(state_atoms);
		}
	}

	if (open_windows == MAX_CLIENTS) {
		fprintf(stderr, "sxwm: max clients reached, ignoring map request\n");
		return;
	}

	int target_ws = get_workspace_for_window(w);
	c = add_client(w, target_ws);
	if (!c) {
		return;
	}
	set_wm_state(w, NormalState);

	Window transient;
	if (!should_float && XGetTransientForHint(dpy, w, &transient)) {
		should_float = True;
	}

	XSizeHints size_hints;
	long supplied_ret;

	if (!should_float &&
		XGetWMNormalHints(dpy, w, &size_hints, &supplied_ret) &&
		(size_hints.flags & PMinSize) && (size_hints.flags & PMaxSize) &&
		size_hints.min_width  == size_hints.max_width &&
		size_hints.min_height == size_hints.max_height) {

		should_float = True;
		c->fixed = True;
	}

	if (should_float || global_floating) {
		c->floating = True;
	}

	if (window_should_start_fullscreen(w)) {
		c->fullscreen = True;
		c->floating = False;
	}

	/* center floating windows & set border */
	if (c->floating && !c->fullscreen) {
		int w_ = MAX(c->w, 64), h_ = MAX(c->h, 64);
		int mx = mons[c->mon].x, my = mons[c->mon].y;
		int mw = mons[c->mon].w, mh = mons[c->mon].h;
		int x = mx + (mw - w_) / 2, y = my + (mh - h_) / 2;
		c->x = x;
		c->y = y;
		c->w = w_;
		c->h = h_;
		XMoveResizeWindow(dpy, w, x, y, w_, h_);
		XSetWindowBorderWidth(dpy, w, user_config.border_width);
	}

	update_net_client_list();
	if (target_ws != current_ws) {
		return;
	}

	/* map & borders */
	if (!global_floating && !c->floating) {
		tile();
	}
	else if (c->floating) {
		XRaiseWindow(dpy, w);
	}

	/* check for swallowing opportunities */
	{
		XClassHint ch = {0};
		Bool can_be_swallowed = False;

		if (XGetClassHint(dpy, w, &ch)) {
			/* check if new window can be swallowed */
			for (int i = 0; i < MAX_ITEMS; i++) {
				if (!user_config.can_be_swallowed[i] || !user_config.can_be_swallowed[i][0]) {
					break;
				}

				if ((ch.res_class && strcasecmp(ch.res_class, user_config.can_be_swallowed[i][0]) == 0) ||
				    (ch.res_name && strcasecmp(ch.res_name, user_config.can_be_swallowed[i][0]) == 0)) {
					can_be_swallowed = True;
					break;
				}
			}

			/* if window can be swallowed look for a potential swallower */
			if (can_be_swallowed) {
				for (Client *p = workspaces[current_ws]; p; p = p->next) {
					if (p == c || p->swallowed || !p->mapped) {
						continue;
					}

					XClassHint pch = {0};
					Bool can_swallow = False;

					if (XGetClassHint(dpy, p->win, &pch)) {
						/* check if this existing window can swallow others */
						for (int i = 0; i < MAX_ITEMS; i++) {
							if (!user_config.can_swallow[i] || !user_config.can_swallow[i][0]) {
								break;
							}

							if ((pch.res_class && strcasecmp(pch.res_class, user_config.can_swallow[i][0]) == 0) ||
							    (pch.res_name && strcasecmp(pch.res_name, user_config.can_swallow[i][0]) == 0)) {
								can_swallow = True;
								break;
							}
						}

						/* check process relationship */
						if (can_swallow && check_parent(p->pid, c->pid)) {
							/* we know class matches and the swallower is the parent -> swallow now */
							swallow_window(p, c);
							XFree(pch.res_class);
							XFree(pch.res_name);
							break;
						}
						XFree(pch.res_class);
						XFree(pch.res_name);
					}
				}
			}
			XFree(ch.res_class);
			XFree(ch.res_name);
		}
	}

	if (window_has_ewmh_state(w, _NET_WM_STATE_FULLSCREEN)) {
		c->fullscreen = True;
		c->floating = False;
	}

	XMapWindow(dpy, w);
	c->mapped = True;
	if (c->fullscreen) {
		apply_fullscreen(c, True);
	}
	set_frame_extents(w);

	if (user_config.new_win_focus) {
		focused = c;
		set_input_focus(focused, False, True);
		return;
	}
	update_borders();
}

void hdl_motion(XEvent *xev)
{
	XMotionEvent *motion_ev = &xev->xmotion;

	if ((drag_mode == DRAG_NONE || !drag_client) ||
			(motion_ev->time - last_motion_time <= (1000 / (Time)user_config.motion_throttle))) {
		return;
	}
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

	if (drag_mode == DRAG_SWAP) {
		Window root_ret, child;
		int rx, ry, wx, wy;
		unsigned int mask;
		XQueryPointer(dpy, root, &root_ret, &child, &rx, &ry, &wx, &wy, &mask);

		Client *last_swap_target = NULL;
		Client *new_target = NULL;

		for (Client *c = workspaces[current_ws]; c; c = c->next) {
			if (c == drag_client || c->floating) {
				continue;
			}
			if (c->win == child) {
				new_target = c;
				break;
			}
		}

		if (new_target != last_swap_target) {
			if (last_swap_target) {
				XSetWindowBorder(
						dpy, last_swap_target->win, (last_swap_target == focused ?
						user_config.border_foc_col : user_config.border_ufoc_col)
				);
			}
			if (new_target) {
				XSetWindowBorder(dpy, new_target->win, user_config.border_swap_col);
			}
			last_swap_target = new_target;
		}

		swap_target = new_target;
		return;
	}
	else if (drag_mode == DRAG_MOVE) {
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
		if (property_ev->atom == _NET_CURRENT_DESKTOP) {
			long *val = NULL;
			Atom actual;
			int fmt;
			unsigned long n;
			unsigned long after;
			if (XGetWindowProperty(dpy, root, _NET_CURRENT_DESKTOP, 0, 1, False, XA_CARDINAL, &actual,
						           &fmt, &n, &after, (unsigned char **)&val) == Success && val) {
				change_workspace((int)val[0]);
				XFree(val);
			}
		}
		else if (property_ev->atom == _NET_WM_STRUT_PARTIAL) {
			update_struts();
			tile();
			update_borders();
		}
	}

	/* client window properties */
	if (property_ev->atom == _NET_WM_STATE) {
		Client *c = find_client(find_toplevel(property_ev->window));
		if (!c) {
			return;
		}

		Bool want = window_has_ewmh_state(c->win, _NET_WM_STATE_FULLSCREEN);
		if (want != c->fullscreen) {
			apply_fullscreen(c, want);
		}
	}
}

void hdl_unmap_ntf(XEvent *xev)
{
	if (!in_ws_switch) {
		Window w = xev->xunmap.window;
		for (Client *c = workspaces[current_ws]; c; c = c->next) {
			if (c->win == w) {
				c->mapped = False;
				break;
			}
		}
	}

	update_net_client_list();
	tile();
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
	user_config.border_swap_col = parse_col("#fff4c0");
	user_config.move_window_amt = 10;
	user_config.resize_window_amt = 10;

	for (int i = 0; i < MAX_MONITORS; i++) {
		user_config.master_width[i] = 50 / 100.0f;
	}

	for (int i = 0; i < MAX_ITEMS; i++) {
		user_config.can_be_swallowed[i] = NULL;
		user_config.can_swallow[i] = NULL;
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

Bool is_child_proc(pid_t parent_pid, pid_t child_pid)
{
	if (parent_pid <= 0 || child_pid <= 0) {
		return False;
	}

	char path[PATH_MAX];
	FILE *f;
	pid_t current_pid = child_pid;
	int max_iterations = 20;

	while (current_pid > 1 && max_iterations-- > 0) {
		snprintf(path, sizeof(path), "/proc/%d/stat", current_pid);
		f = fopen(path, "r");
		if (!f) {
			fprintf(stderr, "sxwm: could not open %s\n", path);
			return False;
		}

		int ppid = 0;
		if (fscanf(f, "%*d %*s %*c %d", &ppid) != 1) {
			fprintf(stderr, "sxwm: failed to read ppid from %s\n", path);
			fclose(f);
			return False;
		}
		fclose(f);

		if (ppid == parent_pid) {
			return True;
		}

		if (ppid <= 1) {
			/* Reached init or kernel */
			fprintf(stderr, "sxwm: reached init/kernel, no relationship found\n");
			break;
		}
		current_pid = ppid;
	}
	return False;
}

void move_master_next(void)
{
	if (!workspaces[current_ws] || !workspaces[current_ws]->next) {
		return;
	}

	Client *first = workspaces[current_ws];
	Client *old_focused = focused;

	workspaces[current_ws] = first->next;
	first->next = NULL;

	Client *tail = workspaces[current_ws];
	while (tail->next) {
		tail = tail->next;
	}
	tail->next = first;

	tile();
	if (user_config.warp_cursor && old_focused) {
		warp_cursor(old_focused);
	}
	if (old_focused) {
		send_wm_take_focus(old_focused->win);
	}
	update_borders();
}

void move_master_prev(void)
{
	if (!workspaces[current_ws] || !workspaces[current_ws]->next) {
		return;
	}

	Client *prev = NULL;
	Client *cur = workspaces[current_ws];
	Client *old_focused = focused;

	while (cur->next) {
		prev = cur;
		cur = cur->next;
	}

	if (prev) {
		prev->next = NULL;
	}

	cur->next = workspaces[current_ws];
	workspaces[current_ws] = cur;

	tile();
	if (user_config.warp_cursor && old_focused) {
		warp_cursor(old_focused);
	}
	if (old_focused) {
		send_wm_take_focus(old_focused->win);
	}
	update_borders();
}

void move_next_mon(void)
{
	if (!focused || n_mons <= 1) {
		return; /* no focused window or only one monitor */
	}

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
	if (user_config.warp_cursor) {
		warp_cursor(focused);
	}

	update_borders();
}

void move_prev_mon(void)
{
	if (!focused || n_mons <= 1) {
		return; /* no focused window or only one monitor */
	}

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
	if (user_config.warp_cursor) {
		warp_cursor(focused);
	}

	update_borders();
}

void move_to_workspace(int ws)
{
	if (!focused || ws >= NUM_WORKSPACES || ws == current_ws) {
		return;
	}
	XUnmapWindow(dpy, focused->win);

	/* remove from current list */
	Client **pp = &workspaces[current_ws];
	while (*pp && *pp != focused) {
		pp = &(*pp)->next;
	}
	if (*pp) {
		*pp = focused->next;
	}

	/* push to target list */
	focused->next = workspaces[ws];
	workspaces[ws] = focused;
	focused->ws = ws;
	long desktop = ws;
	XChangeProperty(dpy, focused->win, _NET_WM_DESKTOP, XA_CARDINAL, 32,
			        PropModeReplace, (unsigned char *)&desktop, 1);

	/* tile current ws */
	tile();
	focused = workspaces[current_ws];
	if (focused) {
		set_input_focus(focused, False, False);
	}
}

void move_win_down(void)
{
	if (!focused || !focused->floating) {
		return;
	}
	focused->y += user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void move_win_left(void)
{
	if (!focused || !focused->floating) {
		return;
	}
	focused->x -= user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void move_win_right(void)
{
	if (!focused || !focused->floating) {
		return;
	}
	focused->x += user_config.move_window_amt;
	XMoveWindow(dpy, focused->win, focused->x, focused->y);
}

void move_win_up(void)
{
	if (!focused || !focused->floating) {
		return;
	}
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

	/* possibly unsafe BUT i dont think it can cause any problems */
	return col.pixel |= 0xff << 24;
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
	XCloseDisplay(dpy);
	XFreeCursor(dpy, cursor_move);
	XFreeCursor(dpy, cursor_normal);
	XFreeCursor(dpy, cursor_resize);
	puts("quitting...");
	running = False;
}

void reload_config(void)
{
	puts("sxwm: reloading config...");

	/* free binding commands without */
	for (int i = 0; i < user_config.n_binds; i++) {
		if (user_config.binds[i].type == TYPE_CMD && user_config.binds[i].action.cmd) {
			free(user_config.binds[i].action.cmd);
		}
		user_config.binds[i].action.cmd = NULL;
		user_config.binds[i].action.fn = NULL;
		user_config.binds[i].type = -1;
		user_config.binds[i].keysym = 0;
		user_config.binds[i].mods = 0;
	}

	/* free swallow-related arrays */
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (user_config.can_swallow[i]) {
			if (user_config.can_swallow[i][0]) {
				free(user_config.can_swallow[i][0]);
			}
			free(user_config.can_swallow[i]);
			user_config.can_swallow[i] = NULL;
		}
		if (user_config.can_be_swallowed[i]) {
			if (user_config.can_be_swallowed[i][0]) {
				free(user_config.can_be_swallowed[i][0]);
			}
			free(user_config.can_be_swallowed[i]);
			user_config.can_be_swallowed[i] = NULL;
		}
		if (user_config.open_in_workspace[i]) {
			if (user_config.open_in_workspace[i][0]) {
				free(user_config.open_in_workspace[i][0]);
			}
			if (user_config.open_in_workspace[i][1]) {
				free(user_config.open_in_workspace[i][1]);
			}
			free(user_config.open_in_workspace[i]);
			user_config.open_in_workspace[i] = NULL;
		}
		if (user_config.start_fullscreen[i]) {
			if (user_config.start_fullscreen[i][0]) {
				free(user_config.start_fullscreen[i][0]);
			}
			free(user_config.start_fullscreen[i]);
			user_config.start_fullscreen[i] = NULL;
		}
	}

	/* free should_float arrays */
	for (int i = 0; i < MAX_ITEMS; i++) {
		if (user_config.should_float[i]) {
			if (user_config.should_float[i][0]) {
				free(user_config.should_float[i][0]);
			}
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
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		}
	}

	Mask root_click_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	Mask root_swap_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	Mask root_resize_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	grab_button(Button1, user_config.modkey, root, True, root_click_masks);
	grab_button(Button1, user_config.modkey | ShiftMask, root, True, root_swap_masks);
	grab_button(Button3, user_config.modkey, root, True, root_resize_masks);

	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			grab_button(Button1, None, c->win, False, ButtonPressMask);
			grab_button(Button1, user_config.modkey, c->win, False, ButtonPressMask);
			grab_button(Button1, user_config.modkey | ShiftMask, c->win, False, ButtonPressMask);
			grab_button(Button2, user_config.modkey, c->win, False, ButtonPressMask);
		}
	}

	update_client_desktop_properties();
	update_net_client_list();
	XSync(dpy, False);

	tile();
	update_borders();
}

void remove_scratchpad(int n)
{
	if (n < 0 || n >= MAX_SCRATCHPADS || scratchpads[n].client == NULL) {
		return;
	}

	Client *c = scratchpads[n].client;

	if (c->win) {
		XMapWindow(dpy, c->win);
		c->mapped = True;
	}

	scratchpads[n].client = NULL;
	scratchpads[n].enabled = False;

	update_net_client_list();
	update_borders();
}

void resize_master_add(void)
{
	/* pick the monitor of the focused window (or 0 if none) */
	int m = focused ? focused->mon : 0;
	float *mw = &user_config.master_width[m];

	if (*mw < MF_MAX - 0.001f) {
		*mw += ((float)user_config.resize_master_amt / 100);
	}
	tile();
	update_borders();
}

void resize_master_sub(void)
{
	/* pick the monitor of the focused window (or 0 if none) */
	int m = focused ? focused->mon : 0;
	float *mw = &user_config.master_width[m];

	if (*mw > MF_MIN + 0.001f) {
		*mw -= ((float)user_config.resize_master_amt / 100);
	}
	tile();
	update_borders();
}

void resize_stack_add(void)
{
	if (!focused || focused->floating || focused == workspaces[current_ws]) {
		return;
	}

	int bw2 = 2 * user_config.border_width;
	int raw_cur = (focused->custom_stack_height > 0) ? focused->custom_stack_height : (focused->h + bw2);

	int raw_new = raw_cur + user_config.resize_stack_amt;
	focused->custom_stack_height = raw_new;
	tile();
}

void resize_stack_sub(void)
{
	if (!focused || focused->floating || focused == workspaces[current_ws]) {
		return;
	}

	int bw2 = 2 * user_config.border_width;
	int raw_cur = (focused->custom_stack_height > 0) ? focused->custom_stack_height : (focused->h + bw2);

	int raw_new = raw_cur - user_config.resize_stack_amt;
	int min_raw = bw2 + 1;

	if (raw_new < min_raw) {
		raw_new = min_raw;
	}
	focused->custom_stack_height = raw_new;
	tile();
}

void resize_win_down(void)
{
	if (!focused || !focused->floating) {
		return;
	}

	int new_h = focused->h + user_config.resize_window_amt;
	int max_h = mons[focused->mon].h - (focused->y - mons[focused->mon].y);
	focused->h = CLAMP(new_h, MIN_WINDOW_SIZE, max_h);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void resize_win_up(void)
{
	if (!focused || !focused->floating) {
		return;
	}

	int new_h = focused->h - user_config.resize_window_amt;
	focused->h = CLAMP(new_h, MIN_WINDOW_SIZE, focused->h);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void resize_win_right(void)
{
	if (!focused || !focused->floating) {
		return;
	}

	int new_w = focused->w + user_config.resize_window_amt;
	int max_w = mons[focused->mon].w - (focused->x - mons[focused->mon].x);
	focused->w = CLAMP(new_w, MIN_WINDOW_SIZE, max_w);
	XResizeWindow(dpy, focused->win, focused->w, focused->h);
}

void resize_win_left(void)
{
	if (!focused || !focused->floating) {
		return;
	}

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
				|| wa.override_redirect || wa.map_state != IsViewable) {
				continue;
			}

			XEvent fake_event = {None};
			fake_event.type = MapRequest;
			fake_event.xmaprequest.window = children[i];
			hdl_map_req(&fake_event);
		}
		if (children) {
			XFree(children);
		}
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
	if ((dpy = XOpenDisplay(NULL)) == False) {
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

	cursor_normal = XcursorLibraryLoadCursor(dpy, "left_ptr");
	cursor_move = XcursorLibraryLoadCursor(dpy, "fleur");
	cursor_resize = XcursorLibraryLoadCursor(dpy, "bottom_right_corner");
	XDefineCursor(dpy, root, cursor_normal);

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));

	update_mons();

	/* select events wm should look for on root */
	Mask wm_masks = StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask |
	                KeyPressMask | PropertyChangeMask;
	select_input(root, wm_masks);

	/* grab mouse button events on root window */
	Mask root_click_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	Mask root_swap_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	Mask root_resize_masks = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	grab_button(Button1, user_config.modkey, root, True, root_click_masks);
	grab_button(Button1, user_config.modkey | ShiftMask, root, True, root_swap_masks);
	grab_button(Button3, user_config.modkey, root, True, root_resize_masks);
	XSync(dpy, False);

	for (int i = 0; i < LASTEvent; i++) {
		evtable[i] = hdl_dummy;
	}
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
	_NET_NUMBER_OF_DESKTOPS = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	_NET_DESKTOP_NAMES = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	_NET_CURRENT_DESKTOP = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	_NET_ACTIVE_WINDOW = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	_NET_SUPPORTED = XInternAtom(dpy, "_NET_SUPPORTED", False);
	_NET_WM_STRUT_PARTIAL = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
	_NET_WM_STRUT = XInternAtom(dpy, "_NET_WM_STRUT", False); /* legacy struts */
	_NET_WM_WINDOW_TYPE = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	_NET_WORKAREA = XInternAtom(dpy, "_NET_WORKAREA", False);
	_NET_WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", False);
	_NET_WM_STATE_FULLSCREEN = XInternAtom(dpy,"_NET_WM_STATE_FULLSCREEN", False);
	WM_STATE = XInternAtom(dpy, "WM_STATE", False);
	WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	_NET_SUPPORTING_WM_CHECK = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	_NET_WM_NAME = XInternAtom(dpy, "_NET_WM_NAME", False);
	UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
	_NET_WM_DESKTOP = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	_NET_CLIENT_LIST = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	_NET_FRAME_EXTENTS = XInternAtom(dpy, "_NET_FRAME_EXTENTS", False);
	_NET_WM_PID = XInternAtom(dpy, "_NET_WM_PID", False);

	_NET_WM_WINDOW_TYPE_DOCK = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	_NET_WM_WINDOW_TYPE_UTILITY = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	_NET_WM_WINDOW_TYPE_DIALOG = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	_NET_WM_WINDOW_TYPE_TOOLBAR = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
	_NET_WM_WINDOW_TYPE_SPLASH = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", False);
	_NET_WM_WINDOW_TYPE_POPUP_MENU = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
	_NET_WM_WINDOW_TYPE_MENU = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_MENU", False);
	_NET_WM_WINDOW_TYPE_DROPDOWN_MENU = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", False);
	_NET_WM_WINDOW_TYPE_TOOLTIP = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLTIP", False);
	_NET_WM_WINDOW_TYPE_NOTIFICATION = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
	_NET_WM_STATE_MODAL = XInternAtom(dpy, "_NET_WM_STATE_MODAL", False);

	Atom support_list[] = {
	    _NET_CURRENT_DESKTOP, _NET_ACTIVE_WINDOW, _NET_SUPPORTED, _NET_WM_STATE,
		_NET_WM_STATE_FULLSCREEN, _NET_WM_WINDOW_TYPE, _NET_WORKAREA, _NET_WM_STRUT,
		_NET_WM_STRUT_PARTIAL, WM_DELETE_WINDOW, _NET_SUPPORTING_WM_CHECK, _NET_WM_NAME,
		UTF8_STRING, _NET_WM_DESKTOP, _NET_CLIENT_LIST, _NET_FRAME_EXTENTS, _NET_WM_PID,

		_NET_WM_WINDOW_TYPE_DOCK, _NET_WM_WINDOW_TYPE_UTILITY, _NET_WM_WINDOW_TYPE_DIALOG,
		_NET_WM_WINDOW_TYPE_TOOLBAR, _NET_WM_WINDOW_TYPE_SPLASH, _NET_WM_WINDOW_TYPE_POPUP_MENU,
		_NET_WM_WINDOW_TYPE_MENU, _NET_WM_WINDOW_TYPE_DROPDOWN_MENU, _NET_WM_WINDOW_TYPE_TOOLTIP,
		_NET_WM_WINDOW_TYPE_NOTIFICATION, _NET_WM_STATE_MODAL,
	};

	/* checking window */
	wm_check_win = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	/* root property -> child window */
	XChangeProperty(dpy, root, _NET_SUPPORTING_WM_CHECK, XA_WINDOW, 32,
			        PropModeReplace, (unsigned char *)&wm_check_win, 1);
	/* child window -> child window */
	XChangeProperty(dpy, wm_check_win, _NET_SUPPORTING_WM_CHECK, XA_WINDOW, 32,
			        PropModeReplace, (unsigned char *)&wm_check_win, 1);
	/* name the wm */
	const char *wmname = "sxwm";
	XChangeProperty(dpy, wm_check_win, _NET_WM_NAME, UTF8_STRING, 8,
			        PropModeReplace, (const unsigned char *)wmname, strlen(wmname));

	/* workspace setup */
	long num_workspaces = NUM_WORKSPACES;
	XChangeProperty(dpy, root, _NET_NUMBER_OF_DESKTOPS, XA_CARDINAL, 32,
			        PropModeReplace, (const unsigned char *)&num_workspaces, 1);

	const char workspace_names[] = WORKSPACE_NAMES;
	int names_len = sizeof(workspace_names);
	XChangeProperty(dpy, root, _NET_DESKTOP_NAMES, UTF8_STRING, 8,
			        PropModeReplace, (const unsigned char *)workspace_names, names_len);

	XChangeProperty(dpy, root, _NET_CURRENT_DESKTOP, XA_CARDINAL, 32,
			        PropModeReplace, (const unsigned char *)&current_ws, 1);

	/* load supported list */
	int support_list_len = sizeof(support_list) / sizeof(Atom);
	XChangeProperty(dpy, root, _NET_SUPPORTED, XA_ATOM, 32,
			        PropModeReplace, (const unsigned char *)support_list, support_list_len);

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
	XChangeProperty(dpy, w, _NET_FRAME_EXTENTS, XA_CARDINAL, 32,
			        PropModeReplace, (unsigned char *)extents, 4);
}

void set_input_focus(Client *c, Bool raise_win, Bool warp)
{
	if (c && c->mapped) {
		focused = c;
		current_mon = c->mon;
		Window w = find_toplevel(c->win);

		XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
		send_wm_take_focus(w);

		if (raise_win) {
			/* if floating_on_top, don't raise a tiled window above floats. */
			if (c->floating || !user_config.floating_on_top) {
				XRaiseWindow(dpy, w);
			}
		}
		/* EWMH focus hint */
		XChangeProperty(dpy, root, _NET_ACTIVE_WINDOW, XA_WINDOW, 32,
				PropModeReplace, (unsigned char *)&w, 1);

		update_borders();

		if (warp && user_config.warp_cursor) {
			warp_cursor(c);
		}
	} else {
		/* no client */
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, _NET_ACTIVE_WINDOW);

		focused = NULL;
		update_borders();
	}

	XFlush(dpy);
}

void set_win_scratchpad(int n)
{
	if (focused == NULL) {
		return;
	}

	Client *pad_client = focused;
	if (scratchpads[n].client != NULL) {
		XMapWindow(dpy, scratchpads[n].client->win);
		scratchpads[n].enabled = False;
		scratchpads[n].client = NULL;
	}
	scratchpads[n].client = pad_client;
	XUnmapWindow(dpy, scratchpads[n].client->win);
	scratchpads[n].enabled = False;
}

void reset_opacity(Window w)
{
	Atom atom = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	XDeleteProperty(dpy, w, atom);
}


void set_opacity(Window w, double opacity)
{
	if (opacity < 0.0) {
		opacity = 0.0;
	}

	if (opacity > 1.0) {
		opacity = 1.0;
	}

	unsigned long op = (unsigned long)(opacity * 0xFFFFFFFFu);
	Atom atom = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
	XChangeProperty(dpy, w, atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&op, 1);
}

void set_wm_state(Window w, long state)
{
	long data[2] = { state, None }; /* state, icon window */
	XChangeProperty(dpy, w, WM_STATE, WM_STATE, 32,
			        PropModeReplace, (unsigned char *)data, 2);
}

int snap_coordinate(int pos, int size, int screen_size, int snap_dist)
{
	if (UDIST(pos, 0) <= snap_dist) {
		return 0;
	}
	if (UDIST(pos + size, screen_size) <= snap_dist) {
		return screen_size - size;
	}
	return pos;
}

void spawn(const char * const *argv)
{
	int argc = 0;
	while (argv[argc]) {
		argc++;
	}

	int cmd_count = 1;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "|") == 0) {
			cmd_count++;
		}
	}

	const char ***commands = malloc(cmd_count * sizeof(char **)); /* *** bruh */
	if (!commands) {
		perror("malloc commands");
		return;
	}

	int cmd_idx = 0;
	int arg_start = 0;
	for (int i = 0; i <= argc; i++) {
		if (!argv[i] || strcmp(argv[i], "|") == 0) {
			int len = i - arg_start;
			const char **cmd_args = malloc((len + 1) * sizeof(char *));
			if (!cmd_args) {
				perror("malloc cmd_args");
				for (int j = 0; j < cmd_idx; j++) {
					free(commands[j]);
				}
				free(commands);
				return;
			}
			for (int j = 0; j < len; j++) {
				cmd_args[j] = argv[arg_start + j];
			}
			cmd_args[len] = NULL;
			commands[cmd_idx++] = cmd_args;
			arg_start = i + 1;
		}
	}

	int (*pipes)[2] = malloc(sizeof(int[2]) * (cmd_count - 1));
	if (!pipes) {
		perror("malloc pipes");
		return;
	}
	for (int i = 0; i < cmd_count - 1; i++) {
		if (pipe(pipes[i]) == -1) {
			perror("pipe");
			for (int j = 0; j < cmd_count; j++) {
				free(commands[j]);
			}
			free(commands);
			free(pipes);
			return;
		}
	}

	for (int i = 0; i < cmd_count; i++) {
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork");
			for (int k = 0; k < cmd_count - 1; k++) {
				close(pipes[k][0]);
				close(pipes[k][1]);
			}
			for (int j = 0; j < cmd_count; j++) {
				free(commands[j]);
			}
			free(commands);
			free(pipes);
			return;
		}
		if (pid == 0) {
			close(ConnectionNumber(dpy));

			if (i > 0) {
				dup2(pipes[i - 1][0], STDIN_FILENO);
			}
			if (i < cmd_count - 1) {
				dup2(pipes[i][1], STDOUT_FILENO);
			}

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

	for (int i = 0; i < cmd_count; i++) {
		free(commands[i]);
	}
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
				for (int j = 0; argv[j]; j++) {
					free((void*)(uintptr_t)argv[j]);
				}
				free(argv);
			}
		}
	}
}

void swallow_window(Client *swallower, Client *swallowed)
{
	if (!swallower || !swallowed || swallower->swallowed || swallowed->swallower) {
		return;
	}

	XUnmapWindow(dpy, swallower->win);
	swallower->mapped = False;

	swallower->swallowed = swallowed;
	swallowed->swallower = swallower;

	swallowed->floating = swallower->floating;
	if (swallowed->floating) {
		swallowed->x = swallower->x;
		swallowed->y = swallower->y;
		swallowed->w = swallower->w;
		swallowed->h = swallower->h;

		if (swallowed->win) {
			XMoveResizeWindow(dpy, swallowed->win, swallowed->x, swallowed->y, swallowed->w, swallowed->h);
		}
	}

	tile();
	update_borders();
}

void swap_clients(Client *a, Client *b)
{
	if (!a || !b || a == b) {
		return;
	}

	Client **head = &workspaces[current_ws];
	Client **pa = head, **pb = head;

	while (*pa && *pa != a) {
		pa = &(*pa)->next;
	}
	while (*pb && *pb != b) {
		pb = &(*pb)->next;
	}

	if (!*pa || !*pb) {
		return;
	}

	/* if next to it swap */
	if (*pa == b && *pb == a) {
		Client *tmp = b->next;
		b->next = a;
		a->next = tmp;
		*pa = b;
		return;
	}

	/* full swap */
	Client *ta = *pa;
	Client *tb = *pb;
	Client *ta_next = ta->next;
	Client *tb_next = tb->next;

	*pa = tb;
	tb->next = ta_next == tb ? ta : ta_next;

	*pb = ta;
	ta->next = tb_next == ta ? tb : tb_next;
}

void switch_previous_workspace(void)
{
	change_workspace(previous_workspace);
}

void tile(void)
{
	update_struts();
	Client *head = workspaces[current_ws];
	int total = 0;
	Bool fullscreen_present = False;

	for (Client *c = head; c; c = c->next) {
		if (c->mapped && !c->floating && !c->fullscreen) {
			total++;
		}
		if (!c->floating && c->fullscreen) {
			fullscreen_present = True;
		}
	}

	if (total == 1 && fullscreen_present) {
		return;
	}

	if (monocle) {
		for (Client *c = head; c; c = c->next) {
			if (!c->mapped || c->fullscreen) {
				continue;
			}

			int border_width = user_config.border_width;
			int gaps = user_config.gaps;

			int mon = c->mon;
			int x = mons[mon].x + mons[mon].reserve_left + gaps;
			int y = mons[mon].y + mons[mon].reserve_top + gaps;
			int w = mons[mon].w - mons[mon].reserve_left - mons[mon].reserve_right - 2 * gaps;
			int h = mons[mon].h - mons[mon].reserve_top - mons[mon].reserve_bottom - 2 * gaps;

			XWindowChanges wc = {
				.x = x,
				.y = y,
				.width = MAX(1, w - 2 * border_width),
				.height = MAX(1, h - 2 * border_width),
				.border_width = border_width
			};
			XConfigureWindow(dpy, c->win,
					CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
			XRaiseWindow(dpy, c->win);

			c->x = wc.x;
			c->y = wc.y;
			c->w = wc.width;
			c->h = wc.height;
		}

		update_borders();
		return;
	}

	for (int m = 0; m < n_mons; m++) {
		int mon_x = mons[m].x + mons[m].reserve_left;
		int mon_y = mons[m].y + mons[m].reserve_top;
		int mon_width = MAX(1, mons[m].w - mons[m].reserve_left - mons[m].reserve_right);
		int mon_height = MAX(1, mons[m].h - mons[m].reserve_top  - mons[m].reserve_bottom);

		Client *tileable[MAX_CLIENTS] = {0};
		int n_tileable = 0;
		for (Client *c = head; c && n_tileable < MAX_CLIENTS; c = c->next) {
			if (c->mapped && !c->floating && !c->fullscreen && c->mon == m) {
				tileable[n_tileable++] = c;
			}
		}

		if (n_tileable == 0) {
			continue;
		}

		int gaps = user_config.gaps;
		int tile_x = mon_x + gaps;
		int tile_y = mon_y + gaps;
		int tile_width = MAX(1, mon_width - 2 * gaps);
		int tile_height = MAX(1, mon_height - 2 * gaps);
		float master_frac = CLAMP(user_config.master_width[m], MF_MIN, MF_MAX);
		int master_width = (n_tileable > 1) ? (int)(tile_width * master_frac) : tile_width;
		int stack_width = (n_tileable > 1) ? (tile_width - master_width - gaps) : 0;

		{
			Client *c = tileable[0];
			int border_width = 2 * user_config.border_width;
			XWindowChanges wc = {
				.x = tile_x,
				.y = tile_y,
				.width = MAX(1, master_width - border_width),
				.height = MAX(1, tile_height - border_width),
				.border_width = user_config.border_width
			};

			Bool geom_differ =
				c->x != wc.x || c->y != wc.y ||
				c->w != wc.width || c->h != wc.height;
			if (geom_differ) {
				XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
			}

			c->x = wc.x;
			c->y = wc.y;
			c->w = wc.width;
			c->h = wc.height;
		}

		if (n_tileable == 1) {
			update_borders();
			continue;
		}

		int border_width = 2 * user_config.border_width;
		int n_stack = n_tileable - 1;
		int min_stack_height = border_width + 1;
		int total_fixed_heights = 0;
		int n_auto = 0; /* automatically take up leftover space */
		int heights_final[MAX_CLIENTS] = {0};

		for (int i = 1 ; i < n_tileable; i++) { /* i=1 - we are excluding master */
			if (tileable[i]->custom_stack_height > 0) {
				total_fixed_heights += tileable[i]->custom_stack_height;
			}
			else {
				n_auto++;
			}
		}

		int total_vgaps = (n_stack - 1) * gaps;
		int remaining = tile_height - total_fixed_heights - total_vgaps;

		if (n_auto > 0 && remaining >= n_auto * min_stack_height) {
			int used = 0;
			int count = 0;
			int auto_height = remaining / n_auto;

			for (int i = 1; i < n_tileable; i++) {
				if (tileable[i]->custom_stack_height > 0) {
					heights_final[i] = tileable[i]->custom_stack_height;
				}
				else {
					count++;
					heights_final[i] = (count < n_auto) ? auto_height : remaining - used;
					used += auto_height;
				}
			}
		}
		else {
			for (int i = 1; i < n_tileable; i++) {
				if (tileable[i]->custom_stack_height > 0) {
					heights_final[i] = tileable[i]->custom_stack_height;
				}
				else {
					heights_final[i] = min_stack_height;
				}
			}
		}
		int total_height = total_vgaps;
		for (int i = 1; i < n_tileable; i++) {
			total_height += heights_final[i];
		}
		int overfill = total_height - tile_height;
		if (overfill > 0) {
			/* shrink from top down, excluding bottom */
			for (int i = 1; i < n_tileable - 1 && overfill > 0; i++) {
				int shrink = MIN(overfill, heights_final[i] - min_stack_height);
				heights_final[i] -= shrink;
				overfill -= shrink;
			}
		}

		/* if its not perfectly filled stretch bottom to absorb remainder */
		int actual_height = total_vgaps;
		for (int i = 1; i < n_tileable; i++) {
			actual_height += heights_final[i];
		}
		int shortfall = tile_height - actual_height;
		if (shortfall > 0) {
			heights_final[n_tileable - 1] += shortfall;
		}

		int stack_y = tile_y;
		for (int i = 1; i < n_tileable; i++) {
			Client *c = tileable[i];
			XWindowChanges wc = {
				.x = tile_x + master_width + gaps,
				.y = stack_y,
				.width = MAX(1, stack_width - (2 * user_config.border_width)),
				.height = MAX(1, heights_final[i] - (2 * user_config.border_width)),
				.border_width = user_config.border_width
			};

			Bool geom_differ =
				c->x != wc.x || c->y != wc.y ||
				c->w != wc.width || c->h != wc.height;
			if (geom_differ) {
				XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
			}

			c->x = wc.x;
			c->y = wc.y;
			c->w = wc.width;
			c->h = wc.height;

			stack_y += heights_final[i] + gaps;
		}
		update_borders();
	}
}

void toggle_floating(void)
{
	if (!focused) {
		return;
	}

	if (focused->fullscreen) {
		focused->fullscreen = False;
		tile();
		XSetWindowBorderWidth(dpy, focused->win, user_config.border_width);
	}

	focused->floating = !focused->floating;

	if (focused->floating) {
		XWindowAttributes wa;
		if (XGetWindowAttributes(dpy, focused->win, &wa)) {
			focused->x = wa.x;
			focused->y = wa.y;
			focused->w = wa.width;
			focused->h = wa.height;

			XConfigureWindow(dpy, focused->win, CWX | CWY | CWWidth | CWHeight, &(XWindowChanges){
					.x = focused->x,
					.y = focused->y,
					.width = focused->w,
					.height = focused->h
			});
		}
	}
	else {
		focused->mon = get_monitor_for(focused);
	}

	if (!focused->floating) {
		focused->mon = get_monitor_for(focused);
	}
	tile();
	update_borders();

	/* raise and refocus floating window */
	if (focused->floating) {
		set_input_focus(focused, True, False);
	}
}

void toggle_floating_global(void)
{
	global_floating = !global_floating;
	Bool any_tiled = False;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (!c->floating) {
			any_tiled = True;
			break;
		}
	}

	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		c->floating = any_tiled;
		if (c->floating) {
			XWindowAttributes wa;
			XGetWindowAttributes(dpy, c->win, &wa);
			c->x = wa.x;
			c->y = wa.y;
			c->w = wa.width;
			c->h = wa.height;

			XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight,
			                 &(XWindowChanges){.x = c->x, .y = c->y, .width = c->w, .height = c->h});
			XRaiseWindow(dpy, c->win);
		}
	}

	tile();
	update_borders();
}

void toggle_fullscreen(void)
{
	if (!focused) {
		return;
	}
	apply_fullscreen(focused, !focused->fullscreen);
}

void toggle_monocle(void)
{
	monocle = !monocle;
	tile();
	update_borders();
	if (focused) {
		set_input_focus(focused, True, True);
	}
}

void toggle_scratchpad(int n)
{
	if (n < 0 || n >= MAX_SCRATCHPADS || scratchpads[n].client == NULL) {
		return;
	}

	Client *c = scratchpads[n].client;

	if (c->ws != current_ws) {
		/* unlink from old workspace */
		Client **pp = &workspaces[c->ws];
		while (*pp && *pp != c) {
			pp = &(*pp)->next;
		}
		if (*pp) {
			*pp = c->next;
		}

		/* link to current workspace */
		c->next = workspaces[current_ws];
		workspaces[current_ws] = c;
		c->ws = current_ws;

		long desktop = current_ws;
		XChangeProperty(dpy, c->win, _NET_WM_DESKTOP, XA_CARDINAL, 32,
				        PropModeReplace, (unsigned char *)&desktop, 1);
	}

	c->mon = focused ? focused->mon : current_mon;

	if (scratchpads[n].enabled) {
		XUnmapWindow(dpy, c->win);
		c->mapped = False;
		scratchpads[n].enabled = False;
		focus_prev();
	}
	else {
		XMapWindow(dpy, c->win);
		c->mapped = True;
		scratchpads[n].enabled = True;

		focused = c;
		set_input_focus(focused, True, True);
	}

	tile();
	update_borders();
	update_net_client_list();
}

void unswallow_window(Client *c)
{
	if (!c || !c->swallower) {
		return;
	}

	Client *swallower = c->swallower;

	/* unlink windows */
	swallower->swallowed = NULL;
	c->swallower = NULL;

	if (swallower->win) {
		XMapWindow(dpy, swallower->win);
		swallower->mapped = True;

		focused = swallower;
		set_input_focus(focused, False, True);
	}

	tile();
	update_borders();
}

void update_borders(void)
{
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		XSetWindowBorder(dpy, c->win, (c == focused ? user_config.border_foc_col : user_config.border_ufoc_col));
	}
	if (focused) {
		Window w = focused->win;
		XChangeProperty(dpy, root, _NET_ACTIVE_WINDOW, XA_WINDOW, 32,
				        PropModeReplace, (unsigned char *)&w, 1);
	}
}

void update_client_desktop_properties(void)
{
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			long desktop = ws;
			XChangeProperty(dpy, c->win, _NET_WM_DESKTOP, XA_CARDINAL, 32,
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
			if (keycode == num) {
				numlock_mask = (1u << i); /* which mod bit == NumLock key */
			}
			if (keycode == mode) {
				mode_switch_mask = (1u << i); /* which mod bit == Mode_switch key */
			}
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
		XDefineCursor(dpy, scr_root, cursor_normal);
	}

	if (XineramaIsActive(dpy)) {
		info = XineramaQueryScreens(dpy, &n_mons);
		mons = malloc(sizeof *mons * n_mons);
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
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			wins[n++] = c->win;
		}
	}
	XChangeProperty(dpy, root, _NET_CLIENT_LIST, XA_WINDOW, 32,
			        PropModeReplace, (unsigned char *)wins, n);
}

void update_struts(void)
{
	/* reset all reserves */
	for (int i = 0; i < n_mons; i++) {
		mons[i].reserve_left = mons[i].reserve_right = mons[i].reserve_top = mons[i].reserve_bottom = 0;
	}

	Window root_ret, parent_ret, *children;
	unsigned int nchildren;

	if (!XQueryTree(dpy, root, &root_ret, &parent_ret, &children, &nchildren)) {
		return;
	}

	for (unsigned int i = 0; i < nchildren; i++) {
		Window w = children[i];

		Atom actual_type;
		int actual_format;
		unsigned long n_items, bytes_after;
		Atom *types = NULL;

		if (XGetWindowProperty(dpy, w, _NET_WM_WINDOW_TYPE, 0, 4, False, XA_ATOM, &actual_type, &actual_format,
		                       &n_items, &bytes_after, (unsigned char **)&types) != Success || !types) {
			continue;
		}

		Bool is_dock = False;
		for (unsigned long j = 0; j < n_items; j++) {
			if (types[j] == _NET_WM_WINDOW_TYPE_DOCK) {
				is_dock = True;
				break;
			}
		}
		XFree(types);
		if (!is_dock) {
			continue;
		}

		long *str = NULL;
		Atom actual;
		int sfmt;
		unsigned long len, rem;

		if (XGetWindowProperty(dpy, w, _NET_WM_STRUT_PARTIAL, 0, 12, False, XA_CARDINAL, &actual, &sfmt, &len, &rem,
		                       (unsigned char **)&str) == Success && str && len >= 4) {
			XWindowAttributes wa;
			if (XGetWindowAttributes(dpy, w, &wa)) {
				/* find the monitor this dock belongs to */
				for (int m = 0; m < n_mons; m++) {
					if (wa.x >= mons[m].x && wa.x < mons[m].x + mons[m].w && wa.y >= mons[m].y &&
					    wa.y < mons[m].y + mons[m].h) {
						mons[m].reserve_left = MAX(mons[m].reserve_left, str[0]);
						mons[m].reserve_right = MAX(mons[m].reserve_right, str[1]);
						mons[m].reserve_top = MAX(mons[m].reserve_top, str[2]);
						mons[m].reserve_bottom = MAX(mons[m].reserve_bottom, str[3]);
					}
				}
			}
			XFree(str);
		}
	}
	XFree(children);
	update_workarea();
}

void update_workarea(void)
{
	long workarea[4 * MAX_MONITORS];

	for (int i = 0; i < n_mons && i < MAX_MONITORS; i++) {
		workarea[i * 4 + 0] = mons[i].x + mons[i].reserve_left;
		workarea[i * 4 + 1] = mons[i].y + mons[i].reserve_top;
		workarea[i * 4 + 2] = mons[i].w - mons[i].reserve_left - mons[i].reserve_right;
		workarea[i * 4 + 3] = mons[i].h - mons[i].reserve_top - mons[i].reserve_bottom;
	}

	XChangeProperty(dpy, root, _NET_WORKAREA, XA_CARDINAL, 32,
			        PropModeReplace, (unsigned char *)workarea, n_mons * 4);
}

void warp_cursor(Client *c)
{
	if (!c) {
		return;
	}

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
	Atom *atoms = NULL;

	if (XGetWindowProperty(dpy, w, _NET_WM_STATE, 0, 1024, False, XA_ATOM, &type,
		&format, &n_atoms, &unread, (unsigned char**)&atoms) == Success && atoms) {

		for (unsigned long i = 0; i < n_atoms; i++) {
			if (atoms[i] == state) {
				XFree(atoms);
				return True;
			}
		}
		XFree(atoms);
	}
	return False;
}

void window_set_ewmh_state(Window w, Atom state, Bool add)
{
	Atom type;
	int format;
	unsigned long n_atoms = 0;
	unsigned long unread = 0;
	Atom *atoms = NULL;

	if (XGetWindowProperty(dpy, w, _NET_WM_STATE, 0, 1024, False, XA_ATOM, &type,
		&format, &n_atoms, &unread, (unsigned char**)&atoms) != Success) {
		atoms = NULL;
		n_atoms = 0;
	}

	/* build new list */
	Atom buf[16];
	Atom *list = buf;
	unsigned long list_len = 0;

	if (atoms) {
		for (unsigned long i = 0; i < n_atoms; i++) {
			if (atoms[i] != state) {
				list[list_len++] = atoms[i];
			}	
		}
	}
	if (add && list_len < 16) {
		list[list_len++] = state;
	}

	if (list_len == 0) {
		XDeleteProperty(dpy, w, _NET_WM_STATE);
	}
	else {
		XChangeProperty(dpy, w, _NET_WM_STATE, XA_ATOM, 32,
				        PropModeReplace, (unsigned char*)list, list_len);
	}

	if (atoms) {
		XFree(atoms);
	}	
}

Bool window_should_float(Window w)
{
	XClassHint ch = {0};
	if (XGetClassHint(dpy, w, &ch)) {
		for (int i = 0; i < MAX_ITEMS; i++) {
			if (!user_config.should_float[i] || !user_config.should_float[i][0]) {
				break;
			}

			if ((ch.res_class && !strcmp(ch.res_class, user_config.should_float[i][0])) ||
			    (ch.res_name && !strcmp(ch.res_name, user_config.should_float[i][0]))) {
				XFree(ch.res_class);
				XFree(ch.res_name);
				return True;
			}
		}
		XFree(ch.res_class);
		XFree(ch.res_name);
	}

	return False;
}

Bool window_should_start_fullscreen(Window w)
{
	XClassHint ch = {0};
	if (XGetClassHint(dpy, w, &ch)) {
		for (int i = 0; i < MAX_ITEMS; i++) {
			if (!user_config.start_fullscreen[i] || !user_config.start_fullscreen[i][0]) {
				break;
			}

			if ((ch.res_class && !strcmp(ch.res_class, user_config.start_fullscreen[i][0])) ||
			    (ch.res_name && !strcmp(ch.res_name, user_config.start_fullscreen[i][0]))) {
				XFree(ch.res_class);
				XFree(ch.res_name);
				return True;
			}
		}
		XFree(ch.res_class);
		XFree(ch.res_name);
	}

	return False;
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
		if ((ignore[i].req == 0 || ignore[i].req == ee->request_code) && (ignore[i].code == ee->error_code)) {
			return 0;
		}
	}

	return 0;
	(void)d;
	(void)ee;
}

void xev_case(XEvent *xev)
{
	if (xev->type >= 0 && xev->type < LASTEvent) {
		evtable[xev->type](xev);
	}
	else {
		fprintf(stderr, "sxwm: invalid event type: %d\n", xev->type);
	}
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
