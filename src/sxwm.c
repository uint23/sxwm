/*
 *  See LICENSE for more info
 *
 *  simple xorg window manager
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

#include <X11/X.h>
#include <err.h>
#include <stdio.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xinerama.h>
#include <X11/Xcursor/Xcursor.h>

#include "defs.h"
#include "parser.h"

Client *add_client(Window w, int ws);
/* void centre_window(); */
void change_workspace(int ws);
int clean_mask(int mask);
/* void close_focused(void); */
/* void dec_gaps(void); */
Window find_toplevel(Window w);
/* void focus_next(void); */
/* void focus_prev(void); */
int get_monitor_for(Client *c);
pid_t get_pid(Window w);
int get_workspace_for_window(Window w);
void grab_keys(void);
void hdl_button(XEvent *xev);
void hdl_button_release(XEvent *xev);
void hdl_client_msg(XEvent *xev);
void hdl_config_ntf(XEvent *xev);
void hdl_config_req(XEvent *xev);
void hdl_dummy(XEvent *xev);
void hdl_destroy_ntf(XEvent *xev);
void hdl_keypress(XEvent *xev);
void hdl_map_req(XEvent *xev);
void hdl_motion(XEvent *xev);
void hdl_root_property(XEvent *xev);
void hdl_unmap_ntf(XEvent *xev);
/* void inc_gaps(void); */
void init_defaults(void);
Bool is_child_proc(pid_t pid1, pid_t pid2);
/* void move_master_next(void); */
/* void move_master_prev(void); */
void move_to_workspace(int ws);
void other_wm(void);
int other_wm_err(Display *dpy, XErrorEvent *ee);
/* long parse_col(const char *hex); */
/* void quit(void); */
/* void reload_config(void); */
void remove_scratchpad(int n);
/* void resize_master_add(void); */
/* void resize_master_sub(void); */
/* void resize_stack_add(void); */
/* void resize_stack_sub(void); */
void run(void);
void scan_existing_windows(void);
void send_wm_take_focus(Window w);
void setup(void);
void setup_atoms(void);
void set_frame_extents(Window w);
void set_win_scratchpad(int n);
int snap_coordinate(int pos, int size, int screen_size, int snap_dist);
void spawn(const char **argv);
void startup_exec(void);
void swallow_window(Client *swallower, Client *swallowed);
void swap_clients(Client *a, Client *b);
void tile(void);
/* void toggle_floating(void); */
/* void toggle_floating_global(void); */
/* void toggle_fullscreen(void); */
void toggle_scratchpad(int n);
void unswallow_window(Client *c);
void update_borders(void);
void update_client_desktop_properties(void);
void update_monitors(void);
void update_net_client_list(void);
void update_struts(void);
void update_workarea(void);
void warp_cursor(Client *c);
Bool window_should_float(Window w);
Bool window_should_start_fullscreen(Window w);
int xerr(Display *dpy, XErrorEvent *ee);
void xev_case(XEvent *xev);
#include "config.h"

Atom atom_net_active_window;
Atom atom_net_current_desktop;
Atom atom_net_supported;
Atom atom_net_wm_state;
Atom atom_wm_window_type;
Atom atom_net_wm_window_type_dock;
Atom atom_net_workarea;
Atom atom_wm_delete;
Atom atom_wm_strut;
Atom atom_wm_strut_partial;
Atom atom_net_supporting_wm_check;
Atom atom_net_wm_name;
Atom atom_utf8_string;
Atom atom_net_wm_desktop;
Atom atom_net_client_list;
Atom atom_net_frame_extents;

Cursor c_normal, c_move, c_resize;
Client *workspaces[NUM_WORKSPACES] = {NULL};
Config default_config;
Config user_config;
int current_ws = 0;
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
int monsn = 0;
int current_monitor = 0;
Bool global_floating = False;
Bool in_ws_switch = False;
Bool backup_binds = False;
Bool running = False;

long last_motion_time = 0;
int scr_width;
int scr_height;
int open_windows = 0;
int drag_start_x, drag_start_y;
int drag_orig_x, drag_orig_y, drag_orig_w, drag_orig_h;

int reserve_left = 0;
int reserve_right = 0;
int reserve_top = 0;
int reserve_bottom = 0;

Bool next_should_float = False;

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
	XSelectInput(dpy, w,
	             EnterWindowMask | LeaveWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask |
	                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	XGrabButton(dpy, Button1, 0, w, False, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button1, user_config.modkey, w, False, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button1, user_config.modkey | ShiftMask, w, False, ButtonPressMask, GrabModeSync, GrabModeAsync,
	            None, None);
	XGrabButton(dpy, Button3, user_config.modkey, w, False, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);

	Atom protos[] = {atom_wm_delete};
	XSetWMProtocols(dpy, w, protos, 1);

	XWindowAttributes wa;
	XGetWindowAttributes(dpy, w, &wa);
	c->x = wa.x;
	c->y = wa.y;
	c->w = wa.width;
	c->h = wa.height;

	/* set monitor based on pointer location */
	Window root_ret, child_ret;
	int root_x, root_y, win_x, win_y;
	unsigned int mask;
	int pointer_mon = 0;

	if (XQueryPointer(dpy, root, &root_ret, &child_ret, &root_x, &root_y, &win_x, &win_y, &mask)) {
		for (int i = 0; i < monsn; i++) {
			if (root_x >= mons[i].x && root_x < mons[i].x + mons[i].w && root_y >= mons[i].y &&
			    root_y < mons[i].y + mons[i].h) {
				pointer_mon = i;
				break;
			}
		}
	}

	c->mon = pointer_mon;
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
	}

	long desktop = ws;
	XChangeProperty(dpy, w, XInternAtom(dpy, "_NET_WM_DESKTOP", False), XA_CARDINAL, 32, PropModeReplace,
	                (unsigned char *)&desktop, 1);

	XRaiseWindow(dpy, w);
	return c;
}

