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

#include "defs.h"

static void add_client(Window w);
static void change_workspace(uint ws);
static uint clean_mask(uint mask);
static void close_focused(void);
static void dec_gaps(void);
static void focus_next(void);
static void focus_prev(void);
static void grab_keys(void);
static void hdl_button(XEvent *xev);
static void hdl_button_release(XEvent *xev);
static void hdl_client_msg(XEvent *xev);
static void hdl_config_req(XEvent *xev);
static void hdl_dummy(XEvent *xev);
static void hdl_destroy_ntf(XEvent *xev);
static void hdl_enter(XEvent *xev);
static void hdl_keypress(XEvent *xev);
static void hdl_map_req(XEvent *xev);
static void hdl_motion(XEvent *xev);
static void inc_gaps(void);
static void move_master_next(void);
static void move_master_prev(void);
static void move_to_workspace(uint ws);
static void other_wm(void);
static int other_wm_err(Display *dpy, XErrorEvent *ee);
static ulong parse_col(const char *hex);
static void quit(void);
static void resize_master_add(void);
static void resize_master_sub(void);
static void run(void);
static void scan_existing_windows(void);
static void setup(void);
static void setup_atoms(void);
static void spawn(const char **cmd);
static void tile(void);
static void toggle_floating(void);
static void toggle_floating_global(void);
static void toggle_fullscreen(void);
static void update_borders(void);
static void update_net_client_list(void);
static int xerr(Display *dpy, XErrorEvent *ee);
static void xev_case(XEvent *xev);
#include "config.h"

static Atom atom_wm_delete;
static Atom atom_wm_strut_partial;
static Atom atom_wm_window_type;
static Atom atom_net_supported;
static Atom atom_net_wm_state;
static Atom atom_net_wm_state_fullscreen;
static Atom atom_net_wm_window_type_dock;
static Atom atom_net_workarea;

static Cursor c_normal, c_move, c_resize;
static Client *workspaces[NUM_WORKSPACES] = { NULL };
static uint current_ws = 0;
static Client *drag_client = NULL;
static Client *focused = NULL;
static EventHandler evtable[LASTEvent];
static Display *dpy;
static Window root;
static Bool global_floating = False;

static ulong last_motion_time = 0;
static ulong border_foc_col;
static ulong border_ufoc_col;
static float master_frac = MASTER_WIDTH;
static uint gaps = GAPS;
static uint scr_width;
static uint scr_height;
static uint open_windows = 0;
static int drag_start_x, drag_start_y;
static int drag_orig_x, drag_orig_y, drag_orig_w, drag_orig_h;

static uint reserve_left = 0;
static uint reserve_right = 0;
static uint reserve_top = 0;
static uint reserve_bottom = 0;
INIT_WORKSPACE

static void
add_client(Window w)
{
	Client *c = malloc(sizeof(Client));
	if (!c) {
		fprintf(stderr, "sxwm: could not alloc memory for client\n");
		return;
	}
	c->win = w;
	c->next = workspaces[current_ws];
	workspaces[current_ws] = c;
	if (!focused)
		focused = c;
	++open_windows;

	XSelectInput(dpy, w,
			EnterWindowMask | LeaveWindowMask |
			FocusChangeMask | PropertyChangeMask |
			StructureNotifyMask);
	Atom protos[] = { atom_wm_delete };
	XSetWMProtocols(dpy, w, protos, 1);

	XWindowAttributes wa;
	XGetWindowAttributes(dpy, w, &wa);
	c->x = wa.x;
	c->y = wa.y;
	c->w = wa.width;
	c->h = wa.height;
	c->fixed = False;
	c->floating = False;
	c->fullscreen = False;

	if (global_floating) {
		XSetWindowBorder(dpy, c->win, border_foc_col);
		XSetWindowBorderWidth(dpy, c->win, BORDER_WIDTH);
	}
	XRaiseWindow(dpy, w);
}

static uint
clean_mask(uint mask)
{
	return mask & ~(LockMask | Mod2Mask | Mod3Mask);
}

