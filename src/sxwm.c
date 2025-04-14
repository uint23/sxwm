#include "sxwm.h"
#include "util.h"

static void
run(void)
{
	XEvent xev;
	for (;;) {
		XNextEvent(dpy, &xev);
	}
}

void
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

int
main(int ac, char **av)
{
	if (ac > 1) {
		if (strcmp(av[1], "-v") == 0 || strcmp(av[1], "--version") == 0)
			errx(0, "%s\n%s\n%s\n", SXWM_VERSION, SXWM_AUTHOR, SXWM_LICINFO);
		else
			errx(0, "usage:\n[-v || --version]: See the version of sxwm\n");
	}
	run();
	dpy = XOpenDisplay(NULL);
	return 0;
}
