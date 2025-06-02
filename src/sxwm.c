/*
 *	  See LICENSE for more info
 *
 *	  simple xorg window manager
 *	  sxwm is a user-friendly, easily configurable yet powerful
 *	  tiling window manager inspired by window managers such as
 *	  DWM and i3.
 *
 *	  The userconfig is designed to be as user-friendly as
 *	  possible, and I hope it is easy to configure even without
 *	  knowledge of C or programming, although most people who
 *	  will use this will probably be programmers :)
 *
 *	  (C) Abhinav Prasai 2025
 */

#include <X11/X.h>
#include <err.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
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
void change_workspace(int ws);
int clean_mask(int mask);
/* void close_focused(void); */
/* void dec_gaps(void); */
/* void focus_next(void); */
/* void focus_prev(void); */
int get_monitor_for(Client *c);
void grab_keys(void);
void hdl_button(XEvent *xev);
void hdl_button_release(XEvent *xev);
void hdl_client_msg(XEvent *xev);
void hdl_config_ntf(XEvent *xev);
void hdl_config_req(XEvent *xev);
void hdl_dummy(XEvent *xev);
void hdl_destroy_ntf(XEvent *xev);
void hdl_enter(XEvent *xev);
void hdl_keypress(XEvent *xev);
void hdl_map_req(XEvent *xev);
void hdl_motion(XEvent *xev);
void hdl_root_property(XEvent *xev);
void hdl_unmap_ntf(XEvent *xev);
/* void inc_gaps(void); */
void init_defaults(void);
/* void move_master_next(void); */
/* void move_master_prev(void); */
void move_to_workspace(int ws);
void other_wm(void);
int other_wm_err(Display *dpy, XErrorEvent *ee);
/* long parse_col(const char *hex); */
/* void quit(void); */
/* void reload_config(void); */
/* void resize_master_add(void); */
/* void resize_master_sub(void); */
void run(void);
void scan_existing_windows(void);
void send_wm_take_focus(Window w);
void setup(void);
void setup_atoms(void);
Bool window_should_float(Window w);
void spawn(const char **argv);
void swap_clients(Client *a, Client *b);
void tile(void);
/* void toggle_floating(void); */
/* void toggle_floating_global(void); */
/* void toggle_fullscreen(void); */
void update_borders(void);
void update_monitors(void);
void update_net_client_list(void);
void update_struts(void);
void warp_cursor(Client *c);
int xerr(Display *dpy, XErrorEvent *ee);
void xev_case(XEvent *xev);
#include "config.h"

Atom atom_net_active_window;
Atom atom_net_current_desktop;
Atom atom_net_supported;
Atom atom_net_wm_state;
Atom atom_net_wm_state_fullscreen;
Atom atom_wm_window_type;
Atom atom_net_wm_window_type_dock;
Atom atom_net_workarea;
Atom atom_wm_delete;
Atom atom_wm_strut;
Atom atom_wm_strut_partial;
Atom atom_net_supporting_wm_check;
Atom atom_net_wm_name;
Atom atom_utf8_string;

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
int monsn = 0;
Bool global_floating = False;
Bool in_ws_switch = False;

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

	if (!workspaces[ws]) {
		workspaces[ws] = c;
	}
	else {
		Client *tail = workspaces[ws];
		while (tail->next)
			tail = tail->next;
		tail->next = c;
	}

	open_windows++;
	XSelectInput(dpy, w,
	             EnterWindowMask | LeaveWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);

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

	if (global_floating) {
		c->floating = True;
	}

	if (ws == current_ws && !focused) {
		focused = c;
	}

	XRaiseWindow(dpy, w);
	return c;
}

