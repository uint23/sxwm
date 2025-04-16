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
static void grab_keys(void);
static void hdl_dummy(XEvent *xev);
static void hdl_destroy_ntf(XEvent *xev);
static void hdl_keypress(XEvent *xev);
static void hdl_map_req(XEvent *xev);
static void other_wm(void);
static int other_wm_err(Display *dpy, XErrorEvent *ee);
static uint64_t parse_col(const char *hex);
static void quit(void);
static void remove_client(Window w);
static void run(void);
static void setup(void);
static void spawn(const char **cmd);
static void tile(void);
static int xerr(Display *dpy, XErrorEvent *ee);
static void xev_case(XEvent *xev);

static Client *clients = NULL;
static EventHandler evtable[LASTEvent];
static Display *dpy;
static Window root;

static uint64_t border_foc_col;
static uint64_t border_ufoc_col;
static uint scr_width;
static uint scr_height;

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
	++open_windows;

	XSelectInput(dpy, w, EnterWindowMask | LeaveWindowMask | FocusChangeMask |
			PropertyChangeMask | StructureNotifyMask);

	XRaiseWindow(dpy, w);

}

static uint
clean_mask(uint mask)
{
	return mask & ~(LockMask | Mod2Mask | Mod3Mask);
}

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
hdl_dummy(XEvent *xev)
{
	(void) xev;
}

static void
hdl_destroy_ntf(XEvent *xev)
{
	remove_client(xev->xdestroywindow.window);
	tile();
}

static void
hdl_map_req(XEvent *xev)
{
	XConfigureRequestEvent *config_req = &xev->xconfigurerequest;
	add_client(config_req->window);
	tile();
	XMapWindow(dpy, config_req->window);
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
	errx(0, "quitting...");
}

static void
remove_client(Window w)
{
    Client **curr = &clients; // Current window
    while (*curr) {
        if ((*curr)->win == w) {
            Client *tmp = *curr;
            *curr = (*curr)->next;
            free(tmp);
            open_windows--;
            break;
        }
        curr = &(*curr)->next;
    }
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
	other_wm();
	grab_keys();

	scr_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	scr_height = XDisplayHeight(dpy, DefaultScreen(dpy));
	XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

	for (int i = 0; i < LASTEvent; ++i)
		evtable[i] = hdl_dummy;

	evtable[DestroyNotify] = hdl_destroy_ntf;
	evtable[KeyPress] = hdl_keypress;
	evtable[MapRequest] = hdl_map_req;

	border_foc_col = parse_col(BORDER_FOC_COL);
	border_ufoc_col = parse_col(BORDER_UFOC_COL);
}

static void
spawn(const char **cmd)
{
	if (!cmd)
		return;

	if (fork() == 0) {
		setsid();
		execvp(cmd[0], (char *const*)cmd);
		errx(1, "sxwm: execvp '%s' failed\n", cmd[0]);
	} else {
		fprintf(stderr, "sxwm: falied to fork proc %s", cmd[0]);
	}
}

static void
tile(void)
{
    if (!open_windows)
        return;

    int masterX = GAPS + BORDER_WIDTH,
        masterY = GAPS + BORDER_WIDTH,
        availableH = scr_height - (GAPS * 2),
        masterW, masterH, stackW = 0, stackWinH = 0,
        stackCount = open_windows - 1;

    if (open_windows == 1) {
        masterW = scr_width - (GAPS * 2 + BORDER_WIDTH * 2);
        masterH = availableH - (BORDER_WIDTH * 2);
    } else {
        int totalGapsW = GAPS * 4, totalBordersW = BORDER_WIDTH * 4;
        masterW = (scr_width - totalGapsW - totalBordersW) / 2;
        stackW = masterW;
        masterH = availableH - (BORDER_WIDTH * 2);

        int totalGapsH = (stackCount > 0 ? GAPS * (stackCount - 1) : 0),
            totalBordersH = BORDER_WIDTH * 2 * stackCount,
            totalStackH = availableH - totalGapsH - totalBordersH;
        stackWinH = (stackCount > 0 ? totalStackH / stackCount : 0);
    }

    int stackX = masterX + masterW + GAPS + (BORDER_WIDTH * 2),
        stackY = GAPS;
    Client *c = clients;
    uint i = 0;

    for (; c; c = c->next, ++i) {
        XWindowChanges changes = { .border_width = BORDER_WIDTH };
        if (i == 0) {
            changes.x = masterX;
            changes.y = masterY;
            changes.width = masterW;
            changes.height = masterH;
        } else {
            changes.x = stackX;
            changes.y = stackY + BORDER_WIDTH; // adjust for border
            changes.width = stackW;
            changes.height = stackWinH;
            if (i == open_windows - 1) {
                int used = stackY - GAPS + stackWinH + (BORDER_WIDTH * 2);
                changes.height += (availableH - used);
            }
            stackY += stackWinH + (BORDER_WIDTH * 2) + GAPS;
        }
        XSetWindowBorder(dpy, c->win, border_foc_col);
        XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &changes);
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