void centre_window()
{
	if (!focused || !focused->mapped || !focused->floating) {
		return;
	}

	int x = mons[focused->mon].x + (mons[focused->mon].w - focused->w) / 2;
	int y = mons[focused->mon].y + (mons[focused->mon].h - focused->h) / 2;

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
	XGrabServer(dpy);

	/* unmap those still marked mapped */
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped) {
			XUnmapWindow(dpy, c->win);
		}
	}

	current_ws = ws;

	/* map those still marked mapped */
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped) {
			XMapWindow(dpy, c->win);
		}
	}

	tile();

	focused = NULL;
	if (workspaces[current_ws]) {
		focused = workspaces[current_ws];
		Window focused_win = find_toplevel(focused->win);
		XSetInputFocus(dpy, focused_win, RevertToPointerRoot, CurrentTime);
		if (user_config.warp_cursor) {
			warp_cursor(focused);
		}
		update_borders();
	}
	else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}

	long cd = current_ws;
	XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False), XA_CARDINAL, 32, PropModeReplace,
	                (unsigned char *)&cd, 1);
	update_client_desktop_properties();

	XUngrabServer(dpy);
	XSync(dpy, False);
	in_ws_switch = False;
}

int clean_mask(int mask)
{
	return mask & ~(LockMask | Mod2Mask | Mod3Mask);
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

	Atom *protos;
	int n;
	if (XGetWMProtocols(dpy, focused->win, &protos, &n) && protos) {
		for (int i = 0; i < n; i++) {
			if (protos[i] == atom_wm_delete) {
				XEvent ev = {.xclient = {.type = ClientMessage,
				                         .window = focused->win,
				                         .message_type = XInternAtom(dpy, "WM_PROTOCOLS", False),
				                         .format = 32}};
				ev.xclient.data.l[0] = atom_wm_delete;
				ev.xclient.data.l[1] = CurrentTime;
				XSendEvent(dpy, focused->win, False, NoEventMask, &ev);
				XFree(protos);
				return;
			}
		}
		XUnmapWindow(dpy, focused->win);
		XFree(protos);
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

void startup_exec(void)
{
	for (int i = 0; i < 256; i++) {
		if (user_config.torun[i]) {
			const char **argv = build_argv(user_config.torun[i]);
			if (argv) {
				spawn(argv);
				for (int j = 0; argv[j]; j++) {
					free((char *)argv[j]);
				}
				free(argv);
			}
		}
	}
}

Window find_toplevel(Window w)
{
	Window root = None;
	Window parent;
	Window *kids;
	unsigned nkids;

	while (True) {
		if (w == root) {
			break;
		}
		if (XQueryTree(dpy, w, &root, &parent, &kids, &nkids) == 0) {
			break;
		}
		XFree(kids);
		if (parent == root || parent == None) {
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
	} while (!c->mapped && c != start);

	/* this stops invisible windows being detected or focused */
	if (!c->mapped) {
		return;
	}

	focused = c;
	current_monitor = c->mon;
	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, c->win);
	if (user_config.warp_cursor)
		warp_cursor(c);
	update_borders();
}

void focus_prev(void)
{
	if (!workspaces[current_ws]) {
		return;
	}

	Client *start = focused ? focused : workspaces[current_ws];
	Client *c = start;

	/* loop until we find a mapped client or return to start */
	do {
		Client *p = workspaces[current_ws], *prev = NULL;
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
	} while (!c->mapped && c != start);

	/* this stops invisible windows being detected or focused */
	if (!c->mapped) {
		return;
	}

	focused = c;
	current_monitor = c->mon;

	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, c->win);
	if (user_config.warp_cursor) {
		warp_cursor(c);
	}
	update_borders();
}

void focus_next_mon(void)
{
	if (monsn <= 1) {
		return; /* only one monitor, nothing to switch to */
	}

	/* use current_monitor if no focused window, otherwise use focused window's monitor */
	int current_mon = focused ? focused->mon : current_monitor;
	int target_mon = (current_mon + 1) % monsn;

	/* find the first window on the target monitor in current workspace */
	Client *target_client = NULL;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mon == target_mon && c->mapped && !c->fullscreen) {
			target_client = c;
			break;
		}
	}

	if (target_client) {
		/* focus the window on target monitor */
		focused = target_client;
		current_monitor = target_mon;
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
		XRaiseWindow(dpy, focused->win);
		if (user_config.warp_cursor) {
			warp_cursor(focused);
		}
		update_borders();
	}
	else {
		/* no windows on target monitor, just move cursor to center and update current_monitor */
		current_monitor = target_mon;
		int center_x = mons[target_mon].x + mons[target_mon].w / 2;
		int center_y = mons[target_mon].y + mons[target_mon].h / 2;
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, center_x, center_y);
		XSync(dpy, False);
	}
}

void focus_prev_mon(void)
{
	if (monsn <= 1) {
		return; /* only one monitor, nothing to switch to */
	}

	/* use current_monitor if no focused window, otherwise use focused window's monitor */
	int current_mon = focused ? focused->mon : current_monitor;
	int target_mon = (current_mon - 1 + monsn) % monsn;

	/* find the first window on the target monitor in current workspace */
	Client *target_client = NULL;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mon == target_mon && c->mapped && !c->fullscreen) {
			target_client = c;
			break;
		}
	}

	if (target_client) {
		/* focus the window on target monitor */
		focused = target_client;
		current_monitor = target_mon;
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
		XRaiseWindow(dpy, focused->win);
		if (user_config.warp_cursor) {
			warp_cursor(focused);
		}
		update_borders();
	}
	else {
		/* no windows on target monitor, just move cursor to center and update current_monitor */
		current_monitor = target_mon;
		int center_x = mons[target_mon].x + mons[target_mon].w / 2;
		int center_y = mons[target_mon].y + mons[target_mon].h / 2;
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, center_x, center_y);
		XSync(dpy, False);
	}
}

