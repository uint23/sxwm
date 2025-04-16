/*  See LICENSE for more info

	sxwm is a user-friendly, easily configurable yet powerful
	tiling window manager inspired by window managers such as
	DWM and i3

	The userconfig is designed to be as user-friendly as
	possible, and I hope it is easy to configure even without
	knowledge of C or programming, although most people who
	will use this will probably be programmers :)

*///			(C) Abhinav Prasai 2025

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "defs.h"

typedef void (*EventHandler)(XEvent *);

static unsigned int clean_mask(unsigned int mask);
static void hdl_dummy(XEvent *xev);
static void hdl_config_req(XEvent *xev);
static void hdl_destroy_ntf(XEvent *xev);
static void hdl_keypress(XEvent *xev);
static void hdl_map_req(XEvent *xev);
static void hdl_unmap_ntf(XEvent *xev);
static void other_wm(void);
static int other_wm_err(Display *dpy, XErrorEvent *ee);
static unsigned long parse_col(const char *hex);
static void quit(void);
static void run(void);
static void setup(void);
static void spawn(const char **cmd);
static int xerr(Display *dpy, XErrorEvent *ee);
static void xev_case(XEvent *xev);

static Client clients[MAXCLIENTS] = {0};
static EventHandler evtable[LASTEvent];
static Display 	*dpy;
static Window	root;

static unsigned long border_foc_col;
static unsigned long border_ufoc_col;
static unsigned int scr_width;
static unsigned int scr_height;

#include "usercfg.h"

static unsigned int
clean_mask(unsigned int mask)
{
	return mask & ~(LockMask | Mod2Mask | Mod3Mask);
}

static void
hdl_dummy(XEvent *xev)
{}

static void
hdl_config_req(XEvent *xev)
{
	XConfigureRequestEvent ev = xev->xconfigurerequest;
	XWindowChanges wc;
	wc.y = ev.y;
	wc.width = ev.width;
	wc.height = ev.height;
	wc.border_width = ev.border_width;
	wc.sibling = ev.above;
	wc.stack_mode = ev.detail;

	XConfigureWindow(dpy, ev.window, ev.value_mask, &wc);
	printf("sxwm: window configured: %ld\n", ev.window);
}


static void
hdl_destroy_ntf(XEvent *xev)
{
	XDestroyWindowEvent ev = xev->xdestroywindow;
	XDestroyWindow(dpy, ev.window);
	printf("sxwm: window destroyed: %ld\n", ev.window);
}

static void
hdl_keypress(XEvent *xev)
{
	KeySym keysym;
	XKeyEvent *ev = &xev->xkey;
	unsigned int modifiers;

	keysym = XkbKeycodeToKeysym(dpy, ev->keycode, 0, 0);
	modifiers = clean_mask(ev->state);

	int lenbindings = sizeof(binds) / sizeof(binds[0]);
	for (int i = 0; i < lenbindings; ++i) {
		if (keysym == binds[i].keysym && modifiers == clean_mask(binds[i].mods)) {
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
	XMapRequestEvent ev = xev->xmaprequest;
	XSetWindowBorder(dpy, ev.window, border_foc_col);
	XSetWindowBorderWidth(dpy, ev.window, BORDER_WIDTH);
	XMapWindow(dpy, ev.window);

	XWindowAttributes wa;
	if (!XGetWindowAttributes(dpy, ev.window, &wa))
		return;

	printf("sxwm: window mapped: %ld\n", ev.window);
}

static void
hdl_unmap_ntf(XEvent *xev)
{
	XUnmapEvent ev = xev->xunmap;
	XUnmapWindow(dpy, ev.window);
	printf("sxwm: window unmapped: %ld\n", ev.window);
}

static void
other_wm(void)
{
	XSetErrorHandler(other_wm_err);
	XChangeWindowAttributes(dpy, root, CWEventMask, 
			&(XSetWindowAttributes){.event_mask = SubstructureRedirectMask});
	XSync(dpy, False);
	XSetErrorHandler(xerr);
	XSync(dpy, False);
}

static int
other_wm_err(Display *dpy, XErrorEvent *ee)
{
	errx(0, "sxwm: can't start because another window manager is already running");
	return 0;
	if (dpy && ee) return 0;
}

static unsigned long
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
	errx(0, "sxwm: quitting...");
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
	dpy = XOpenDisplay(NULL);
	if (dpy == 0)
		errx(0, "sxwm: can't open display.");

	root = XDefaultRootWindow(dpy);
	other_wm();
	XSelectInput(dpy, root,
			SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

	for (int i = 0; i < LASTEvent; ++i)
		evtable[i] = hdl_dummy;

	evtable[ConfigureRequest] = hdl_config_req;
	evtable[DestroyNotify] = hdl_destroy_ntf;
	evtable[KeyPress] = hdl_keypress;
	evtable[MapRequest] = hdl_map_req;
	evtable[UnmapNotify] = hdl_unmap_ntf;

	border_foc_col = parse_col(BORDER_FOC_COL);
	border_ufoc_col = parse_col(BORDER_UFOC_COL);
	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy)); }

static void
spawn(const char **cmd)
{
	if (!cmd) return;
	printf("sxwm: attempting to spawn: %s\n", cmd[0]);

	pid_t pid = fork();
	if (pid == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(cmd[0], (char *const*)cmd);
		errx(1, "sxwm: execvp '%s' failed\n", cmd[0]);
	} else if (pid < 0) {
		fprintf(stderr, "sxwm: failed to fork process\n");
	}
}

static int
xerr(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "sxwm: fatal error\nrequest code:%d\nerror code:%d",
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
