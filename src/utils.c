#include <err.h>
#include <X11/Xlib.h>
#include "util.h"

void
otherwm(void)
{
	XSetErrorHandler(otherwmerr);
	XChangeWindowAttributes(dpy, root, CWEventMask, 
			&(XSetWindowAttributes){.event_mask = SubstructureRedirectMask});
	XSync(dpy, False);
	XSetErrorHandler(xerr);
	XSync(dpy, False);
}

int
otherwmerr(Display *dpy, XErrorEvent *ee)
{
	errx(0, "sxwm: another window manager is already running, please close it");
	return 0;
	if (dpy && ee) return 0;
}

int
xerr(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "sxwm: fatal error\nrequest code:%d\nerror code:%d",
			ee->request_code, ee->error_code);
	return 0;
	if (dpy && ee) return 0;
}