static void
close_focused(void)
{
	if (!focused) return;

	Atom *protos;
	int n;
	if (XGetWMProtocols(dpy, focused->win, &protos, &n) && protos) {
		for (int i = 0; i < n ; ++i)
			if (protos[i] == atom_wm_delete) {
				XEvent ev = { .xclient = {
					.type = ClientMessage,
					.window = focused->win,
					.message_type = XInternAtom(dpy,"WM_PROTOCOLS",False),
					.format = 32
				}};
				ev.xclient.data.l[0] = atom_wm_delete;
				ev.xclient.data.l[1] = CurrentTime;
				XSendEvent(dpy, focused->win, False, NoEventMask, &ev);
				XFree(protos);
				return;
			}
		XFree(protos);
	}
	XKillClient(dpy, focused->win);
}

static void
dec_gaps(void)
{
	if (gaps > 0) {
		--gaps;
		tile();
		update_borders();
	}
}

static void
focus_next(void)
{
	if (!focused)
		return;

	focused = (focused->next ? focused->next : workspaces[current_ws]);
	XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, focused->win);
	update_borders();
}

static void
focus_prev(void)
{
	if (!focused)
		return;

	Client *p = workspaces[current_ws], *prev = NULL;
	while (p && p != focused) {
		prev = p;
		p = p->next;
	}

	if (!prev) {
		while (p->next) p = p->next;
		focused = p;
	} else {
		focused = prev;
	}

	XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, focused->win);
	update_borders();
}

static void
grab_keys(void)
{
	KeyCode keycode;
	uint modifiers[] = { 0, LockMask, Mod2Mask, LockMask|Mod2Mask };

	/* ungrab all keys */
	XUngrabKey(dpy, AnyKey, AnyModifier, root);

	for (uint i = 0; i < LENGTH(binds); ++i) {
		if ((keycode = XKeysymToKeycode(dpy, binds[i].keysym))) {
			for (uint j = 0; j < LENGTH(modifiers); ++j) {
				XGrabKey(dpy, keycode,
						binds[i].mods | modifiers[j],
						root, True, GrabModeAsync, GrabModeAsync);
			}
		}
	}
}

static void
hdl_button(XEvent *xev)
{
	XButtonEvent *e = &xev->xbutton;
	Window w = e->subwindow;
	if (!w) return;

	Client *head = workspaces[current_ws];
	for (Client *c = head; c; c = c->next) {
		if (c->win != w)
			continue;

		if (((e->state & MOD) && e->button == Button1 && !c->floating) ||
			((e->state & MOD) && e->button == Button3 && !c->floating)) {
			focused = c;
			toggle_floating();
		}

		if (!c->floating)
			return;

		if (c->fixed && e->button == Button3)
			return;

		Cursor cur = (e->button == Button1) ? c_move : c_resize;
		XGrabPointer(dpy, root, True,
				ButtonReleaseMask|PointerMotionMask,
				GrabModeAsync, GrabModeAsync,
				None, cur, CurrentTime);

		drag_client		= c;
		drag_start_x	= e->x_root;
		drag_start_y	= e->y_root;
		drag_orig_x		= c->x;
		drag_orig_y		= c->y;
		drag_orig_w		= c->w;
		drag_orig_h		= c->h;
		drag_mode		= (e->button == Button1 ? DRAG_MOVE : DRAG_RESIZE);
		focused			= c;
		XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
		update_borders();
		XRaiseWindow(dpy, c->win);
		return;
	}
}

static void
hdl_button_release(XEvent *xev)
{
	(void)xev;
	XUngrabPointer(dpy, CurrentTime);
	drag_mode	= DRAG_NONE;
	drag_client = NULL;
}

static void
hdl_client_msg(XEvent *xev)
{
	if (xev->xclient.message_type == atom_net_wm_state) {
		long action = xev->xclient.data.l[0];
		Atom  target = xev->xclient.data.l[1];
		if (target == atom_net_wm_state_fullscreen) {
			if (action == 1 || action == 2)
				toggle_fullscreen();
			else if (action == 0 && focused && focused->fullscreen)
				toggle_fullscreen();
		}
		return;
	}
}

static void
hdl_config_req(XEvent *xev)
{
	XConfigureRequestEvent *e = &xev->xconfigurerequest;
	Client *c = NULL;

	for (uint ws = 0; ws < NUM_WORKSPACES && !c; ++ws)
		for (c = workspaces[ws]; c; c = c->next)
			if (c->win == e->window) break;


	if (!c || c->floating || c->fullscreen) {
		/* allow the client to configure itself */
		XWindowChanges wc = { .x = e->x, .y = e->y,
			.width  = e->width,  .height = e->height,
			.border_width = e->border_width,
			.sibling = e->above, .stack_mode = e->detail };
		XConfigureWindow(dpy, e->window, e->value_mask, &wc);
		return;
	}

	/* managed and tiling – ignore size hints unless it’s fixed */
	if (c->fixed)
		return;
}