void change_workspace(int ws)
{
	if (ws >= NUM_WORKSPACES || ws == current_ws)
		return;

	in_ws_switch = True;
	XGrabServer(dpy);

	/* unmap those still marked mapped */
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped)
			XUnmapWindow(dpy, c->win);
	}

	current_ws = ws;

	/* map those still marked mapped */
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->mapped)
			XMapWindow(dpy, c->win);
	}

	tile();
	if (workspaces[current_ws]) {
		focused = workspaces[current_ws];
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
		if (user_config.warp_cursor) {
			warp_cursor(focused);
		}
	}

	long cd = current_ws;
	XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False), XA_CARDINAL, 32, PropModeReplace,
	                (unsigned char *)&cd, 1);

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

	Atom *protos;
	int n;
	if (XGetWMProtocols(dpy, focused->win, &protos, &n) && protos) {
		for (int i = 0; i < n; i++)
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

void focus_next(void)
{
	if (!focused || !workspaces[current_ws]) {
		return;
	}

	focused = (focused->next ? focused->next : workspaces[current_ws]);
	XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, focused->win);
	if (user_config.warp_cursor) {
		warp_cursor(focused);
	}
	update_borders();
}

void focus_prev(void)
{
	if (!focused || !workspaces[current_ws]) {
		return;
	}

	Client *p = workspaces[current_ws], *prev = NULL;
	while (p && p != focused) {
		prev = p;
		p = p->next;
	}

	if (!prev) {
		while (p->next)
			p = p->next;
		focused = p;
	}
	else {
		focused = prev;
	}

	XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, focused->win);
	if (user_config.warp_cursor) {
		warp_cursor(focused);
	}
	update_borders();
}

int get_monitor_for(Client *c)
{
	int cx = c->x + c->w / 2, cy = c->y + c->h / 2;
	for (int i = 0; i < monsn; i++) {
		if (cx >= (int)mons[i].x && cx < mons[i].x + mons[i].w && cy >= (int)mons[i].y && cy < mons[i].y + mons[i].h)
			return i;
	}
	return 0;
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

		if ((b->type == TYPE_CWKSP && b->mods != user_config.modkey) ||
		    (b->type == TYPE_MWKSP && b->mods != (user_config.modkey | ShiftMask)))
			continue;

		KeyCode kc = XKeysymToKeycode(dpy, b->keysym);
		if (!kc)
			continue;

		for (size_t g = 0; g < sizeof guards / sizeof *guards; g++)
			XGrabKey(dpy, kc, b->mods | guards[g], root, True, GrabModeAsync, GrabModeAsync);
	}
}

void hdl_button(XEvent *xev)
{
	XButtonEvent *e = &xev->xbutton;
	Window w = e->subwindow;
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
	if (xev->xclient.message_type == atom_net_wm_state) {
		long action = xev->xclient.data.l[0];
		Atom target = xev->xclient.data.l[1];
		if (target == atom_net_wm_state_fullscreen) {
			if (action == 1 || action == 2) {
				toggle_fullscreen();
			}
			else if (action == 0 && focused && focused->fullscreen) {
				toggle_fullscreen();
			}
		}
		return;
	}

	if (xev->xclient.message_type == XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False)) {
		Window target = xev->xclient.data.l[0];
		for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
			for (Client *c = workspaces[ws]; c; c = c->next) {
				if (c->win == target) {
					/* do NOT move to current ws */
					if (ws == current_ws) {
						focused = c;
						send_wm_take_focus(c->win);
						XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
						XRaiseWindow(dpy, c->win);
						update_borders();
					}
					return;
				}
			}
		}
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

	for (int ws = 0; ws < NUM_WORKSPACES && !c; ws++)
		for (c = workspaces[ws]; c; c = c->next)
			if (c->win == e->window) {
				break;
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

void hdl_enter(XEvent *xev)
{
	Window w = xev->xcrossing.window;

	Client *head = workspaces[current_ws];
	for (Client *c = head; c; c = c->next) {
		if (c->win == w) {
			focused = c;
			XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
			update_borders();
			break;
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
					if (b->action.fn)
						b->action.fn();
					break;
				case TYPE_CWKSP:
					change_workspace(b->action.ws);
					update_net_client_list();
					break;
				case TYPE_MWKSP:
					move_to_workspace(b->action.ws);
					update_net_client_list();
					break;
			}
			return;
		}
	}
}

