#ifndef USER_CONFIG
#define USER_CONFIG

#include <X11/keysym.h>
#include "defs.h"

#define BORDER_WIDTH	1
#define BORDER_FOC_COL	"#AAFFFA" // the border color when focused
#define BORDER_UFOC_COL	"#FF4439" // the border color when unfocused
#define GAPS			10 // how many pixels wide the border is
#define MASTER_WIDTH	0.6	// how much of the screen the master window takes up (0.0-1.0)
#define MOTION_THROTTLE	144 // set this to your screen refreshrate
#define SNAP_DISTANCE	5	// snap distance

static const char *termcmd[] = {"st", NULL};

#define MOD				ALT
static const Binding binds[] = {
	BIND(MOD, 			Return,		termcmd),
	CALL(MOD|SHIFT,		e,			quit),
	CALL(MOD, 			j, 			focus_next),
	CALL(MOD, 			k, 			focus_prev),
	CALL(MOD|SHIFT,		q,			close_focused),
	CALL(MOD,			f,			toggle_floating),
	CALL(MOD,			equal,		inc_gaps),
	CALL(MOD,			minus,		dec_gaps),
};

#endif