static void
hdl_dummy(XEvent *xev)
{
	(void) xev;
}

static void
hdl_destroy_ntf(XEvent *xev)
{
	Window w = xev->xdestroywindow.window;

	Client *prev = NULL, *c = workspaces[current_ws];
	while (c && c->win != w) {
		prev = c;
		c = c->next;
	}
	if (c) {
		if (focused == c) {
			if (c->next)
				focused = c->next;
			else if (prev)
				focused = prev;
			else
				focused = NULL;
		}
		if (!prev)
			workspaces[current_ws] = c->next;
		else
			prev->next = c->next;
		free(c);
		update_net_client_list();
		--open_windows;
	}

	tile();
	update_borders();

	if (focused) {
		XSetInputFocus(dpy, focused->win,
				RevertToPointerRoot, CurrentTime);
		XRaiseWindow(dpy, focused->win);
	}
}

static void
hdl_enter(XEvent *xev)
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

static void
hdl_keypress(XEvent *xev)
{
	KeySym keysym = XLookupKeysym(&xev->xkey, 0);
	uint mods = clean_mask(xev->xkey.state);

	for (uint i = 0; i < LENGTH(binds); ++i) {
		if (keysym == binds[i].keysym && mods == clean_mask(binds[i].mods)) {
			if (binds[i].is_func)
				binds[i].action.fn();
			else
				spawn(binds[i].action.cmd);
			return;
		}
	}
}

static void
hdl_map_req(XEvent *xev)
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
	ulong nitems, bytes_after;
	Atom *types = NULL;

	if (XGetWindowProperty(dpy, cr->window,
				atom_wm_window_type, 0, 1, False,
				XA_ATOM, &type, &format,
				&nitems, &bytes_after,
				(u_char**)&types) == Success && types)
	{
		if (nitems > 0 && types[0] == atom_net_wm_window_type_dock) {
			XFree(types);

			XMapWindow(dpy, cr->window);
			ulong *strut = NULL;
			if (XGetWindowProperty(dpy, cr->window,
						atom_wm_strut_partial, 0, 12, False,
						XA_CARDINAL, &type, &format,
						&nitems, &bytes_after,
						(u_char**)&strut) == Success && strut)
			{
				if (nitems >= 4) {
					reserve_left	= strut[0];
					reserve_right	= strut[1];
					reserve_top		= strut[2];
					reserve_bottom	= strut[3];
				}
				XFree(strut);
			}

			ulong workarea[4] = {
				reserve_left,
				reserve_top,
				scr_width  - reserve_left	- reserve_right,
				scr_height - reserve_top	- reserve_bottom
			};
			XChangeProperty(dpy, root,
					atom_net_workarea, XA_CARDINAL, 32,
					PropModeReplace, (u_char*)workarea, 4);
			return;
		}
		XFree(types);
	}

	if (open_windows == MAXCLIENTS)
		return;
	add_client(cr->window);
	Client *c = workspaces[current_ws];
	Window trans;

	if (XGetTransientForHint(dpy, c->win, &trans))
		c->floating = True;

	XSizeHints sh; long supplied;
	if (XGetWMNormalHints(dpy, c->win, &sh, &supplied) &&
			(sh.flags & PMinSize) && (sh.flags & PMaxSize) &&
			sh.min_width == sh.max_width &&
			sh.min_height == sh.max_height) {
		c->floating = True;
		c->fixed = True;

		XSetWindowBorderWidth(dpy, c->win, BORDER_WIDTH);
		XSetWindowBorder      (dpy, c->win,
				(c == focused ? border_foc_col : border_ufoc_col));
	}

	{
		Window transient;
		if (XGetTransientForHint(dpy, cr->window, &transient)) {
			Client *c = workspaces[current_ws];
			c->floating = True;
			XSetWindowBorderWidth(dpy, c->win, BORDER_WIDTH);
			XSetWindowBorder (dpy, c->win,
					(c == focused ? border_foc_col : border_ufoc_col));

			if (c->w < 64 || c->h < 64) {
				int w = (c->w < 64 ? 640 : c->w);
				int h = (c->h < 64 ? 480 : c->h);
				int x = (scr_width  - w) / 2;
				int y = (scr_height - h) / 2;
				XMoveResizeWindow(dpy, c->win, x, y, w, h);
			}
		}
	}

	XMapWindow(dpy, cr->window);
	update_net_client_list();
	if (!global_floating)
		tile();
	update_borders();
}