void move_next_mon(void)
{
	if (!focused || monsn <= 1) {
		return; /* no focused window or only one monitor */
	}

	int target_mon = (focused->mon + 1) % monsn;

	/* update window's monitor assignment */
	focused->mon = target_mon;
	current_monitor = target_mon;

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
	if (!focused || monsn <= 1) {
		return; /* no focused window or only one monitor */
	}

	int target_mon = (focused->mon - 1 + monsn) % monsn;

	/* update window's monitor assignment */
	focused->mon = target_mon;
	current_monitor = target_mon;

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

int get_monitor_for(Client *c)
{
	int cx = c->x + c->w / 2, cy = c->y + c->h / 2;
	for (int i = 0; i < monsn; i++) {
		if (cx >= (int)mons[i].x && cx < mons[i].x + mons[i].w && cy >= (int)mons[i].y && cy < mons[i].y + mons[i].h) {
			return i;
		}
	}
	return 0;
}

pid_t get_pid(Window w)
{
	pid_t pid = 0;
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *prop = NULL;
	Atom atom_pid = XInternAtom(dpy, "_NET_WM_PID", False);

	if (XGetWindowProperty(dpy, w, atom_pid, 0, 1, False, XA_CARDINAL, &actual_type, &actual_format, &nitems,
	                       &bytes_after, &prop) == Success &&
	    prop) {
		if (actual_format == 32 && nitems == 1) {
			pid = *(pid_t *)prop;
		}
		XFree(prop);
	}
	return pid;
}

int get_workspace_for_window(Window w)
{
	XClassHint ch;
	if (!XGetClassHint(dpy, w, &ch)) {
		return current_ws; /* default to current workspace */
	}

	for (int i = 0; i < 256; i++) {
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
	return current_ws; /* default to current workspace */
}

void grab_keys(void)
{
	const int guards[] = {0,
	                      LockMask,
	                      Mod2Mask,
	                      LockMask | Mod2Mask,
	                      Mod5Mask,
	                      LockMask | Mod5Mask,
	                      Mod2Mask | Mod5Mask,
	                      LockMask | Mod2Mask | Mod5Mask};
	XUngrabKey(dpy, AnyKey, AnyModifier, root);

	for (int i = 0; i < user_config.bindsn; i++) {
		Binding *b = &user_config.binds[i];

		if ((b->type == TYPE_WS_CHANGE && b->mods != user_config.modkey) ||
		    (b->type == TYPE_WS_MOVE && b->mods != (user_config.modkey | ShiftMask))) {
			continue;
		}

		KeyCode kc = XKeysymToKeycode(dpy, b->keysym);
		if (!kc) {
			continue;
		}

		for (size_t g = 0; g < sizeof guards / sizeof *guards; g++) {
			XGrabKey(dpy, kc, b->mods | guards[g], root, True, GrabModeAsync, GrabModeAsync);
		}
	}
}

void hdl_button(XEvent *xev)
{
	XButtonEvent *e = &xev->xbutton;
	Window w = (e->subwindow != None) ? e->subwindow : e->window;
	w = find_toplevel(w);

	XAllowEvents(dpy, ReplayPointer, e->time);
	if (!w) {
		return;
	}

	Client *head = workspaces[current_ws];
	for (Client *c = head; c; c = c->next) {
		if (c->win != w) {
			continue;
		}

		/* begin swap drag mode */
		if ((e->state & user_config.modkey) && (e->state & ShiftMask) && e->button == Button1 && !c->floating) {
			drag_client = c;
			drag_start_x = e->x_root;
			drag_start_y = e->y_root;
			drag_orig_x = c->x;
			drag_orig_y = c->y;
			drag_orig_w = c->w;
			drag_orig_h = c->h;
			drag_mode = DRAG_SWAP;
			XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None,
			             c_move, CurrentTime);
			focused = c;
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			XSetWindowBorder(dpy, c->win, user_config.border_swap_col);
			XRaiseWindow(dpy, c->win);
			return;
		}

		if ((e->state & user_config.modkey) && (e->button == Button1 || e->button == Button3) && !c->floating) {
			focused = c;
			toggle_floating();
		}

		if (!(e->state & user_config.modkey) && e->button == Button1) {
			focused = c;
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			send_wm_take_focus(c->win);
			XRaiseWindow(dpy, c->win);
			update_borders();
			return;
		}

		if (!c->floating) {
			return;
		}

		if (c->fixed && e->button == Button3) {
			return;
		}

		Cursor cur = (e->button == Button1) ? c_move : c_resize;
		XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, cur,
		             CurrentTime);

		drag_client = c;
		drag_start_x = e->x_root;
		drag_start_y = e->y_root;
		drag_orig_x = c->x;
		drag_orig_y = c->y;
		drag_orig_w = c->w;
		drag_orig_h = c->h;
		drag_mode = (e->button == Button1) ? DRAG_MOVE : DRAG_RESIZE;
		focused = c;

		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		update_borders();
		XRaiseWindow(dpy, c->win);
		return;
	}
}

void hdl_button_release(XEvent *xev)
{
	(void)xev;

	if (drag_mode == DRAG_SWAP) {
		if (swap_target) {
			XSetWindowBorder(dpy, swap_target->win,
			                 (swap_target == focused ? user_config.border_foc_col : user_config.border_ufoc_col));
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
	/* clickable bar workspace switching */
	if (xev->xclient.message_type == atom_net_current_desktop) {
		int ws = (int)xev->xclient.data.l[0];
		change_workspace(ws);
		return;
	}
}

void hdl_config_ntf(XEvent *xev)
{
	if (xev->xconfigure.window == root) {
		update_monitors();
		tile();
		update_borders();
	}
}

void hdl_config_req(XEvent *xev)
{
	XConfigureRequestEvent *e = &xev->xconfigurerequest;
	Client *c = NULL;

	for (int ws = 0; ws < NUM_WORKSPACES && !c; ws++) {
		for (c = workspaces[ws]; c; c = c->next) {
			if (c->win == e->window) {
				break;
			}
		}
	}

	if (!c || c->floating || c->fullscreen) {
		/* allow client to configure itself */
		XWindowChanges wc = {.x = e->x,
		                     .y = e->y,
		                     .width = e->width,
		                     .height = e->height,
		                     .border_width = e->border_width,
		                     .sibling = e->above,
		                     .stack_mode = e->detail};
		XConfigureWindow(dpy, e->window, e->value_mask, &wc);
		return;
	}

	if (c->fixed) {
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

	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		Client *prev = NULL, *c = workspaces[ws];
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
				if (c->next) {
					focused = c->next;
				}
				else if (prev) {
					focused = prev;
				}
				else {
					if (ws == current_ws) {
						focused = NULL;
					}
				}
			}

			if (!prev) {
				workspaces[ws] = c->next;
			}
			else {
				prev->next = c->next;
			}

			free(c);
			update_net_client_list();
			open_windows--;

			if (ws == current_ws) {
				tile();
				update_borders();

				if (focused) {
					XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
					XRaiseWindow(dpy, focused->win);
				}
			}
			return;
		}
	}
}

