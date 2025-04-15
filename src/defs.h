#ifndef DEFS_H
#define DEFS_H

#include <X11/Xlib.h>

#define SXWM_VERSION    "sxwm ver. 0.1.0"
#define SXWM_AUTHOR     "(C) Abhinav Prasai 2025"
#define SXWM_LICINFO    "See LICENSE for more info"

#define ALT     Mod1Mask
#define SUPER   Mod4Mask
#define SHIFT   ShiftMask

#define BIND(mod, key, cmdstr)  { (mod), XK_##key, { cmdstr }, 0 }
#define CALL(mod, key, fnptr)   { (mod), XK_##key, { .fn = fnptr }, 1 }

#define MAXCLIENTS	64

typedef void
(*EventHandler)(XEvent *);

typedef union {
    const char **cmd;
    void (*fn)(void);
} Action;

typedef struct {
    unsigned int mods;
    KeySym keysym;
    Action action;
    int is_func;
} Binding;

typedef struct {
    Window id;
    int x, y;
    unsigned int w, h;
    unsigned int bw;
    Bool isfocused;
    Bool isfloating;
} Client;

#endif
