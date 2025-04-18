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
 *	   execute apps. You can use the CMD macro
 *	   to make new variables.
 *
 *	   How do you make a command to run an app
 *	   It's simple! Just do this:
 *
 *	   CALL(appcallname, "app", "arg2", ...);
 *
 *	   What is appcallname? This is just the
 *	   variable name given to this string of
 *	   commands given to execvp, the function
 *	   that executes these programs.
 *
 * ——————————— —————————————————————————————— *
 *
*/

CMD(terminal, 	"st");
CMD(browser, 	"firefox");

/*< This is your modifier key (MOD/SUPER) >*/
#define MOD	ALT

/*
 * ———————————————< Bindings >————————————————*
 *
 *     This is where you assign keybinds to
 *     perform some actions.
 *
 *     How do you bind keys? In sxwm, there is
 *     two ways to bind keys to perform tasks,
 *     BIND, or CALL. CALL, calls a function
 *     whereas BIND, executes a specified
 *     program.
 *
 *     USEAGE:
 *     	 BIND(MODIFIERS, KEY, FUNCTION)
 *
 *     	 MODIFIERS:
 *     	 The mod keys you want held down
 *     	 for the task to execute. I have
 *     	 also defined SHIFT as a substitute
 *     	 for ShiftMask.
 *
 *     	 KEY:
 *     	 The key to press in combination
 *     	 with the MODIFERS to run the task.
 *
 *     	 FUNCTION:
 *     	 The task to execute. Depending on
 *     	 whether you're calling CALL or
 *     	 BIND, this will execute a program
 *     	 or call a function.
 *
 *     	 If you're
 *     	 calling a function, just put the
 *     	 name of the funtion.
 *
 *     	 Otherwise, put the program you
 *     	 either defined with the CMD above
 *     	 or you can skip that step and just
 *     	 do something like this to create a
 *     	 "string" in the bindings array:
 *
 *     	 { "program", "arg1", NULL }
 *
 *     End the line with a comma, as this is
 *     an array.
 *
 * ——————————— —————————————————————————————— *
*/

static const Binding binds[] =
{
/*——< MODIFIER(S) >———  < KEY >—————< FUNCTION >——*/

/*———  ——< Here are your functions calls > ———— — */
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

/*—————< Here are your executable functions >—————*/
	BIND(MOD, 			Return,		terminal),
	BIND(MOD,			b,			browser),
};