static void
hdl_motion(XEvent *xev)
{
	if (drag_mode == DRAG_NONE || !drag_client)
		return;

	XMotionEvent *e = &xev->xmotion;
	if (e->time - last_motion_time <= (1000 / MOTION_THROTTLE))
		return;
	last_motion_time = e->time;

	int dx = e->x_root - drag_start_x;
	int dy = e->y_root - drag_start_y;
	int nx = drag_orig_x + dx;
	int ny = drag_orig_y + dy;

	if (drag_mode == DRAG_MOVE) {
		int outer_w = drag_client->w + 2 * BORDER_WIDTH;
		int outer_h = drag_client->h + 2 * BORDER_WIDTH;

		if (UDIST(nx, 0) <= SNAP_DISTANCE) {
			nx = 0;
		} else if (UDIST(nx + outer_w, scr_width) <= SNAP_DISTANCE) {
			nx = scr_width - outer_w;
		}

		/* snap y */
		if (UDIST(ny, 0) <= SNAP_DISTANCE) {
			ny = 0;
		} else if (UDIST(ny + outer_h, scr_height) <= SNAP_DISTANCE) {
			ny = scr_height - outer_h;
		}

		if (!drag_client->floating &&
				(UDIST(nx, drag_client->x) > SNAP_DISTANCE ||
				 UDIST(ny, drag_client->y) > SNAP_DISTANCE)) {
			toggle_floating();
		}

		XMoveWindow(dpy, drag_client->win, nx, ny);
		drag_client->x = nx;
		drag_client->y = ny;
	}


	else {
		/* resize clamp is 20px */
		int nw = drag_orig_w + dx;
		int nh = drag_orig_h + dy;
		drag_client->w = nw < 20 ? 20 : nw;
		drag_client->h = nh < 20 ? 20 : nh;
		XResizeWindow(dpy, drag_client->win,
				drag_client->w, drag_client->h);
	}
}

static void
inc_gaps(void)
{
	if (gaps < MAXGAPS) {
		++gaps;
		tile();
		update_borders();
	}
}

static void
move_master_next(void)
{
	if (!workspaces[current_ws] || !workspaces[current_ws]->next)
		return;
	Client *first = workspaces[current_ws];
	workspaces[current_ws] = first->next;
	first->next = NULL;
	Client *tail = workspaces[current_ws];
	while (tail->next)
		tail = tail->next;
	tail->next = first;
	tile();
	update_borders();
}

static void
move_master_prev(void)
{
	if (!workspaces[current_ws] || !workspaces[current_ws]->next)
		return;
	Client *prev = NULL, *cur = workspaces[current_ws];
	while (cur->next) {
		prev = cur;
		cur	 = cur->next;
	}
	prev->next	= NULL;
	cur->next	= workspaces[current_ws];
	workspaces[current_ws]		= cur;
	tile();
	update_borders();
}

static void
move_to_workspace(uint ws)
{
	if (!focused || ws >= NUM_WORKSPACES || ws == current_ws)
		return;

	if (focused->fullscreen) {
		focused->fullscreen = False;
		XMoveResizeWindow(dpy, focused->win, focused->orig_x, focused->orig_y, focused->orig_w, focused->orig_h);
		XSetWindowBorderWidth(dpy, focused->win, BORDER_WIDTH);
	}

	/* remove from current list */
	Client **pp = &workspaces[current_ws];
	while (*pp && *pp != focused) pp = &(*pp)->next;
	if (*pp) *pp = focused->next;

	/* push to target list */
	focused->next = workspaces[ws];
	workspaces[ws] = focused;

	/* unmap it here if switching away */
	XUnmapWindow(dpy, focused->win);

	/* tile current ws */
	tile();
	focused = workspaces[current_ws];
	if (focused)
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
}

static void
other_wm(void)
{
	XSetErrorHandler(other_wm_err);
	XChangeWindowAttributes(dpy, root, CWEventMask, 
			&(XSetWindowAttributes){.event_mask = SubstructureRedirectMask});
	XSync(dpy, False);
	XSetErrorHandler(xerr);
	XChangeWindowAttributes(dpy, root, CWEventMask, 
			&(XSetWindowAttributes){.event_mask = 0});
	XSync(dpy, False);
}

