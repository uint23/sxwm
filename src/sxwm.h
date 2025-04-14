#ifndef SXWM_H
#define SXWM_H

#include <string.h>
#include <err.h>
#include <X11/Xlib.h>

#define SXWM_VERSION    "sxwm ver. 0.1.0"
#define SXWM_AUTHOR     "(C) Abhinav Prasai 2025"
#define SXWM_LICINFO    "See LICENSE for more info"

#define ALT     Mod1Mask
#define SUPER   Mod4Mask
#define SHIFT   ShiftMask

static void run(void);
static void setup(void);

static Display 	*dpy;
static Window	root;
#endif