void swap_clients(Client *a, Client *b)
{
	if (!a || !b || a == b) {
		return;
	}

	Client **head = &workspaces[current_ws];
	Client **pa = head, **pb = head;

	while (*pa && *pa != a)
		pa = &(*pa)->next;
	while (*pb && *pb != b)
		pb = &(*pb)->next;

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

	Client *c = add_client(w, current_ws);
	if (!c)
		return;

	Window tr;
	if (!should_float && XGetTransientForHint(dpy, w, &tr))
		should_float = True;
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

	/* map & borders */
	update_net_client_list();
	if (!global_floating && !c->floating)
		tile();
	else if (c->floating)
		XRaiseWindow(dpy, w);

	if (user_config.new_win_focus) {
		focused = c;
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		send_wm_take_focus(c->win);
		if (user_config.warp_cursor) {
			warp_cursor(c);
		}
	}

	XMapWindow(dpy, w);
	for (Client *c = workspaces[current_ws]; c; c = c->next)
		if (c->win == w)
			c->mapped = True;

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

		if (UDIST(nx, 0) <= user_config.snap_distance) {
			nx = 0;
		}
		else if (UDIST(nx + outer_w, scr_width) <= user_config.snap_distance) {
			nx = scr_width - outer_w;
		}

		if (UDIST(ny, 0) <= user_config.snap_distance) {
			ny = 0;
		}
		else if (UDIST(ny + outer_h, scr_height) <= user_config.snap_distance) {
			ny = scr_height - outer_h;
		}

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
		drag_client->w = nw < 20 ? 20 : nw;
		drag_client->h = nh < 20 ? 20 : nh;
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
	reserve_left = reserve_right = reserve_top = reserve_bottom = 0;

	Window root_ret, parent_ret, *children;
	unsigned int nchildren;
	if (!XQueryTree(dpy, root, &root_ret, &parent_ret, &children, &nchildren))
		return;

	for (unsigned int i = 0; i < nchildren; i++) {
		Window w = children[i];

		Atom actual_type;
		int actual_format;
		unsigned long nitems, bytes_after;
		Atom *types = NULL;

		if (XGetWindowProperty(dpy, w, atom_wm_window_type, 0, 4, False, XA_ATOM, &actual_type, &actual_format, &nitems,
		                       &bytes_after, (unsigned char **)&types) != Success ||
		    !types)
			continue;

		Bool is_dock = False;
		for (unsigned long j = 0; j < nitems; j++) {
			if (types[j] == atom_net_wm_window_type_dock) {
				is_dock = True;
				break;
			}
		}
		XFree(types);
		if (!is_dock)
			continue;

		long *str = NULL;
		Atom actual;
		int sfmt;
		unsigned long len, rem;
		if (XGetWindowProperty(dpy, w, atom_wm_strut_partial, 0, 12, False, XA_CARDINAL, &actual, &sfmt, &len, &rem,
		                       (unsigned char **)&str) == Success &&
		    str && len >= 4) {
			reserve_left = MAX(reserve_left, str[0]);
			reserve_right = MAX(reserve_right, str[1]);
			reserve_top = MAX(reserve_top, str[2]);
			reserve_bottom = MAX(reserve_bottom, str[3]);
			XFree(str);
		}
		else if (XGetWindowProperty(dpy, w, atom_wm_strut, 0, 4, False, XA_CARDINAL, &actual, &sfmt, &len, &rem,
		                            (unsigned char **)&str) == Success &&
		         str && len == 4) {
			reserve_left = MAX(reserve_left, str[0]);
			reserve_right = MAX(reserve_right, str[1]);
			reserve_top = MAX(reserve_top, str[2]);
			reserve_bottom = MAX(reserve_bottom, str[3]);
			XFree(str);
		}
	}
	XFree(children);
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
	for (int i = 0; i < MAX_MONITORS; i++)
		default_config.master_width[i] = 50 / 100.0f;

	default_config.motion_throttle = 60;
	default_config.resize_master_amt = 5;
	default_config.snap_distance = 5;
	default_config.bindsn = 0;
	default_config.new_win_focus = True;
	default_config.warp_cursor = True;

	for (unsigned long i = 0; i < LENGTH(binds); i++) {
		default_config.binds[i].mods = binds[i].mods;
		default_config.binds[i].keysym = binds[i].keysym;
		default_config.binds[i].action.cmd = binds[i].action.cmd;
		default_config.binds[i].type = binds[i].type;
		default_config.bindsn++;
	}

	user_config = default_config;
}