static int
other_wm_err(Display *dpy, XErrorEvent *ee)
{
	errx(0, "can't start because another window manager is already running");
	return 0;
	(void) dpy;
	(void) ee;
}

static ulong
parse_col(const char *hex)
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

static void
quit(void)
{
	for (uint ws = 0; ws < NUM_WORKSPACES; ++ws) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			XUnmapWindow(dpy, c->win);
			XKillClient(dpy, c->win);
		}
	}
	XSync(dpy, False);
	XCloseDisplay(dpy);
	errx(0, "quitting...");
}

static void
resize_master_add(void)
{
	if (master_frac < MF_MAX - 0.001f)
		master_frac += ((float) RESIZE_MASTER_AMT / 100);
	tile();
	update_borders();
}

static void
resize_master_sub(void)
{
	if (master_frac > MF_MIN + 0.001f)
		master_frac -= ((float) RESIZE_MASTER_AMT / 100);
	tile();
	update_borders();
}

static void
run(void)
{
	XEvent xev;
	for (;;) {
		XNextEvent(dpy, &xev);
		xev_case(&xev);
	}
}

static void
scan_existing_windows(void)
{
	Window root_return, parent_return;
	Window *children;
	uint nchildren;

	if (XQueryTree(dpy, root, &root_return, &parent_return, &children, &nchildren)) {
		for (uint i = 0; i < nchildren; i++) {
			XWindowAttributes wa;
			if (!XGetWindowAttributes(dpy, children[i], &wa) || wa.override_redirect || wa.map_state != IsViewable)
				continue;

			XEvent fake_event = {0};
			fake_event.type = MapRequest;
			fake_event.xmaprequest.window = children[i];
			hdl_map_req(&fake_event);
		}
		if (children)
			XFree(children);
	}
}

static void
setup(void)
{
	if ((dpy = XOpenDisplay(NULL)) == 0)
		errx(0, "can't open display. quitting...");
	root = XDefaultRootWindow(dpy);

	setup_atoms();
	other_wm();
	grab_keys();

	c_normal = XCreateFontCursor(dpy, XC_left_ptr);
	c_move	 = XCreateFontCursor(dpy, XC_fleur);
	c_resize = XCreateFontCursor(dpy, XC_bottom_right_corner);
	XDefineCursor(dpy, root, c_normal);

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));
	XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);
	XGrabButton(dpy, Button1, MOD, root,
			True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
			GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, Button3, MOD, root,
			True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
			GrabModeAsync, GrabModeAsync, None, None);
	XSync(dpy, False);

	for (int i = 0; i < LASTEvent; ++i)
		evtable[i] = hdl_dummy;

	evtable[ButtonPress]	= hdl_button;
	evtable[ButtonRelease]	= hdl_button_release;
	evtable[ClientMessage]	= hdl_client_msg;
	evtable[ConfigureRequest] = hdl_config_req;
	evtable[DestroyNotify]	= hdl_destroy_ntf;
	evtable[EnterNotify] 	= hdl_enter;
	evtable[KeyPress]		= hdl_keypress;
	evtable[MapRequest]		= hdl_map_req;
	evtable[MotionNotify]	= hdl_motion;

	border_foc_col = parse_col(BORDER_FOC_COL);
	border_ufoc_col = parse_col(BORDER_UFOC_COL);

	scan_existing_windows();
}

