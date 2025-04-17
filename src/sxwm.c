/*  See LICENSE for more info
 *
 *	sxwm is a user-friendly, easily configurable yet powerful
 *	tiling window manager inspired by window managers such as
 *	DWM and i3
 *
 *	The userconfig is designed to be as user-friendly as
 *	possible, and I hope it is easy to configure even without
 *	knowledge of C or programming, although most people who
 *	will use this will probably be programmers :)
 *			(C) Abhinav Prasai 2025
*/

#include <X11/cursorfont.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "defs.h"

static void add_client(Window w);
static uint clean_mask(uint mask);
static void close_focused(void);
static void focus_next(void);
static void focus_prev(void);
static void grab_keys(void);
static void hdl_button(XEvent *);
static void hdl_button_release(XEvent *);
static void hdl_dummy(XEvent *xev);
static void hdl_destroy_ntf(XEvent *xev);
static void hdl_enter(XEvent *xev);
static void hdl_keypress(XEvent *xev);
static void hdl_map_req(XEvent *xev);
static void hdl_motion(XEvent *);
static void other_wm(void);
static int other_wm_err(Display *dpy, XErrorEvent *ee);
static uint64_t parse_col(const char *hex);
static void quit(void);
static void run(void);
static void setup(void);
static void spawn(const char **cmd);
static void tile(void);
static void toggle_floating(void);
static void update_borders(void);
static int xerr(Display *dpy, XErrorEvent *ee);
static void xev_case(XEvent *xev);

static Cursor c_normal, c_move, c_resize;
static Client *clients = NULL;
static Client *drag_client = NULL;
static Client *focused = NULL;
static EventHandler evtable[LASTEvent];
static Display *dpy;
static Window root;

static uint64_t border_foc_col;
static uint64_t border_ufoc_col;
static uint scr_width;
static uint scr_height;
static int drag_start_x, drag_start_y;
static int drag_orig_x, drag_orig_y, drag_orig_w, drag_orig_h;

static uint open_windows = 0;
#include "usercfg.h"

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

	// ICCCM first ;)
	Window w = focused->win;
	Atom *protocols;
	int n;
	if (XGetWMProtocols(dpy, w, &protocols, &n)) {
		Atom del = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
		for (int i = 0; i < n; ++i) {
			if (protocols[i] == del) {
				XEvent ev = { 0 };
				ev.xclient.type			= ClientMessage;
				ev.xclient.window		= w;
				ev.xclient.message_type	= XInternAtom(dpy, "WM_PROTOCOLS", False);
				ev.xclient.format		= 32;
				ev.xclient.data.l[0]	= del;
				ev.xclient.data.l[1]	= CurrentTime;
				XSendEvent(dpy, w, False, NoEventMask, &ev);
				XFree(protocols);
				return;
			}
		}
		XFree(protocols);
	}
	XKillClient(dpy, w);
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

	// ungrab all keys
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

	// find the Client*
	for (Client *c = clients; c; c = c->next) {
		if (c->win == w && c->floating) {

			Cursor cur = (e->button == Button1) ? c_move : c_resize;
			XGrabPointer(dpy, root, True,
					ButtonReleaseMask|PointerMotionMask,
					GrabModeAsync, GrabModeAsync,
					None,
					cur,
					CurrentTime);
			
			drag_client		= c;
			drag_start_x	= e->x_root;
			drag_start_y	= e->y_root;
			drag_orig_x		= c->x;
			drag_orig_y		= c->y;
			drag_orig_w		= c->w;
			drag_orig_h		= c->h;
			drag_mode = (e->button == Button1 ? DRAG_MOVE : DRAG_RESIZE);
			focused = c;
			XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
			update_borders();
			return;
		}
	}
}

static void
hdl_button_release(XEvent *xev)
{
    (void)xev;
	XUngrabPointer(dpy, CurrentTime);
	drag_mode   = DRAG_NONE;
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

	tile();
	update_borders();
}

static void
hdl_motion(XEvent *xe)
{
	if (drag_mode == DRAG_NONE || !drag_client) return;
	XMotionEvent *e = &xe->xmotion;
	int dx = e->x_root - drag_start_x;
	int dy = e->y_root - drag_start_y;

	if (drag_mode == DRAG_MOVE) {
		drag_client->x = drag_orig_x + dx;
		drag_client->y = drag_orig_y + dy;
		XMoveWindow(dpy, drag_client->win,
				drag_client->x, drag_client->y);
	} else { // drag resize
		int nw = drag_orig_w + dx;
		int nh = drag_orig_h + dy;
		if (nw < 20) nw = 20;
		if (nh < 20) nh = 20;
		drag_client->w = nw;
		drag_client->h = nh;
		XResizeWindow(dpy, drag_client->win,
				drag_client->w, drag_client->h);
	}
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
	if (!open_windows)
		return;

	int masterx = GAPS + BORDER_WIDTH,
		mastery = GAPS + BORDER_WIDTH,
		availableh = scr_height - (GAPS * 2),
		masterw, masterh, stackw = 0, stackwinh = 0,
		stack_count = open_windows - 1;

	if (open_windows == 1) {
		masterw = scr_width - (GAPS * 2 + BORDER_WIDTH * 2);
		masterh = availableh - (BORDER_WIDTH * 2);
	} else {
		int total_gapsw = GAPS * 4, total_bordersw = BORDER_WIDTH * 4;
		masterw = (scr_width - total_gapsw - total_bordersw) / 2;
		stackw = masterw;
		masterh = availableh - (BORDER_WIDTH * 2);

		int total_gapsh = (stack_count > 0 ? GAPS * (stack_count - 1) : 0),
			total_bordersh = BORDER_WIDTH * 2 * stack_count,
			total_stackh = availableh - total_gapsh - total_bordersh;
		stackwinh = (stack_count > 0 ? total_stackh / stack_count : 0);
	}

	int stackx = masterx + masterw + GAPS + (BORDER_WIDTH * 2),
		stacky = GAPS;
	Client *c = clients;
	uint i = 0;

	for (; c; c = c->next, ++i) {
		if (c->floating)
			continue;

		XWindowChanges changes = { .border_width = BORDER_WIDTH };
		if (i == 0) {
			changes.x = masterx;
			changes.y = mastery;
			changes.width = masterw;
			changes.height = masterh;
		} else {
			changes.x = stackx;
			changes.y = stacky + BORDER_WIDTH; // adjust for border
			changes.width = stackw;
			changes.height = stackwinh;
			if (i == open_windows - 1) {
				int used = stacky - GAPS + stackwinh + (BORDER_WIDTH * 2);
				changes.height += (availableh - used);
			}
			stacky += stackwinh + (BORDER_WIDTH * 2) + GAPS;
		}
		XSetWindowBorder(dpy, c->win,
				i == 0 ? border_foc_col : border_ufoc_col);

		XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &changes);
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
				.width  = focused->w,
				.height = focused->h
				});
	}

	tile();
	update_borders();

	// floating windows are on top
	if (focused->floating) {
		XRaiseWindow(dpy, focused->win);
		XSetInputFocus(dpy, focused->win,
				RevertToPointerRoot, CurrentTime);
	}
}

static void
update_borders(void)
{
	for (Client *c = clients; c; c = c->next) {
		XSetWindowBorder(dpy, c->win,
				(c == focused ? border_foc_col : border_ufoc_col));
	}
	XSync(dpy, False);
}

static int
xerr(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "sxwm: fatal error\nrequest code:%d\nerror code:%d\n",
			ee->request_code, ee->error_code);
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
