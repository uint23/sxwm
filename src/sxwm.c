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
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "defs.h"

typedef void (*EventHandler)(XEvent *);

static void hdl_dummy(XEvent *xev);
static void hdl_keypress(XEvent *xev);
static void other_wm(void);
static int other_wm_err(Display *dpy, XErrorEvent *ee);
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

#include "usercfg.h"

static void
hdl_dummy(XEvent *xev){}

static void
hdl_keypress(XEvent *xev)
{
	KeySym keysym;
	XKeyEvent *ev = &xev->xkey;
	unsigned int modifiers;

	modifiers = ev->state;
	keysym = XkbKeycodeToKeysym(dpy, ev->keycode, 0, 0);

	int lenbindings = sizeof(binds) / sizeof(binds[0]);
	for (int i = 0; i < lenbindings; ++i) {
		if (keysym == binds[i].keysym && modifiers == binds[i].mods) {
			if (binds[i].is_func)
				binds[i].action.fn();
			else
				spawn(binds[i].action.cmd);
			return;
		}
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
	XSync(dpy, False);
}

static int
other_wm_err(Display *dpy, XErrorEvent *ee)
{
	errx(0, "sxwm: can't start because another window manager is already running");
	return 0;
	if (dpy && ee) return 0;
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
		SubstructureRedirectMask | KeyPressMask | KeyReleaseMask
	);

	for (int i = 0; i < LASTEvent; ++i)
		evtable[i] = hdl_dummy;

	evtable[KeyPress] = hdl_keypress;
}

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