void move_master_next(void)
{
	if (!workspaces[current_ws] || !workspaces[current_ws]->next) {
		return;
	}
	Client *first = workspaces[current_ws];
	workspaces[current_ws] = first->next;
	first->next = NULL;

	Client *tail = workspaces[current_ws];
	while (tail->next) {
		tail = tail->next;
	}
	tail->next = first;

	tile();
	update_borders();
}

void move_master_prev(void)
{
	if (!workspaces[current_ws] || !workspaces[current_ws]->next) {
		return;
	}
	Client *prev = NULL, *cur = workspaces[current_ws];
	while (cur->next) {
		prev = cur;
		cur = cur->next;
	}
	prev->next = NULL;
	cur->next = workspaces[current_ws];
	workspaces[current_ws] = cur;
	tile();
	update_borders();
}

void move_to_workspace(int ws)
{
	if (!focused || ws >= NUM_WORKSPACES || ws == current_ws) {
		return;
	}

	if (focused->fullscreen) {
		focused->fullscreen = False;
		XMoveResizeWindow(dpy, focused->win, focused->orig_x, focused->orig_y, focused->orig_w, focused->orig_h);
		XSetWindowBorderWidth(dpy, focused->win, user_config.border_width);
	}

	XUnmapWindow(dpy, focused->win);
	/* remove from current list */
	Client **pp = &workspaces[current_ws];
	while (*pp && *pp != focused)
		pp = &(*pp)->next;
	if (*pp) {
		*pp = focused->next;
	}

	/* push to target list */
	focused->next = workspaces[ws];
	workspaces[ws] = focused;

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

	/* return col.pixel |= 0xff << 24; */
	/* This is a fix for picom making the borders transparent. DANGEROUS */
	return col.pixel;
}

void quit(void)
{
	for (int ws = 0; ws < NUM_WORKSPACES; ws++) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			XUnmapWindow(dpy, c->win);
			XKillClient(dpy, c->win);
		}
	}
	XSync(dpy, False);
	XCloseDisplay(dpy);
	XFreeCursor(dpy, c_move);
	XFreeCursor(dpy, c_normal);
	XFreeCursor(dpy, c_resize);
	errx(0, "quitting...");
}