void hdl_keypress(XEvent *xev)
{
	KeySym ks = XkbKeycodeToKeysym(dpy, xev->xkey.keycode, 0, 0);
	int mods = clean_mask(xev->xkey.state);

	for (int i = 0; i < user_config.bindsn; i++) {
		Binding *b = &user_config.binds[i];
		if (b->keysym == ks && clean_mask(b->mods) == mods) {
			switch (b->type) {
				case TYPE_CMD:
					spawn(b->action.cmd);
					break;

				case TYPE_FUNC:
					if (b->action.fn) {
						b->action.fn();
					}
					break;
				case TYPE_WS_CHANGE:
					change_workspace(b->action.ws);
					update_net_client_list();
					break;
				case TYPE_WS_MOVE:
					move_to_workspace(b->action.ws);
					update_net_client_list();
					break;
				case TYPE_SP_REMOVE:
					remove_scratchpad(b->action.sp);
					break;
				case TYPE_SP_TOGGLE:
					toggle_scratchpad(b->action.sp);
					break;
				case TYPE_SP_CREATE:
					set_win_scratchpad(b->action.sp);
					break;
			}
			return;
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

void hdl_map_req(XEvent *xev)
{
	Window w = xev->xmaprequest.window;
	XWindowAttributes wa;

	if (!XGetWindowAttributes(dpy, w, &wa)) {
		return;
	}

	if (wa.override_redirect || wa.width <= 0 || wa.height <= 0) {
		XMapWindow(dpy, w);
		return;
	}

	/* check if this window is already managed on any workspace */
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			if (c->win == w) {
				if (ws == current_ws) {
					if (!c->mapped) {
						XMapWindow(dpy, w);
						c->mapped = True;
					}
					if (user_config.new_win_focus) {
						focused = c;
						XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
						send_wm_take_focus(c->win);
						if (user_config.warp_cursor) {
							warp_cursor(c);
						}
					}
					update_borders();
				}
				return;
			}
		}
	}

	Atom type;
	int format;
	unsigned long nitems, after;
	Atom *types = NULL;
	Bool should_float = False;

	if (XGetWindowProperty(dpy, w, atom_wm_window_type, 0, 8, False, XA_ATOM, &type, &format, &nitems, &after,
	                       (unsigned char **)&types) == Success &&
	    types) {
		Atom dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
		Atom util = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False);
		Atom dialog = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
		Atom toolbar = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
		Atom splash = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", False);
		Atom popup = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);

		for (unsigned long i = 0; i < nitems; i++) {
			if (types[i] == dock) {
				XFree(types);
				XMapWindow(dpy, w);
				return;
			}
			if (types[i] == util || types[i] == dialog || types[i] == toolbar || types[i] == splash ||
			    types[i] == popup) {
				should_float = True;
				break;
			}
		}
		XFree(types);
	}

	if (!should_float) {
		should_float = window_should_float(w);
	}

	if (open_windows == MAXCLIENTS) {
		fprintf(stderr, "sxwm: max clients reached, ignoring map request\n");
		return;
	}

	int target_ws = get_workspace_for_window(w);
	Client *c = add_client(w, target_ws);
	if (!c) {
		return;
	}

	Window tr;
	if (!should_float && XGetTransientForHint(dpy, w, &tr)) {
		should_float = True;
	}
	XSizeHints sh;
	long sup;
	if (!should_float && XGetWMNormalHints(dpy, w, &sh, &sup) && (sh.flags & PMinSize) && (sh.flags & PMaxSize) &&
	    sh.min_width == sh.max_width && sh.min_height == sh.max_height) {
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

	if (target_ws != current_ws) {
		update_net_client_list();
		return;
	}

	/* map & borders */
	update_net_client_list();
	if (!global_floating && !c->floating) {
		tile();
	}
	else if (c->floating) {
		XRaiseWindow(dpy, w);
	}

	/* check for swallowing opportunities */
	{
		XClassHint ch;
		Bool can_be_swallowed = False;

		if (XGetClassHint(dpy, w, &ch)) {
			/* check if new window can be swallowed */
			for (int i = 0; i < 256; i++) {
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

					XClassHint pch;
					Bool can_swallow = False;

					if (XGetClassHint(dpy, p->win, &pch)) {
						/* check if this existing window can swallow others */
						for (int i = 0; i < 256; i++) {
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
						if (can_swallow) {
							/* we know class matches — swallow now */
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

	XMapWindow(dpy, w);
	c->mapped = True;
	if (c->fullscreen) {
		int m = c->mon;
		XSetWindowBorderWidth(dpy, w, 0);
		XMoveResizeWindow(dpy, w, mons[m].x, mons[m].y, mons[m].w, mons[m].h);
	}
	set_frame_extents(w);

	if (user_config.new_win_focus) {
		focused = c;
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		send_wm_take_focus(c->win);
		if (user_config.warp_cursor) {
			warp_cursor(c);
		}
	}
	update_borders();
}

void hdl_motion(XEvent *xev)
{
	XMotionEvent *e = &xev->xmotion;

	if ((drag_mode == DRAG_NONE || !drag_client) ||
	    (e->time - last_motion_time <= (1000 / (long unsigned int)user_config.motion_throttle))) {
		return;
	}
	last_motion_time = e->time;

	/* figure out which monitor the pointer is in right now */
	int mon = 0;
	for (int i = 0; i < monsn; i++) {
		if (e->x_root >= mons[i].x && e->x_root < mons[i].x + mons[i].w && e->y_root >= mons[i].y &&
		    e->y_root < mons[i].y + mons[i].h) {
			mon = i;
			break;
		}
	}
	Monitor *m = &mons[mon];

	if (drag_mode == DRAG_SWAP) {
		Window root_ret, child;
		int rx, ry, wx, wy;
		unsigned int mask;
		XQueryPointer(dpy, root, &root_ret, &child, &rx, &ry, &wx, &wy, &mask);

		static Client *last_swap_target = NULL;
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
				    dpy, last_swap_target->win,
				    (last_swap_target == focused ? user_config.border_foc_col : user_config.border_ufoc_col));
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
		int dx = e->x_root - drag_start_x;
		int dy = e->y_root - drag_start_y;
		int nx = drag_orig_x + dx;
		int ny = drag_orig_y + dy;

		int outer_w = drag_client->w + 2 * user_config.border_width;
		int outer_h = drag_client->h + 2 * user_config.border_width;

		/* snap relative to this mons bounds: */
		int rel_x = nx - m->x;
		int rel_y = ny - m->y;

		rel_x = snap_coordinate(rel_x, outer_w, m->w, user_config.snap_distance);
		rel_y = snap_coordinate(rel_y, outer_h, m->h, user_config.snap_distance);

		nx = m->x + rel_x;
		ny = m->y + rel_y;

		if (!drag_client->floating && (UDIST(nx, drag_client->x) > user_config.snap_distance ||
		                               UDIST(ny, drag_client->y) > user_config.snap_distance)) {
			toggle_floating();
		}

		XMoveWindow(dpy, drag_client->win, nx, ny);
		drag_client->x = nx;
		drag_client->y = ny;
	}
	else if (drag_mode == DRAG_RESIZE) {
		int dx = e->x_root - drag_start_x;
		int dy = e->y_root - drag_start_y;
		int nw = drag_orig_w + dx;
		int nh = drag_orig_h + dy;

		/* clamp relative to this mon */
		int max_w = (m->w - (drag_client->x - m->x));
		int max_h = (m->h - (drag_client->y - m->y));

		drag_client->w = CLAMP(nw, MIN_WINDOW_SIZE, max_w);
		drag_client->h = CLAMP(nh, MIN_WINDOW_SIZE, max_h);

		XResizeWindow(dpy, drag_client->win, drag_client->w, drag_client->h);
	}
}

void hdl_root_property(XEvent *xev)
{
	XPropertyEvent *e = &xev->xproperty;
	if (e->atom == atom_net_current_desktop) {
		long *val = NULL;
		Atom actual;
		int fmt;
		unsigned long n, after;
		if (XGetWindowProperty(dpy, root, atom_net_current_desktop, 0, 1, False, XA_CARDINAL, &actual, &fmt, &n, &after,
		                       (unsigned char **)&val) == Success &&
		    val) {
			change_workspace((int)val[0]);
			XFree(val);
		}
	}
	else if (e->atom == atom_wm_strut_partial) {
		update_struts();
		tile();
		update_borders();
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

void update_struts(void)
{
	/* reset all reserves */
	for (int i = 0; i < monsn; i++) {
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

		if (XGetWindowProperty(dpy, w, atom_wm_window_type, 0, 4, False, XA_ATOM,
					&actual_type, &actual_format, &n_items, &bytes_after,
					(unsigned char **)&types) != Success || !types) {
			continue;
		}

		Bool is_dock = False;
		for (unsigned long j = 0; j < n_items; j++) {
			if (types[j] == atom_net_wm_window_type_dock) {
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

		if (XGetWindowProperty(dpy, w, atom_wm_strut_partial, 0, 12, False, XA_CARDINAL,
					&actual, &sfmt, &len, &rem, (unsigned char **)&str) == Success && str && len >= 4) {

			XWindowAttributes wa;
			if (XGetWindowAttributes(dpy, w, &wa)) {
				/* find the monitor this dock belongs to */
				for (int m = 0; m < monsn; m++) {
					if (wa.x >= mons[m].x && wa.x < mons[m].x + mons[m].w &&
							wa.y >= mons[m].y && wa.y < mons[m].y + mons[m].h) {
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

	for (int i = 0; i < monsn && i < MAX_MONITORS; i++) {
		workarea[i * 4 + 0] = mons[i].x + mons[i].reserve_left;
		workarea[i * 4 + 1] = mons[i].y + mons[i].reserve_top;
		workarea[i * 4 + 2] = mons[i].w - mons[i].reserve_left - mons[i].reserve_right;
		workarea[i * 4 + 3] = mons[i].h - mons[i].reserve_top - mons[i].reserve_bottom;
	}

	XChangeProperty(dpy, root, atom_net_workarea, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *)workarea, monsn * 4);
}

void inc_gaps(void)
{
	user_config.gaps++;
	tile();
	update_borders();
}

void init_defaults(void)
{
	default_config.modkey = Mod4Mask;
	default_config.gaps = 10;
	default_config.border_width = 1;
	default_config.border_foc_col = parse_col("#c0cbff");
	default_config.border_ufoc_col = parse_col("#555555");
	default_config.border_swap_col = parse_col("#fff4c0");

	for (int i = 0; i < MAX_MONITORS; i++) {
		default_config.master_width[i] = 50 / 100.0f;
	}

	for (int i = 0; i < 256; i++) {
		default_config.can_be_swallowed[i] = NULL;
		default_config.can_swallow[i] = NULL;
		default_config.open_in_workspace[i] = NULL;
		default_config.start_fullscreen[i] = NULL;
	}

	default_config.motion_throttle = 60;
	default_config.resize_master_amt = 5;
	default_config.resize_stack_amt = 20;
	default_config.snap_distance = 5;
	default_config.bindsn = 0;
	default_config.new_win_focus = True;
	default_config.warp_cursor = True;
	default_config.new_win_master = False;

	if (backup_binds) {
		for (unsigned long i = 0; i < LENGTH(binds); i++) {
			default_config.binds[i].mods = binds[i].mods;
			default_config.binds[i].keysym = binds[i].keysym;
			default_config.binds[i].action.cmd = binds[i].action.cmd;
			default_config.binds[i].type = binds[i].type;
			default_config.bindsn++;
		}
	}

	user_config = default_config;
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
			printf("sxwm: could not open %s\n", path);
			return False;
		}

		int ppid = 0;
		if (fscanf(f, "%*d %*s %*c %d", &ppid) != 1) {
			printf("sxwm: failed to read ppid from %s\n", path);
			fclose(f);
			return False;
		}
		fclose(f);

		if (ppid == parent_pid) {
			return True;
		}

		if (ppid <= 1) { /* Reached init or kernel */
			printf("sxwm: reached init/kernel, no relationship found\n");
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
	XChangeProperty(dpy, focused->win, XInternAtom(dpy, "_NET_WM_DESKTOP", False), XA_CARDINAL, 32, PropModeReplace,
	                (unsigned char *)&desktop, 1);

	/* tile current ws */
	tile();
	focused = workspaces[current_ws];
	if (focused) {
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	}
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

int other_wm_err(Display *dpy, XErrorEvent *ee)
{
	errx(0, "can't start because another window manager is already running");
	return 0;
	(void)dpy;
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
	XFreeCursor(dpy, c_move);
	XFreeCursor(dpy, c_normal);
	XFreeCursor(dpy, c_resize);
	printf("quitting...\n");
	running = False;
}

void reload_config(void)
{
	puts("sxwm: reloading config...");

	/* free binding commands without */
	for (int i = 0; i < user_config.bindsn; i++) {
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
	for (int i = 0; i < 256; i++) {
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
	for (int i = 0; i < 256; i++) {
		if (user_config.should_float[i]) {
			if (user_config.should_float[i][0]) {
				free(user_config.should_float[i][0]);
			}
			free(user_config.should_float[i]);
			user_config.should_float[i] = NULL;
		}
	}

	/* free any exec strings */
	for (int i = 0; i < 256; i++) {
		if (user_config.torun[i]) {
			free(user_config.torun[i]);
			user_config.torun[i] = NULL;
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
	XGrabButton(dpy, Button1, user_config.modkey, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button1, user_config.modkey | ShiftMask, root, True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button3, user_config.modkey, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			XGrabButton(dpy, Button1, 0, c->win, False, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
			XGrabButton(dpy, Button1, user_config.modkey, c->win, False, ButtonPressMask, GrabModeSync, GrabModeAsync,
			            None, None);
			XGrabButton(dpy, Button1, user_config.modkey | ShiftMask, c->win, False, ButtonPressMask, GrabModeSync,
			            GrabModeAsync, None, None);
			XGrabButton(dpy, Button3, user_config.modkey, c->win, False, ButtonPressMask, GrabModeSync, GrabModeAsync,
			            None, None);
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
	}

	scratchpads[n].client = NULL;
	scratchpads[n].enabled = False;
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
	Window root_return, parent_return;
	Window *children;
	unsigned int nchildren;

	if (XQueryTree(dpy, root, &root_return, &parent_return, &children, &nchildren)) {
		for (unsigned int i = 0; i < nchildren; i++) {
			XWindowAttributes wa;
			if (!XGetWindowAttributes(dpy, children[i], &wa) || wa.override_redirect || wa.map_state != IsViewable) {
				continue;
			}

			XEvent fake_event = {0};
			fake_event.type = MapRequest;
			fake_event.xmaprequest.window = children[i];
			hdl_map_req(&fake_event);
		}
		if (children) {
			XFree(children);
		}
	}
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
				    .xclient = {.type = ClientMessage, .window = w, .message_type = wm_protocols, .format = 32}};
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
	if ((dpy = XOpenDisplay(NULL)) == 0) {
		errx(0, "can't open display. quitting...");
	}
	root = XDefaultRootWindow(dpy);

	setup_atoms();
	other_wm();
	init_defaults();
	if (parser(&user_config)) {
		fprintf(stderr, "sxwmrc: error parsing config file\n");
		init_defaults();
	}
	grab_keys();
	startup_exec();

	c_normal = XcursorLibraryLoadCursor(dpy, "left_ptr");
	c_move = XcursorLibraryLoadCursor(dpy, "fleur");
	c_resize = XcursorLibraryLoadCursor(dpy, "bottom_right_corner");
	XDefineCursor(dpy, root, c_normal);

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));
	update_monitors();

	XSelectInput(dpy, root,
	             StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask |
	                 PropertyChangeMask);

	XGrabButton(dpy, Button1, user_config.modkey, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button1, user_config.modkey | ShiftMask, root, True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

	XGrabButton(dpy, Button3, user_config.modkey, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);
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
	evtable[MapRequest] = hdl_map_req;
	evtable[MotionNotify] = hdl_motion;
	evtable[PropertyNotify] = hdl_root_property;
	evtable[UnmapNotify] = hdl_unmap_ntf;
	scan_existing_windows();

	signal(SIGCHLD, SIG_IGN); /* prevent child processes from becoming zombies */
}

void setup_atoms(void)
{
	Atom a_num = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	Atom a_names = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	atom_net_current_desktop = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	atom_net_active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	atom_net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
	atom_wm_strut_partial = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
	atom_wm_strut = XInternAtom(dpy, "_NET_WM_STRUT", False); /* legacy struts */
	atom_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	atom_net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	atom_net_workarea = XInternAtom(dpy, "_NET_WORKAREA", False);
	atom_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	atom_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	atom_net_supporting_wm_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	atom_net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
	atom_utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
	atom_net_wm_desktop = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	atom_net_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	atom_net_frame_extents = XInternAtom(dpy, "_NET_FRAME_EXTENTS", False);

	Atom support_list[] = {
	    atom_net_current_desktop,
	    atom_net_active_window,
	    atom_net_supported,
	    atom_net_wm_state,
	    atom_wm_window_type,
	    atom_net_wm_window_type_dock,
	    atom_net_workarea,
	    atom_wm_strut,
	    atom_wm_strut_partial,
	    atom_wm_delete,
	    atom_net_supporting_wm_check,
	    atom_net_wm_name,
	    atom_utf8_string,
	    atom_net_wm_desktop,
	    atom_net_client_list,
		atom_net_frame_extents,
	};

	long num = NUM_WORKSPACES;
	XChangeProperty(dpy, root, a_num, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&num, 1);

	const char names[] = WORKSPACE_NAMES;
	int names_len = sizeof(names);

	XChangeProperty(dpy, root, a_names, XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace,
	                (unsigned char *)names, names_len);

	long initial = current_ws;
	XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False), XA_CARDINAL, 32, PropModeReplace,
	                (unsigned char *)&initial, 1);

	XChangeProperty(dpy, root, atom_net_supported, XA_ATOM, 32, PropModeReplace, (unsigned char *)support_list,
	                sizeof(support_list) / sizeof(Atom));

	update_workarea();
}

void set_frame_extents(Window w)
{
	long extents[4] = {user_config.border_width, user_config.border_width, user_config.border_width, user_config.border_width};
	XChangeProperty(dpy, w, atom_net_frame_extents, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)extents, 4);
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

Bool window_should_float(Window w)
{
	XClassHint ch;
	if (XGetClassHint(dpy, w, &ch)) {
		for (int i = 0; i < 256; i++) {
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

void spawn(const char **argv)
{
	int argc = 0;
	while (argv[argc])
		argc++;

	int cmd_count = 1;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "|") == 0) {
			cmd_count++;
		}
	}

	char ***commands = malloc(cmd_count * sizeof(char **));
	if (!commands) {
		perror("malloc commands");
		return;
	}

	int cmd_idx = 0;
	int arg_start = 0;
	for (int i = 0; i <= argc; i++) {
		if (!argv[i] || strcmp(argv[i], "|") == 0) {
			int len = i - arg_start;
			char **cmd_args = malloc((len + 1) * sizeof(char *));
			if (!cmd_args) {
				perror("malloc cmd_args");
				for (int j = 0; j < cmd_idx; j++) {
					free(commands[j]);
				}
				free(commands);
				return;
			}
			for (int j = 0; j < len; j++) {
				cmd_args[j] = (char *)argv[arg_start + j];
			}
			cmd_args[len] = NULL;
			commands[cmd_idx++] = cmd_args;
			arg_start = i + 1;
		}
	}

	int pipes[cmd_count - 1][2];
	for (int i = 0; i < cmd_count - 1; i++) {
		if (pipe(pipes[i]) == -1) {
			perror("pipe");
			for (int j = 0; j < cmd_count; j++) {
				free(commands[j]);
			}
			free(commands);
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

			execvp(commands[i][0], commands[i]);
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
}

void tile(void)
{
	update_struts();
	Client *head = workspaces[current_ws];
	int total = 0;

	for (Client *c = head; c; c = c->next) {
		if (c->mapped && !c->floating && !c->fullscreen) {
			total++;
		}
	}

	if (total == 1) {
		for (Client *c = head; c; c = c->next) {
			if (!c->floating && c->fullscreen) {
				return;
			}
		}
	}

	for (int m = 0; m < monsn; m++) {
		int mon_x = mons[m].x, mon_y = mons[m].y + mons[m].reserve_top;
		int mon_w = mons[m].w, mon_h = mons[m].h - mons[m].reserve_top - mons[m].reserve_bottom;

		Client *stackers[MAXCLIENTS];
		int N = 0;
		for (Client *c = head; c && N < MAXCLIENTS; c = c->next) {
			if (c->mapped && !c->floating && !c->fullscreen && c->mon == m) {
				stackers[N++] = c;
			}
		}

		if (N == 0) {
			continue;
		}

		int gx = user_config.gaps, gy = user_config.gaps;
		int tile_x = mon_x + gx, tile_y = mon_y + gy;
		int tile_w = MAX(1, mon_w - 2 * gx);
		int tile_h = MAX(1, mon_h - 2 * gy);
		float mf = CLAMP(user_config.master_width[m], MF_MIN, MF_MAX);
		int master_w = (N > 1) ? (int)(tile_w * mf) : tile_w;
		int stack_w = (N > 1) ? (tile_w - master_w - gx) : 0;

		{
			Client *c = stackers[0];
			int bw2 = 2 * user_config.border_width;
			XWindowChanges wc = {.x = tile_x,
			                     .y = tile_y,
			                     .width = MAX(1, master_w - bw2),
			                     .height = MAX(1, tile_h - bw2),
			                     .border_width = user_config.border_width};

			if (c->x != wc.x || c->y != wc.y || c->w != wc.width || c->h != wc.height) {
				XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
			}

			c->x = wc.x;
			c->y = wc.y;
			c->w = wc.width;
			c->h = wc.height;
		}

		if (N == 1) {
			update_borders();
			continue;
		}

		int bw2 = 2 * user_config.border_width;
		int num_stack = N - 1;
		int min_raw = bw2 + 1;
		int total_fixed_heights = 0, auto_count = 0;
		int heights_final[MAXCLIENTS] = {0};
		Bool is_fixed[MAXCLIENTS] = {0};

		for (int i = 1; i < N; i++) {
			if (stackers[i]->custom_stack_height > 0) {
				is_fixed[i] = True;
				total_fixed_heights += stackers[i]->custom_stack_height;
			}
			else {
				auto_count++;
			}
		}

		int total_vgaps = (num_stack - 1) * gy;
		int remaining = tile_h - total_fixed_heights - total_vgaps;

		if (auto_count > 0 && remaining >= auto_count * min_raw) {
			int auto_h = remaining / auto_count, used = 0, count = 0;
			for (int i = 1; i < N; i++) {
				if (!is_fixed[i]) {
					count++;
					heights_final[i] = (count < auto_count) ? auto_h : remaining - used;
					used += auto_h;
				}
				else {
					heights_final[i] = stackers[i]->custom_stack_height;
				}
			}
		}
		else {
			for (int i = 1; i < N; i++) {
				if (is_fixed[i]) {
					heights_final[i] = stackers[i]->custom_stack_height;
				}
				else {
					heights_final[i] = min_raw;
				}
			}
		}

		int total_height = total_vgaps;
		for (int i = 1; i < N; i++) {
			total_height += heights_final[i];
		}
		int overfill = total_height - tile_h;
		if (overfill > 0) {
			/* shrink from top down, excluding bottom */
			for (int i = 1; i < N - 1 && overfill > 0; i++) {
				int shrink = MIN(overfill, heights_final[i] - min_raw);
				heights_final[i] -= shrink;
				overfill -= shrink;
			}
		}

		/* if its not perfectly filled stretch bottom to absorb remainder */
		int actual_height = total_vgaps;
		for (int i = 1; i < N; i++) {
			actual_height += heights_final[i];
		}
		int shortfall = tile_h - actual_height;
		if (shortfall > 0) {
			heights_final[N - 1] += shortfall;
		}

		int sy = tile_y;
		for (int i = 1; i < N; i++) {
			Client *c = stackers[i];
			XWindowChanges wc = {.x = tile_x + master_w + gx,
			                     .y = sy,
			                     .width = MAX(1, stack_w - (2 * user_config.border_width)),
			                     .height = MAX(1, heights_final[i] - (2 * user_config.border_width)),
			                     .border_width = user_config.border_width};

			if (c->x != wc.x || c->y != wc.y || c->w != wc.width || c->h != wc.height) {
				XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
			}

			c->x = wc.x;
			c->y = wc.y;
			c->w = wc.width;
			c->h = wc.height;

			sy += heights_final[i] + gy;
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

			XConfigureWindow(
			    dpy, focused->win, CWX | CWY | CWWidth | CWHeight,
			    &(XWindowChanges){.x = focused->x, .y = focused->y, .width = focused->w, .height = focused->h});
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
		XRaiseWindow(dpy, focused->win);
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
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

	if (focused->floating) {
		focused->floating = False;
	}

	focused->fullscreen = !focused->fullscreen;

	if (focused->fullscreen) {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, focused->win, &wa);
		focused->orig_x = wa.x;
		focused->orig_y = wa.y;
		focused->orig_w = wa.width;
		focused->orig_h = wa.height;

		int m = focused->mon;
		int fs_x = mons[m].x;
		int fs_y = mons[m].y;
		int fs_w = mons[m].w;
		int fs_h = mons[m].h;

		XSetWindowBorderWidth(dpy, focused->win, 0);
		XMoveResizeWindow(dpy, focused->win, fs_x, fs_y, fs_w, fs_h);
		XRaiseWindow(dpy, focused->win);
	}
	else {
		XMoveResizeWindow(dpy, focused->win, focused->orig_x, focused->orig_y, focused->orig_w, focused->orig_h);
		XSetWindowBorderWidth(dpy, focused->win, user_config.border_width);

		if (!focused->floating) {
			focused->mon = get_monitor_for(focused);
		}
		tile();
		update_borders();
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

		/* link to new workspace */
		c->next = workspaces[current_ws];
		workspaces[current_ws] = c;

		c->ws = current_ws;
		c->mon = get_monitor_for(c);

		tile();
		update_borders();
		update_client_desktop_properties();
		update_net_client_list();
	}

	if (scratchpads[n].enabled) {
		XUnmapWindow(dpy, c->win);
		scratchpads[n].enabled = False;
		focus_prev();
		if (focused) {
			send_wm_take_focus(focused->win);
		}
		update_borders();
	}
	else {
		XMapWindow(dpy, c->win);
		XRaiseWindow(dpy, c->win);
		scratchpads[n].enabled = True;
		focused = c;
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		send_wm_take_focus(c->win);
		if (user_config.warp_cursor) {
			warp_cursor(focused);
		}
		tile();
		update_borders();
	}
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
		XSetInputFocus(dpy, swallower->win, RevertToPointerRoot, CurrentTime);
		XRaiseWindow(dpy, swallower->win);
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
		XChangeProperty(dpy, root, atom_net_active_window, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&w, 1);
	}
}

void update_client_desktop_properties(void)
{
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			long desktop = ws;
			XChangeProperty(dpy, c->win, XInternAtom(dpy, "_NET_WM_DESKTOP", False), XA_CARDINAL, 32, PropModeReplace,
			                (unsigned char *)&desktop, 1);
		}
	}
}

void update_monitors(void)
{
	XineramaScreenInfo *info;
	Monitor *old = mons;

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));

	for (int s = 0; s < ScreenCount(dpy); s++) {
		Window scr_root = RootWindow(dpy, s);
		XDefineCursor(dpy, scr_root, c_normal);
	}

	if (XineramaIsActive(dpy)) {
		info = XineramaQueryScreens(dpy, &monsn);
		mons = malloc(sizeof *mons * monsn);
		for (int i = 0; i < monsn; i++) {
			mons[i].x = info[i].x_org;
			mons[i].y = info[i].y_org;
			mons[i].w = info[i].width;
			mons[i].h = info[i].height;
		}
		XFree(info);
	}
	else {
		monsn = 1;
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
	Window wins[MAXCLIENTS];
	int n = 0;
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			wins[n++] = c->win;
		}
	}
	Atom prop = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	XChangeProperty(dpy, root, prop, XA_WINDOW, 32, PropModeReplace, (unsigned char *)wins, n);
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

Bool window_should_start_fullscreen(Window w)
{
	XClassHint ch;
	if (XGetClassHint(dpy, w, &ch)) {
		for (int i = 0; i < 256; i++) {
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

int xerr(Display *dpy, XErrorEvent *ee)
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
	(void)dpy;
	(void)ee;
}

void xev_case(XEvent *xev)
{
	if (xev->type >= 0 && xev->type < LASTEvent) {
		evtable[xev->type](xev);
	}
	else {
		printf("sxwm: invalid event type: %d\n", xev->type);
	}
}

int main(int ac, char **av)
{
	if (ac > 1) {
		if (strcmp(av[1], "-v") == 0 || strcmp(av[1], "--version") == 0) {
			printf("%s\n%s\n%s\n", SXWM_VERSION, SXWM_AUTHOR, SXWM_LICINFO);
			exit(0);
		}
		else if (strcmp(av[1], "-b") == 0 || strcmp(av[1], "--backup") == 0) {
			puts("sxwm: using backup keybinds");
			backup_binds = True;
		}
		else {
			puts("usege:\n");
			puts("\t[-v || --version]: See the version of sxwm\n");
			puts("\t[-b || --backup]: Use backup set of keybinds with sxwm\n");
			exit(0);
		}
	}
	setup();
	printf("sxwm: starting...\n");
	run();
	return 0;
}
