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
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/XKBlib.h>

#include "defs.h"

static void add_client(Window w);
static uint clean_mask(uint mask);
static void close_focused(void);
static void dec_gaps(void);
static void focus_next(void);
static void focus_prev(void);
static void grab_keys(void);
static void hdl_button(XEvent *xev);
static void hdl_button_release(XEvent *xev);
static void hdl_dummy(XEvent *xev);
static void hdl_destroy_ntf(XEvent *xev);
static void hdl_enter(XEvent *xev);
static void hdl_keypress(XEvent *xev);
static void hdl_map_req(XEvent *xev);
static void hdl_motion(XEvent *xev);
static void inc_gaps(void);
static void move_master_next(void);
static void move_master_prev(void);
static void other_wm(void);
static int other_wm_err(Display *dpy, XErrorEvent *ee);
static uint64_t parse_col(const char *hex);
static void quit(void);
static void run(void);
static void setup(void);
static void spawn(const char **cmd);
static void tile(void);
static void toggle_floating(void);
static void toggle_floating_global(void);
static void update_borders(void);
static int xerr(Display *dpy, XErrorEvent *ee);
static void xev_case(XEvent *xev);
#include "usercfg.h"

static Cursor c_normal, c_move, c_resize;
static Client *clients = NULL;
static Client *drag_client = NULL;
static Client *focused = NULL;
static EventHandler evtable[LASTEvent];
static Display *dpy;
static Window root;
static Bool global_floating = False;

static uint64_t last_motion_time = 0;
static uint64_t border_foc_col;
static uint64_t border_ufoc_col;
static uint gaps = GAPS;
static uint scr_width;
static uint scr_height;
static uint open_windows = 0;
static int drag_start_x, drag_start_y;
static int drag_orig_x, drag_orig_y, drag_orig_w, drag_orig_h;


static void
add_client(Window w)
{
	Client *c = malloc(sizeof(Client));
	if (!c) {
		fprintf(stderr, "sxwm: could not alloc memory for client\n");
		return;
	}
	c->win = w;
	c->next = clients;
	clients = c;
	if (!focused)
		focused = c;
	++open_windows;

	XSelectInput(dpy, w,
			EnterWindowMask | LeaveWindowMask |
			FocusChangeMask | PropertyChangeMask |
			StructureNotifyMask);

	XWindowAttributes wa;
	XGetWindowAttributes(dpy, w, &wa);
	c->x = wa.x;
	c->y = wa.y;
	c->w = wa.width;
	c->h = wa.height;
	c->floating = 0;

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
	if (!clients)
		return;

	Window w = focused->win;
	XKillClient(dpy, w);
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

	focused = (focused->next ? focused->next : clients);
	XSetInputFocus(dpy, focused->win, RevertToPointerRoot, CurrentTime);
	XRaiseWindow(dpy, focused->win);
	update_borders();
}

