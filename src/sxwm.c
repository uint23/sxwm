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

#include <X11/cursorfont.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xinerama.h>

#include "defs.h"
#include "parser.h"

void add_client(Window w);
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
void spawn(const char **cmd);
void swap_clients(Client *a, Client *b);
void tile(void);
/* void toggle_floating(void); */
/* void toggle_floating_global(void); */
/* void toggle_fullscreen(void); */
void update_borders(void);
void update_monitors(void);
void update_net_client_list(void);
int xerr(Display *dpy, XErrorEvent *ee);
void xev_case(XEvent *xev);
INIT_WORKSPACE
#include "config.h"

Atom atom_wm_delete;
Atom atom_wm_strut_partial;
Atom atom_wm_window_type;
Atom atom_net_supported;
Atom atom_net_wm_state;
Atom atom_net_current_desktop;
Atom atom_net_wm_state_fullscreen;
Atom atom_net_wm_window_type_dock;
Atom atom_net_workarea;

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
Monitor *mons = NULL;
int monsn = 0;
Bool global_floating = False;

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

void add_client(Window w)
{
	Client *c = malloc(sizeof(Client));
	if (!c) {
		fprintf(stderr, "sxwm: could not alloc memory for client\n");
		return;
	}

	if (workspaces[current_ws] == NULL) {
		workspaces[current_ws] = c;
	}
	else {
		Client *tail = workspaces[current_ws];
		while (tail->next)
			tail = tail->next;
		tail->next = c;
	}
	c->win = w;
	c->next = NULL;

	if (!focused) {
		focused = c;
	}

	++open_windows;
	XSelectInput(dpy, w,
	             EnterWindowMask | LeaveWindowMask | FocusChangeMask | PropertyChangeMask |
	                 StructureNotifyMask);
	Atom protos[] = {atom_wm_delete};
	XSetWMProtocols(dpy, w, protos, 1);

	XWindowAttributes wa;
	XGetWindowAttributes(dpy, w, &wa);
	c->x = wa.x;
	c->y = wa.y;
	c->w = wa.width;
	c->h = wa.height;

	if (focused) {
		c->mon = focused->mon;
	}
	else {
		c->mon = get_monitor_for(c);
	}

	c->mon = get_monitor_for(c);
	c->fixed = False;
	c->floating = False;
	c->fullscreen = False;

	if (global_floating) {
		c->floating = True;
		XSetWindowBorder(dpy, c->win, user_config.border_foc_col);
		XSetWindowBorderWidth(dpy, c->win, user_config.border_width);
	}
	tile();
	update_borders();
	XRaiseWindow(dpy, w);
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
		for (int i = 0; i < n; ++i)
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
		--user_config.gaps;
		tile();
		update_borders();
	}
}

void focus_next(void)
{
	if (!focused) {
		return;
	}

	focused = (focused->next ? focused->next : workspaces[current_ws]);
	XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, focused->win);
	update_borders();
}

