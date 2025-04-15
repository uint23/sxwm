#ifndef USER_CONFIG
#define USER_CONFIG

#include <X11/keysym.h>
#include "defs.h"

#define BORDER_WIDTH	2
#define BORDER_FOC_COL	"#00FF00"
#define BORDER_UFOC_COL	"#FF0000"

static const char *termcmd[] = {"st", NULL};

#define MOD				ALT
static const Binding binds[] = {
	BIND(MOD, 			Return,		termcmd),
	CALL(MOD|SHIFT,		q,			quit),
};

#endif