static void
focus_prev(void)
{
	if (!focused)
		return;

	Client *p = clients, *prev = NULL;
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
	update_borders();}

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

	for (Client *c = clients; c; c = c->next) {
		if (c->win != w)
			continue;

		if (((e->state & MOD) && e->button == Button1 && !c->floating) ||
			((e->state & MOD) && e->button == Button3 && !c->floating)) {
			focused = c;
			toggle_floating();
		}

		if (!c->floating)
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
hdl_dummy(XEvent *xev)
{
	(void) xev;
}

static void
hdl_destroy_ntf(XEvent *xev)
{
	Window w = xev->xdestroywindow.window;

	Client *prev = NULL, *c = clients;
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
			clients = c->next;
		else
			prev->next = c->next;
		free(c);
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

	for (Client *c = clients; c; c = c->next) {
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
	if (open_windows == MAXCLIENTS)
		return;

	XConfigureRequestEvent *cr = &xev->xconfigurerequest;
	add_client(cr->window);
	XMapWindow(dpy, cr->window);

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
	if (!clients || !clients->next)
		return;
	Client *first = clients;
	clients = first->next;
	first->next = NULL;
	Client *tail = clients;
	while (tail->next)
		tail = tail->next;
	tail->next = first;
	tile();
	update_borders();
}

static void
move_master_prev(void)
{
	if (!clients || !clients->next)
		return;
	Client *prev = NULL, *cur = clients;
	while (cur->next) {
		prev = cur;
		cur	 = cur->next;
	}
	prev->next	= NULL;
	cur->next	= clients;
	clients		= cur;
	tile();
	update_borders();
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
	if (dpy && ee) return 0;
}

static uint64_t
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
	for (Client *c = clients; c; c = c->next) {
		XUnmapWindow(dpy, c->win);
		XKillClient(dpy, c->win);
	}
	XSync(dpy, False);
	XCloseDisplay(dpy);
	errx(0, "quitting...");
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
setup(void)
{
	if ((dpy = XOpenDisplay(NULL)) == 0)
		errx(0, "can't open display. quitting...");
	root = XDefaultRootWindow(dpy);

	c_normal = XCreateFontCursor(dpy, XC_left_ptr);
	c_move	 = XCreateFontCursor(dpy, XC_fleur);
	c_resize = XCreateFontCursor(dpy, XC_bottom_right_corner);
	XDefineCursor(dpy, root, c_normal);

	other_wm();
	grab_keys();

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
	evtable[DestroyNotify]	= hdl_destroy_ntf;
	evtable[EnterNotify] 	= hdl_enter;
	evtable[KeyPress]		= hdl_keypress;
	evtable[MapRequest]		= hdl_map_req;
	evtable[MotionNotify]	= hdl_motion;

	border_foc_col = parse_col(BORDER_FOC_COL);
	border_ufoc_col = parse_col(BORDER_UFOC_COL);
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
	int n = 0;
	for (Client *c = clients; c; c = c->next)
		if (!c->floating)
			++n;
	if (n == 0) return;

	int stack_count = n - 1;

	int outer_x0 = gaps;
	int outer_y0 = gaps;
	int outer_w  = scr_width  - 2*gaps;
	int outer_h  = scr_height - 2*gaps;

	int inter_x		= (stack_count > 0 ? gaps : 0);
	int master_ow	= (stack_count > 0)
		? (int)(outer_w * MASTER_WIDTH)
		: outer_w;
	int stack_ow  = (stack_count > 0)
		? (outer_w - master_ow - inter_x)
		: 0;

	int master_x0 = outer_x0;
	int stack_x0  = outer_x0 + master_ow + inter_x;

	int inter_y_total	= (stack_count > 1 ? gaps * (stack_count - 1) : 0);
	int each_oh			= (stack_count > 0)
		? (outer_h - inter_y_total) / stack_count
		: 0;

	int used_h = each_oh * (stack_count > 0 ? stack_count - 1 : 0)
		+ inter_y_total;
	int last_oh = (stack_count > 0)
		? (outer_h - used_h)
		: outer_h;

	int i = 0;
	for (Client *c = clients; c; c = c->next) {
		if (c->floating) continue;
		XWindowChanges ch = { .border_width = BORDER_WIDTH };

		if (i == 0) {
			ch.x	= master_x0;
			ch.y	= outer_y0;
			ch.width	= master_ow - 2*BORDER_WIDTH;
			ch.height	= outer_h	- 2*BORDER_WIDTH;
		} else {
			int row = i - 1;
			int outer_y = outer_y0 + row * (each_oh + gaps);
			int outer_hh = (row < stack_count - 1 ? each_oh : last_oh);

			ch.x		= stack_x0;
			ch.y		= outer_y;
			ch.width	= stack_ow - 2*BORDER_WIDTH;
			ch.height	= outer_hh - 2*BORDER_WIDTH;
		}

		XSetWindowBorder(dpy, c->win,
				(i == 0 ? border_foc_col : border_ufoc_col));
		XConfigureWindow(dpy, c->win,
				CWX|CWY|CWWidth|CWHeight|CWBorderWidth,
				&ch);
		++i;
	}
}

static void
toggle_floating(void)
{
	if (!focused) return;
	focused->floating = !focused->floating;

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
	for (Client *c = clients; c; c = c->next) {
		if (!c->floating) {
			any_tiled = True;
			break;
		}
	}

	for (Client *c = clients; c; c = c->next) {
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
update_borders(void)
{
	for (Client *c = clients; c; c = c->next) {
		XSetWindowBorder(dpy, c->win,
				(c == focused ? border_foc_col : border_ufoc_col));
	}
}

static int
xerr(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
			|| ee->error_code == BadDrawable
			|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
			|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch))
		return 0;

	char buf[256];
	XGetErrorText(dpy, ee->error_code, buf, sizeof(buf));
	fprintf(stderr,
			"sxwm: X error:\n"
			"\trequest code: %d\n"
			"\terror code:	 %d (%s)\n"
			"\tresource id:	 0x%lx\n",
			ee->request_code,
			ee->error_code, buf,
			ee->resourceid);
	return 0;
	if (dpy && ee) return 0;
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