void focus_prev(void)
{
	if (!focused) {
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
	update_borders();
}

int get_monitor_for(Client *c)
{
	int cx = c->x + c->w / 2, cy = c->y + c->h / 2;
	for (int i = 0; i < monsn; ++i) {
		if (cx >= (int)mons[i].x && cx < mons[i].x + mons[i].w && cy >= (int)mons[i].y &&
		    cy < mons[i].y + mons[i].h)
			return i;
	}
	return 0;
}

void grab_keys(void)
{
	KeyCode keycode;
	int modifiers[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for (int i = 0; i < user_config.bindsn; ++i) {
		if ((keycode = XKeysymToKeycode(dpy, user_config.binds[i].keysym))) {
			for (unsigned int j = 0; j < LENGTH(modifiers); ++j) {
				XGrabKey(dpy, keycode, user_config.binds[i].mods | modifiers[j], root, True,
				         GrabModeAsync, GrabModeAsync);
			}
		}
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
		if ((e->state & MOD) && (e->state & ShiftMask) && e->button == Button1 && !c->floating) {
			drag_client = c;
			drag_start_x = e->x_root;
			drag_start_y = e->y_root;
			drag_orig_x = c->x;
			drag_orig_y = c->y;
			drag_orig_w = c->w;
			drag_orig_h = c->h;
			drag_mode = DRAG_SWAP;
			XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
			             GrabModeAsync, None, c_move, CurrentTime);
			focused = c;
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			XSetWindowBorder(dpy, c->win, user_config.border_swap_col);
			XRaiseWindow(dpy, c->win);
			return;
		}

		if ((e->state & MOD) && (e->button == Button1 || e->button == Button3) && !c->floating) {
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
		XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
		             GrabModeAsync, None, cur, CurrentTime);

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
			                 (swap_target == focused ? user_config.border_foc_col
			                                         : user_config.border_ufoc_col));
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

	for (int ws = 0; ws < NUM_WORKSPACES && !c; ++ws)
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

	Client *prev = NULL, *c = workspaces[current_ws];
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
				focused = NULL;
			}
		}

		if (!prev) {
			workspaces[current_ws] = c->next;
		}
		else {
			prev->next = c->next;
		}

		free(c);
		update_net_client_list();
		--open_windows;
	}

	tile();
	update_borders();

	if (focused) {
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
		XRaiseWindow(dpy, focused->win);
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
	KeySym keysym = XLookupKeysym(&xev->xkey, 0);
	unsigned int mods = clean_mask(xev->xkey.state);

	for (int i = 0; i < user_config.bindsn; ++i) {
		if (keysym == user_config.binds[i].keysym &&
		    mods == (unsigned int)clean_mask(user_config.binds[i].mods)) {
			if (user_config.binds[i].is_func) {
				user_config.binds[i].action.fn();
			}
			else {
				spawn(user_config.binds[i].action.cmd);
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
	XConfigureRequestEvent *cr = &xev->xconfigurerequest;
	XWindowAttributes wa;
	XGetWindowAttributes(dpy, cr->window, &wa);

	if (wa.override_redirect) {
		XMapWindow(dpy, cr->window);
		return;
	}

	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	Atom *types = NULL;

	if (XGetWindowProperty(dpy, cr->window, atom_wm_window_type, 0, 1, False, XA_ATOM, &type,
	                       &format, &nitems, &bytes_after, (unsigned char **)&types) == Success &&
	    types) {
		if (nitems > 0 && types[0] == atom_net_wm_window_type_dock) {
			XFree(types);

			XMapWindow(dpy, cr->window);
			long *strut = NULL;
			if (XGetWindowProperty(dpy, cr->window, atom_wm_strut_partial, 0, 12, False,
			                       XA_CARDINAL, &type, &format, &nitems, &bytes_after,
			                       (unsigned char **)&strut) == Success &&
			    strut) {
				if (nitems >= 4) {
					reserve_left = strut[0];
					reserve_right = strut[1];
					reserve_top = strut[2];
					reserve_bottom = strut[3];
				}
				XFree(strut);
			}

			long workarea[4] = {reserve_left, reserve_top, scr_width - reserve_left - reserve_right,
			                    scr_height - reserve_top - reserve_bottom};
			XChangeProperty(dpy, root, atom_net_workarea, XA_CARDINAL, 32, PropModeReplace,
			                (unsigned char *)workarea, 4);
			return;
		}
		XFree(types);
	}

	if (open_windows == MAXCLIENTS) {
		return;
	}

	add_client(cr->window);
	Client *c = workspaces[current_ws];
	Window trans;

	if (XGetTransientForHint(dpy, c->win, &trans)) {
		c->floating = True;
	}

	XSizeHints sh;
	long supplied;
	if (XGetWMNormalHints(dpy, c->win, &sh, &supplied) && (sh.flags & PMinSize) &&
	    (sh.flags & PMaxSize) && sh.min_width == sh.max_width && sh.min_height == sh.max_height) {
		c->floating = True;
		c->fixed = True;

		XSetWindowBorderWidth(dpy, c->win, user_config.border_width);
		XSetWindowBorder(dpy, c->win,
		                 (c == focused ? user_config.border_foc_col : user_config.border_ufoc_col));
	}

	if (c->floating && !c->fullscreen) {
		int w = (c->w < 64 ? 640 : c->w);
		int h = (c->h < 64 ? 480 : c->h);
		int x = (scr_width - w) / 2;
		int y = (scr_height - h) / 2;

		c->x = x;
		c->y = y;
		c->w = w;
		c->h = h;

		XMoveResizeWindow(dpy, c->win, x, y, w, h);
	}

	{
		Window transient;
		if (XGetTransientForHint(dpy, cr->window, &transient)) {
			Client *c = workspaces[current_ws];
			c->floating = True;
			XSetWindowBorderWidth(dpy, c->win, user_config.border_width);
			XSetWindowBorder(
			    dpy, c->win,
			    (c == focused ? user_config.border_foc_col : user_config.border_ufoc_col));

			if (c->w < 64 || c->h < 64) {
				int w = (c->w < 64 ? 640 : c->w);
				int h = (c->h < 64 ? 480 : c->h);
				int x = (scr_width - w) / 2;
				int y = (scr_height - h) / 2;
				XMoveResizeWindow(dpy, c->win, x, y, w, h);
			}
		}
	}

	XMapWindow(dpy, cr->window);
	update_net_client_list();

	if (!global_floating) {
		tile();
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
				XSetWindowBorder(dpy, last_swap_target->win,
				                 (last_swap_target == focused ? user_config.border_foc_col
				                                              : user_config.border_ufoc_col));
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
	if (e->atom != atom_net_current_desktop) {
		return;
	}

	long *val = NULL;
	Atom actual;
	int fmt;
	unsigned long n, after;
	if (XGetWindowProperty(dpy, root, atom_net_current_desktop, 0, 1, False, XA_CARDINAL, &actual,
	                       &fmt, &n, &after, (unsigned char **)&val) == Success &&
	    val) {
		change_workspace((int)val[0]);
		XFree(val);
	}
}

void inc_gaps(void)
{
	++user_config.gaps;
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
	default_config.master_width = 50 / 100.0f;
	default_config.motion_throttle = 60;
	default_config.resize_master_amt = 5;
	default_config.snap_distance = 5;
	default_config.bindsn = 0;

	for (unsigned long i = 0; i < LENGTH(binds); ++i) {
		default_config.binds[i].mods = binds[i].mods;
		default_config.binds[i].keysym = binds[i].keysym;
		default_config.binds[i].action.cmd = binds[i].action.cmd;
		default_config.binds[i].is_func = binds[i].is_func;
		++default_config.bindsn;
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
		XMoveResizeWindow(dpy, focused->win, focused->orig_x, focused->orig_y, focused->orig_w,
		                  focused->orig_h);
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
	XChangeWindowAttributes(dpy, root, CWEventMask,
	                        &(XSetWindowAttributes){.event_mask = SubstructureRedirectMask});
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

	return col.pixel;
}

void quit(void)
{
	for (int ws = 0; ws < NUM_WORKSPACES; ++ws) {
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
	memset(user_config.binds, 0, sizeof(user_config.binds));
	init_defaults();
	if (parser(&user_config)) {
		fprintf(stderr, "sxrc: error parsing config file\n");
		init_defaults();
	}
	grab_keys();
	tile();
	update_borders();
}

void resize_master_add(void)
{
	if (user_config.master_width < MF_MAX - 0.001f) {
		user_config.master_width += ((float)user_config.resize_master_amt / 100);
	}
	tile();
	update_borders();
}

void resize_master_sub(void)
{
	if (user_config.master_width > MF_MIN + 0.001f) {
		user_config.master_width -= ((float)user_config.resize_master_amt / 100);
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
		for (unsigned int i = 0; i < nchildren; ++i) {
			XWindowAttributes wa;
			if (!XGetWindowAttributes(dpy, children[i], &wa) || wa.override_redirect ||
			    wa.map_state != IsViewable) {
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
		for (int i = 0; i < n; ++i) {
			if (protos[i] == wm_take_focus) {
				XEvent ev = {.xclient = {.type = ClientMessage,
				                         .window = w,
				                         .message_type = wm_protocols,
				                         .format = 32}};
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

	c_normal = XCreateFontCursor(dpy, XC_left_ptr);
	c_move = XCreateFontCursor(dpy, XC_fleur);
	c_resize = XCreateFontCursor(dpy, XC_bottom_right_corner);
	XDefineCursor(dpy, root, c_normal);

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));
	update_monitors();

	XSelectInput(dpy, root,
	             StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask |
	                 KeyPressMask | PropertyChangeMask);

	XGrabButton(dpy, Button1, MOD, root, True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
	            GrabModeAsync, None, None);
	XGrabButton(dpy, Button1, MOD | ShiftMask, root, True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
	            GrabModeAsync, None, None);
	XGrabButton(dpy, Button3, MOD, root, True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
	            GrabModeAsync, None, None);
	XSync(dpy, False);

	for (int i = 0; i < LASTEvent; ++i) {
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
	scan_existing_windows();
}

void setup_atoms(void)
{
	/* bar atoms */
	atom_net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
	atom_wm_strut_partial = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
	atom_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	atom_net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	atom_net_workarea = XInternAtom(dpy, "_NET_WORKAREA", False);

	Atom support_list[] = {
	    XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False),
	    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False),
	    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False),
	    XInternAtom(dpy, "_NET_WORKAREA", False),
	};

	XChangeProperty(dpy, root, atom_net_supported, XA_ATOM, 32, PropModeReplace,
	                (unsigned char *)support_list, sizeof(support_list) / sizeof(Atom));

	/* workspace atoms */
	Atom a_num = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	Atom a_names = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);

	long num = NUM_WORKSPACES;
	XChangeProperty(dpy, root, a_num, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&num, 1);

	const char names[] = WORKSPACE_NAMES;
	int names_len = sizeof(names);

	XChangeProperty(dpy, root, a_names, XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace,
	                (unsigned char *)names, names_len);

	long initial = current_ws;
	XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False), XA_CARDINAL, 32,
	                PropModeReplace, (unsigned char *)&initial, 1);
	/* fullscreen atoms */
	atom_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	atom_net_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

	/* delete atoms */
	atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	/* current desktop atoms */
	atom_net_current_desktop = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
}

void spawn(const char **cmd)
{
	pid_t pid;

	if (!cmd || !cmd[0]) {
		return;
	}

	pid = fork();
	if (pid < 0) {
		errx(1, "sxwm: fork failed");
	}
	else if (pid == 0) {
		setsid();
		execvp(cmd[0], (char *const *)cmd);
		errx(1, "sxwm: execvp '%s' failed", cmd[0]);
	}
}

void tile(void)
{
	int total_windows = 0;
	Client *head = workspaces[current_ws];
	for (Client *c = head; c; c = c->next) {
		if (c->fullscreen) {
			return;
		}
		if (!c->floating) {
			++total_windows;
		}
		c->mon = get_monitor_for(c);
	}

	if (total_windows == 0) {
		return;
	}

	for (int m = 0; m < monsn; ++m) {
		Client *c;
		int count = 0;
		for (c = workspaces[current_ws]; c; c = c->next) {
			if (!c->floating && c->mon == m) {
				++count;
			}
		}

		if (count == 0) {
			continue;
		}

		int master = 1;
		int stack = count - master;

		/* reserved space */
		int left = mons[m].x + reserve_left + user_config.gaps;
		int top = mons[m].y + reserve_top + user_config.gaps;
		int width = mons[m].w - reserve_left - reserve_right - 2 * user_config.gaps;
		int height = mons[m].h - reserve_top - reserve_bottom - 2 * user_config.gaps;

		int master_width = (stack > 0) ? width * user_config.master_width : width;
		int stack_width = (stack > 0) ? (width - master_width - user_config.gaps) : 0;
		int stack_row_height = (stack > 0) ? (height - (stack - 1) * user_config.gaps) / stack : 0;

		int i = 0;
		int stack_x = left + master_width + user_config.gaps;
		focused = workspaces[current_ws];
		for (c = workspaces[current_ws]; c; c = c->next) {
			if (c->floating || c->mon != m) {
				continue;
			}

			XWindowChanges wc = {.border_width = user_config.border_width};
			if (i == 0) {
				/* master */
				wc.x = left;
				wc.y = top;
				wc.width = master_width - 2 * user_config.border_width;
				wc.height = height - 2 * user_config.border_width;
			}
			else {
				/* stack */
				int y = top + (i - 1) * (stack_row_height + user_config.gaps);
				int h = (i == count - 1)
				            ? (height - (stack_row_height + user_config.gaps) * (stack - 1))
				            : stack_row_height;

				wc.x = stack_x;
				wc.y = y;
				wc.width = stack_width - 2 * user_config.border_width;
				wc.height = h - 2 * user_config.border_width;
			}

			XSetWindowBorder(dpy, c->win,
			                 (i == 0 ? user_config.border_foc_col : user_config.border_ufoc_col));

			XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);

			++i;
		}
	}
}

void toggle_floating(void)
{
	if (!focused) {
		return;
	}

	focused->floating = !focused->floating;
	if (focused->fullscreen) {
		focused->fullscreen = False;
		tile();
		XSetWindowBorderWidth(dpy, focused->win, user_config.border_width);
	}

	if (focused->floating) {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, focused->win, &wa);
		focused->x = wa.x;
		focused->y = wa.y;
		focused->w = wa.width;
		focused->h = wa.height;

		XConfigureWindow(
		    dpy, focused->win, CWX | CWY | CWWidth | CWHeight,
		    &(XWindowChanges){
		        .x = focused->x, .y = focused->y, .width = focused->w, .height = focused->h});
	}

	tile();
	update_borders();

	/* floating windows are on top */
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

			XConfigureWindow(
			    dpy, c->win, CWX | CWY | CWWidth | CWHeight,
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
		XMoveResizeWindow(dpy, focused->win, focused->orig_x, focused->orig_y, focused->orig_w,
		                  focused->orig_h);
		XSetWindowBorderWidth(dpy, focused->win, user_config.border_width);
		tile();
		update_borders();
	}
}

void update_borders(void)
{
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		XSetWindowBorder(dpy, c->win,
		                 (c == focused ? user_config.border_foc_col : user_config.border_ufoc_col));
	}
}

void update_monitors(void)
{
	XineramaScreenInfo *info;
	Monitor *old = mons;

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));

	for (int s = 0; s < ScreenCount(dpy); ++s) {
		Window scr_root = RootWindow(dpy, s);
		XDefineCursor(dpy, scr_root, c_normal);
	}

	if (XineramaIsActive(dpy)) {
		info = XineramaQueryScreens(dpy, &monsn);
		mons = malloc(sizeof *mons * monsn);
		for (int i = 0; i < monsn; ++i) {
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
	for (int ws = 0; ws < NUM_WORKSPACES; ++ws) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			wins[n++] = c->win; /* has to be n++ or well get an off by one error i think */
		}
	}
	Atom prop = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	XChangeProperty(dpy, root, prop, XA_WINDOW, 32, PropModeReplace, (unsigned char *)wins, n);
}

void change_workspace(int ws)
{
	if (ws >= NUM_WORKSPACES || ws == current_ws) {
		return;
	}

	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		XUnmapWindow(dpy, c->win);
	}

	current_ws = ws;

	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		XMapWindow(dpy, c->win);
	}

	tile();
	if (workspaces[current_ws]) {
		focused = workspaces[current_ws];
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	}

	long cd = current_ws;
	XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False), XA_CARDINAL, 32,
	                PropModeReplace, (unsigned char *)&cd, 1);
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

	for (size_t i = 0; i < sizeof(ignore) / sizeof(ignore[0]); ++i) {
		if ((ignore[i].req == 0 || ignore[i].req == ee->request_code) &&
		    (ignore[i].code == ee->error_code)) {
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
		if (strcmp(av[1], "-v") == 0 || strcmp(av[1], "--version") == 0)
			errx(0, "%s\n%s\n%s", SXWM_VERSION, SXWM_AUTHOR, SXWM_LICINFO);
		else
			errx(0, "usage:\n[-v || --version]: See the version of sxwm");
	}
	setup();
	run();
	return 0;
}