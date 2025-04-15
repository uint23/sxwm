#ifndef USER_CONFIG
#define USER_CONFIG

#include <X11/keysym.h>
#include "defs.h"

#define MOD	ALT

static const char *termcmd[] = {"st", NULL};

static const Binding binds[] = {
	BIND(MOD, 			Return,		termcmd),
	CALL(MOD|SHIFT,		q,			quit),
};

#endif