static void
setup_atoms(void)
{
	/* bar atoms */
	atom_net_supported				= XInternAtom(dpy, "_NET_SUPPORTED",			False);
	atom_wm_strut_partial			= XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL",		False);
	atom_wm_window_type				= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE",		False);
	atom_net_wm_window_type_dock	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK",	False);
	atom_net_workarea				= XInternAtom(dpy, "_NET_WORKAREA",				False);

	Atom support_list[] = {
		XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL",	False),
		XInternAtom(dpy, "_NET_WM_WINDOW_TYPE",		False),
		XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK",False),
		XInternAtom(dpy, "_NET_WORKAREA",			False),
	};

	XChangeProperty(dpy, root,
			atom_net_supported,
			XA_ATOM, 32, PropModeReplace,
			(u_char*)support_list,
			sizeof(support_list)/sizeof(Atom));

	/* workspace atoms */
	Atom a_num = XInternAtom(dpy, 	"_NET_NUMBER_OF_DESKTOPS",	False);
	Atom a_names= XInternAtom(dpy, 	"_NET_DESKTOP_NAMES",		False);

	long  num = NUM_WORKSPACES;
	XChangeProperty(dpy, root, a_num, XA_CARDINAL, 32,
			PropModeReplace, (u_char*)&num, 1);

	const char names[] = WORKSPACE_NAMES;
	uint names_len = sizeof(names);

	XChangeProperty(dpy, root,
			a_names,
			XInternAtom(dpy, "UTF8_STRING", False),
			8,
			PropModeReplace,
			(u_char*)names,
			names_len);

	ulong initial = current_ws;
	XChangeProperty(dpy, root,
			XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False),
			XA_CARDINAL, 32,
			PropModeReplace,
			(u_char*)&initial, 1);
	/* fullscreen atoms */
	atom_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	atom_net_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

	/* delete atoms */
	atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
}

static void
spawn(const char **cmd)
{
	pid_t pid;

	if (!cmd || !cmd[0])
		return;

	pid = fork();
	if (pid < 0) {
		errx(1, "sxwm: fork failed");
	} else if (pid == 0) {
		setsid();
		execvp(cmd[0], (char *const *)cmd);
		errx(1, "sxwm: execvp '%s' failed", cmd[0]);
	}
}

static void
tile(void)
{
	uint total_windows = 0;
	Client *head = workspaces[current_ws];
	for (Client *c = head; c; c = c->next) {
		if (c->fullscreen)
			return;

		if (!c->floating)
			++total_windows;
	}
	if (total_windows == 0)
		return;

	uint stack_count = total_windows - 1;

	uint left	= reserve_left + gaps;
	uint top	= reserve_top + gaps;
	uint right	= reserve_right + gaps;
	uint bottom	= reserve_bottom + gaps;

	uint workarea_width  = scr_width  - left - right;
	uint workarea_height = scr_height - top  - bottom;

	uint column_gap	= (stack_count > 0 ? gaps : 0);
	uint master_width = (stack_count > 0)
		? workarea_width * master_frac
		: workarea_width;
	uint stack_width  = (stack_count > 0)
		? (workarea_width - master_width - column_gap)
		: 0;

	uint master_x = left;
	uint stack_x  = left + master_width + column_gap;

	uint total_row_gaps	= (stack_count > 1 ? gaps * (stack_count - 1) : 0);
	uint row_height	= (stack_count > 0)
		? (workarea_height - total_row_gaps) / stack_count
		: 0;
	uint used_height = row_height * (stack_count > 0 ? stack_count - 1 : 0)
		+ total_row_gaps;
	uint last_row_height = (stack_count > 0)
		? (workarea_height - used_height)
		: workarea_height;

	uint index = 0;
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		if (c->floating)
			continue;

		XWindowChanges changes = { .border_width = BORDER_WIDTH };

		if (index == 0) {
			changes.x		= master_x;
			changes.y		= top;
			changes.width	= master_width  - 2*BORDER_WIDTH;
			changes.height	= workarea_height - 2*BORDER_WIDTH;
		} else {
			uint row	= index - 1;
			uint y_pos	= top + row * (row_height + gaps);
			uint height	= (row < stack_count - 1)
				? row_height
				: last_row_height;

			changes.x		= stack_x;
			changes.y		= y_pos;
			changes.width	= stack_width - 2*BORDER_WIDTH;
			changes.height	= height - 2*BORDER_WIDTH;
		}

		XSetWindowBorder(dpy, c->win,
				(index == 0 ? border_foc_col : border_ufoc_col));
		XConfigureWindow(dpy, c->win,
				CWX|CWY|CWWidth|CWHeight|CWBorderWidth,
				&changes);
		++index;
	}
}

