/*< You can ignore this >*/
#include <X11/keysym.h>
#include "defs.h"

/*
 * ——————————————< Appearance >—————————————— *
 *
 *	   In this section you can configure the
 *	   settings for sxwm. You can ignore the
 *	   #define as it is C specific syntax
 *
 *	   GAPS (px):
 *	   How many pixels between windows and
 *	   screen edges (including bar)
 *	   BORDER_WIDTH (px):
 *	   How thick your border is
 *
 *	   BORDER_FOC_COL (hex):
 *	   The colour of your border when the
 *	   window is focused
 *
 *	   BORDER_UFOC_COL (hex):
 *	   The colour of your border when the
 *	   window is unfocused
 *
 *	   MASTER_WIDTH (float):
 *	   % of the screen the master window
 *	   should take as a decimal value 0-1
 *
 *	   MOTION_THROTTLE (int):
 *	   Usually you should set this to your
 *	   screen refreshrate. This is set so
 *	   there is no exessive number of
 *	   requests being sent to the X server
 *
 *	   SNAP_DISTANCE (px):
 *	   How many pixels away from the screen
 *	   until the window *snaps* to the edge
 *
 * ———————————————————————————————————————————*
*/

#define GAPS			10
#define BORDER_WIDTH	5
#define BORDER_FOC_COL	"#AAFFFA"
#define BORDER_UFOC_COL	"#FF4439"
#define MASTER_WIDTH	0.6
#define MOTION_THROTTLE	144
#define SNAP_DISTANCE	10

/*
 * ———————————< Keys & Bindins >————————————— *
 *
 *	   This is where you set your keybinds to
 *	   open apps, kill apps, perform in-built
 *	   functions.
 *
 *	   How to make a command to run an app:
 *
 *	   1) Just write this:
 *	      'static const char *'
 *	   	  you don't have to understand this,
 *	   	  this is just how you make a string
 *	   	  in C
 *
 *	   2) Call it what the app is called or
 *	      anything you want really, eg. the
 *	      command to open a terminal is
 *	      termcmd. Also append a '[]' at the
 *	      end to show its an array. This is
 *	      for use of arguments eg 'ls -lah'.
 *
 *	   3) Construct the command by putting '='
 *	      then after that open '{' then put
 *	      the first arg in "arg0",. Repeat for
 *	      all your args and end it with NULL.
 *	      Strings in C are ended with a NULL
 *	      'terminator' which tells the program
 *	      that that is the end of the string.
 *
 *	   4) Finally, close the '}', then add
 *	   	  a semi-colon at the end.
 *
 *	   After doing all that, you should have
 *	   something like this:
 *
 *	   static const char *app = { "app", NULL };
 *
 * ——————————— —————————————————————————————— *
 *
*/

static const char *termcmd[] = 		{ "st", NULL };
static const char *browsercmd[] = 	{ "firefox", NULL };

/*< This is your modifier key (MOD/SUPER) >*/
#define MOD	ALT

/*
 * ———————————————< Bindings >————————————————*
 *
 *     This is where you assign keybinds to
 *     perform some actions.
 *
 *
 * ——————————— —————————————————————————————— *
*/

static const Binding binds[] =
{
/*		 Modifier(s)	Key			Function	*/
	CALL(MOD|SHIFT,		e,			quit),
	CALL(MOD|SHIFT,		q,			close_focused),

	CALL(MOD, 			j, 			focus_next),
	CALL(MOD, 			k, 			focus_prev),

	CALL(MOD|SHIFT, 	j, 			move_master_next),
	CALL(MOD|SHIFT, 	k, 			move_master_prev),

	CALL(MOD,			equal,		inc_gaps),
	CALL(MOD,			minus,		dec_gaps),

	CALL(MOD,			f,			toggle_floating),
	CALL(MOD,			space,		toggle_floating_global),

/*		Here are your executable functions 		*/
	BIND(MOD, 			Return,		termcmd),
	BIND(MOD,			b,			browsercmd),
};