void reload_config(void)
{
	puts("sxwm: reloading config...");
	memset(&user_config, 0, sizeof(user_config));

	for (int i = 0; i < user_config.bindsn; i++) {
		free(user_config.binds[i].action.cmd);
		user_config.binds[i].action.cmd = NULL;

		user_config.binds[i].action.fn = NULL;
		user_config.binds[i].type = -1;
		user_config.binds[i].keysym = 0;
		user_config.binds[i].mods = 0;
	}

	init_defaults();
	if (parser(&user_config)) {
		fprintf(stderr, "sxrc: error parsing config file\n");
		init_defaults();
	}
	grab_keys();
	XUngrabButton(dpy, AnyButton, AnyModifier, root);
	XGrabButton(dpy, Button1, user_config.modkey, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button1, user_config.modkey | ShiftMask, root, True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button3, user_config.modkey, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);
	XSync(dpy, False);

	tile();
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

void run(void)
{
	XEvent xev;
	for (;;) {
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
		fprintf(stderr, "sxrc: error parsing config file\n");
		init_defaults();
	}
	grab_keys();

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
	evtable[EnterNotify] = hdl_enter;
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
	atom_net_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	atom_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	atom_net_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	atom_net_supporting_wm_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	atom_net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
	atom_utf8_string = XInternAtom(dpy, "UTF8_STRING", False);

	Atom support_list[] = {
	    atom_net_current_desktop,
	    atom_net_active_window,
	    atom_net_supported,
	    atom_net_wm_state,
	    atom_net_wm_state_fullscreen,
	    atom_wm_window_type,
	    atom_net_wm_window_type_dock,
	    atom_net_workarea,
	    atom_wm_strut,
	    atom_wm_strut_partial,
	    atom_wm_delete,
	    atom_net_supporting_wm_check,
	    atom_net_wm_name,
	    atom_utf8_string,
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

void spawn(const char **argv)
{
	int pipe_idx = -1;
	for (int i = 0; argv[i]; i++) {
		if (strcmp(argv[i], "|") == 0) {
			pipe_idx = i;
			break;
		}
	}

	if (pipe_idx < 0) {
		if (fork() == 0) {
			close(ConnectionNumber(dpy));
			setsid();
			execvp(argv[0], (char *const *)argv);
			fprintf(stderr, "sxwm: execvp '%s' failed\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	else {
		((char **)argv)[pipe_idx] = NULL;
		const char **left = argv;
		const char **right = argv + pipe_idx + 1;
		int fd[2];
		Bool x = pipe(fd);
		(void)x;

		pid_t pid1 = fork();
		if (pid1 == 0) {
			dup2(fd[1], STDOUT_FILENO);
			close(fd[0]);
			close(fd[1]);
			execvp(left[0], (char *const *)left);
			perror("spawn left");
			exit(EXIT_FAILURE);
		}

		pid_t pid2 = fork();
		if (pid2 == 0) {
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
			close(fd[1]);
			execvp(right[0], (char *const *)right);
			perror("spawn right");
			exit(EXIT_FAILURE);
		}

		close(fd[0]);
		close(fd[1]);
		waitpid(pid1, NULL, 0);
		waitpid(pid2, NULL, 0);
	}
}

void tile(void)
{
	update_struts(); /* fills reserve_top, reserve_bottom */

	Client *head = workspaces[current_ws];

	int total = 0;
	for (Client *c = head; c; c = c->next) {
		if (!c->mapped || c->floating || c->fullscreen)
			continue;
		total++;
	}

	if (total == 1) {
		for (Client *c = head; c; c = c->next) {
			if (!c->floating && c->fullscreen)
				return;
		}
	}

	for (int m = 0; m < monsn; m++) {
		int mon_x = mons[m].x;
		int mon_y = mons[m].y + reserve_top;
		int mon_w = mons[m].w;
		int mon_h = mons[m].h - reserve_top - reserve_bottom;

		int cnt = 0;
		for (Client *c = head; c; c = c->next) {
			if (!c->mapped || c->floating || c->fullscreen || c->mon != m)
				continue;
			cnt++;
		}
		if (!cnt)
			continue;

		int gx = user_config.gaps;
		int gy = user_config.gaps;
		int tile_x = mon_x + gx;
		int tile_y = mon_y + gy;
		int tile_w = mon_w - 2 * gx;
		int tile_h = mon_h - 2 * gy;
		if (tile_w < 1)
			tile_w = 1;
		if (tile_h < 1)
			tile_h = 1;

		/* master‐stack split */
		float mf = user_config.master_width[m];
		if (mf < MF_MIN)
			mf = MF_MIN;
		if (mf > MF_MAX)
			mf = MF_MAX;
		int master_w = (cnt > 1) ? (int)(tile_w * mf) : tile_w;
		int stack_w = tile_w - master_w - ((cnt > 1) ? gx : 0);
		int stack_h = (cnt > 1) ? (tile_h - (cnt - 1) * gy) / (cnt - 1) : 0;

		int i = 0, sy = tile_y;
		for (Client *c = head; c; c = c->next) {
			if (!c->mapped || c->floating || c->fullscreen || c->mon != m)
				continue;

			XWindowChanges wc = {.border_width = user_config.border_width};
			int bw2 = 2 * user_config.border_width;

			if (i == 0) {
				/* master */
				wc.x = tile_x;
				wc.y = tile_y;
				wc.width = MAX(1, master_w - bw2);
				wc.height = MAX(1, tile_h - bw2);
			}
			else {
				/* stack */
				wc.x = tile_x + master_w + gx;
				wc.y = sy;
				int h = (i == cnt - 1) ? (tile_y + tile_h - sy) : stack_h;
				wc.width = MAX(1, stack_w - bw2);
				wc.height = MAX(1, h - bw2);
				sy += h + gy;
			}

			XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
			c->x = wc.x;
			c->y = wc.y;
			c->w = wc.width;
			c->h = wc.height;
			i++;
		}
	}

	update_borders();
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
			printf("%s\n%s\n%s", SXWM_VERSION, SXWM_AUTHOR, SXWM_LICINFO);
			exit(0);
		}
		else {
			printf("usage:\n[-v || --version]: See the version of sxwm");
			exit(0);
		}
	}
	setup();
	printf("sxwm: starting...\n");
	run();
	return 0;
}