static void
toggle_floating(void)
{
	if (!focused) return;

	focused->floating = !focused->floating;
	if (focused->fullscreen) {
		focused->fullscreen = False;
		tile();
		XSetWindowBorderWidth(dpy, focused->win, BORDER_WIDTH);
	}

	if (focused->floating) {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, focused->win, &wa);
		focused->x = wa.x;
		focused->y = wa.y;
		focused->w = wa.width;
		focused->h = wa.height;

		XConfigureWindow(dpy, focused->win,
				CWX|CWY|CWWidth|CWHeight,
				&(XWindowChanges){
				.x = focused->x,
				.y = focused->y,
				.width	= focused->w,
				.height = focused->h
				});
	}

	tile();
	update_borders();

	/* floating windows are on top */
	if (focused->floating) {
		XRaiseWindow(dpy, focused->win);
		XSetInputFocus(dpy, focused->win,
				RevertToPointerRoot, CurrentTime);
	}
}

static void
toggle_floating_global(void)
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

			XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight,
					&(XWindowChanges){
					.x		= c->x,
					.y		= c->y,
					.width	= c->w,
					.height	= c->h
					}
					);
			XRaiseWindow(dpy, c->win);
		}
	}

	tile();
	update_borders();
}

static void
toggle_fullscreen(void)
{
	if (!focused)
		return;

	if (focused->floating)
		focused->floating = False;

	focused->fullscreen = !focused->fullscreen;

	if (focused->fullscreen) {
		XWindowAttributes wa;
		XGetWindowAttributes(dpy, focused->win, &wa);
		focused->orig_x = wa.x;
		focused->orig_y = wa.y;
		focused->orig_w = wa.width;
		focused->orig_h = wa.height;

		uint fs_x = 0;
		uint fs_y = 0;
		uint fs_w = scr_width;
		uint fs_h = scr_height;

		XSetWindowBorderWidth(dpy, focused->win, 0);
		XMoveResizeWindow(dpy, focused->win, fs_x, fs_y, fs_w, fs_h);
		XRaiseWindow(dpy, focused->win);
	} else {
		XMoveResizeWindow(dpy, focused->win, focused->orig_x, focused->orig_y, focused->orig_w, focused->orig_h);
		XSetWindowBorderWidth(dpy, focused->win, BORDER_WIDTH);
		tile();
		update_borders();
	}
}

static void
update_borders(void)
{
	for (Client *c = workspaces[current_ws]; c; c = c->next) {
		XSetWindowBorder(dpy, c->win,
				(c == focused ? border_foc_col : border_ufoc_col));
	}
}

static void
update_net_client_list(void)
{
	Window wins[MAXCLIENTS];
	int n = 0;
	for (int ws = 0; ws < NUM_WORKSPACES; ++ws) {
		for (Client *c = workspaces[ws]; c; c = c->next) {
			wins[n++] = c->win; /* has to be n++ or well get an off by one error i think */
		}
	}
	Atom prop = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	XChangeProperty(dpy, root, prop,
			XA_WINDOW, 32, PropModeReplace,
			(u_char*)wins, n);
}

static void
change_workspace(uint ws)
{
	if (ws >= NUM_WORKSPACES || ws == current_ws)
		return;

	/* unmap old desktop */
	for (Client *c = workspaces[current_ws]; c; c=c->next)
		XUnmapWindow(dpy, c->win);

	current_ws = ws;

	/* map new desktop */
	for (Client *c = workspaces[current_ws]; c; c=c->next)
		XMapWindow(dpy, c->win);

	/* retile & refocus */
	tile();
	if (workspaces[current_ws]) {
		focused = workspaces[current_ws];
		XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	}

	/* update atom */
	ulong cd = current_ws;
	XChangeProperty(dpy, root,
			XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False),
			XA_CARDINAL, 32, PropModeReplace,
			(u_char*)&cd, 1);
}

static int
xerr(Display *dpy, XErrorEvent *ee)
{
	/* ignore noise and non fatal errors */
	static const struct {
		uint req, code;
	} ignore[] = {
		{ 0, BadWindow },
		{ X_GetGeometry, BadDrawable },
		{ X_SetInputFocus, BadMatch },
		{ X_ConfigureWindow, BadMatch },
	};

	for (size_t i = 0; i < sizeof(ignore)/sizeof(ignore[0]); ++i)
		if ((ignore[i].req == 0 || ignore[i].req  == ee->request_code) &&
				(ignore[i].code == ee->error_code))
			return 0;

	return 0;
	(void) dpy;
	(void) ee;
}

static void
xev_case(XEvent *xev)
{
	if (xev->type >= 0 && xev->type < LASTEvent)
		evtable[xev->type](xev);
	else
		printf("sxwm: invalid event type: %d\n", xev->type);
}

int
main(int ac, char **av)
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
