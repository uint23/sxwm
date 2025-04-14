/*  See LICENSE for more info

	sxwm is a user-friendly, easily configurable yet powerful
	tiling window manager inspired by window managers such as
	DWM and i3

	The userconfig is designed to be as user-friendly as
	possible, and I hope it is easy to configure even without
	knowledge of C or programming, although most people who
	will use this will probably be programmers :)

*///			(C) Abhinav Prasai 2025

#include "sxwm.h"

static void
otherwm(void)
{
	XSetErrorHandler(otherwmerr);
	XChangeWindowAttributes(dpy, root, CWEventMask, 
			&(XSetWindowAttributes){.event_mask = SubstructureRedirectMask});
	XSync(dpy, False);
	XSetErrorHandler(xerr);
	XSync(dpy, False);
}

static int
otherwmerr(Display *dpy, XErrorEvent *ee)
{
	errx(0, "sxwm: another window manager is already running, please close it");
	return 0;
	if (dpy && ee) return 0;
}

static void
run(void)
{
	XEvent xev;
	for (;;) {
		XNextEvent(dpy, &xev);
	}
}

static void
setup(void)
{
	dpy = XOpenDisplay(NULL);
	if (dpy == 0)
		errx(0, "sxwm: can't open display.");

	root = XDefaultRootWindow(dpy);
	otherwm();
	XSelectInput(dpy, root,
		SubstructureRedirectMask | KeyPressMask | KeyReleaseMask
	);
}

static int
xerr(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "sxwm: fatal error\nrequest code:%d\nerror code:%d",
			ee->request_code, ee->error_code);
	return 0;
	if (dpy && ee) return 0;
}

int
main(int ac, char **av)
{
	if (ac > 1) {
		if (strcmp(av[1], "-v") == 0 || strcmp(av[1], "--version") == 0)
			errx(0, "%s\n%s\n%s\n", SXWM_VERSION, SXWM_AUTHOR, SXWM_LICINFO);
		else
			errx(0, "usage:\n[-v || --version]: See the version of sxwm\n");
	}
	setup();
	run();
	return 0;
}
