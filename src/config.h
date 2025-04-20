/* See LICENSE for more information on use */

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
 *	   eg. 0.5 is 50%
 *	   
 *	   RESIZE_MASTER_AMT (%):
 *	   % of the master width you want to
 *	   increment by
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
 *	   NUM_WORKSPACES (int):
 *	   This is how many workspaces you want in
 *	   this window manager. Best to leave it
 *	   default (9).
 *
 *	   WORKSPACE_NAMES (char[]):
 *	   This is just the label that will appear
 *	   on your status bar. Doesn't have to be
 *	   a number, it can be anything. Ignore
 *	   the "\0", this is just a NULL that says
 *	   to start again, otherwise it would be
 *	   all of them concatenated together.
 *
 * ———————————————————————————————————————————*
*/

#define GAPS				10

#define BORDER_WIDTH		1
#define BORDER_FOC_COL		"#005577"
#define BORDER_UFOC_COL		"#444444"

#define MASTER_WIDTH		0.6
#define RESIZE_MASTER_AMT	1
#define MOTION_THROTTLE		60
#define SNAP_DISTANCE		5

#define NUM_WORKSPACES		9
#define WORKSPACE_NAMES		\
	"1"					"\0"\
	"2"					"\0"\
	"3"					"\0"\
	"4"					"\0"\
	"5"					"\0"\
	"6"					"\0"\
	"7"					"\0"\
	"8"					"\0"\
	"9"					"\0"\

/*
 * ————————————< Keys & Bindins >—————————————*
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
 * ———————————————————————————————————————————*
 *
*/

CMD(terminal,	"st");
CMD(browser,	"firefox");

/*
 * ———————————————< Bindings >————————————————*
 *
 *     This is where you assign keybinds to
 *     perform some actions.
 *
 *     How do you bind keys? In sxwm, there is
 *     three ways to bind keys to perform
 *     tasks:
 *
 *     BIND, CALL or WORKSPACE.
 *     CALL, calls a function,
 *     BIND, executes a specified
 *     program.
 *     WORKSPACE, sets the bind to move
 *     and item to said workspace or to
 *     change to that workspace.
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
 * ———————————————————————————————————————————*
*/

/*< This is your modifier key (ALT/SUPER) >*/
#define MOD	ALT

#include <X11/keysym.h>
static const Binding binds[] =
{
/*————< MODIFIER(S) >< KEY >—————< FUNCTION >——*/

/*———————< Here are your functions calls >————— — */

	CALL(MOD|SHIFT,		e,			quit),
	CALL(MOD|SHIFT,		q,			close_focused),

	CALL(MOD, 			j, 			focus_next),
	CALL(MOD, 			k, 			focus_prev),

	CALL(MOD|SHIFT, 	j, 			move_master_next),
	CALL(MOD|SHIFT, 	k, 			move_master_prev),

	CALL(MOD, 			l, 			resize_master_add),
	CALL(MOD,		 	h, 			resize_master_sub),

	CALL(MOD,			equal,		inc_gaps),
	CALL(MOD,			minus,		dec_gaps),

	CALL(MOD,			space,		toggle_floating),
	CALL(MOD|SHIFT,		space,		toggle_floating_global),

	CALL(MOD|SHIFT,		f,			toggle_fullscreen),

/*—————< Here are your executable functions >—————*/

	BIND(MOD, 			Return,		terminal),
	BIND(MOD,			b,			browser),

/*—————< This is for workspaces >—————————————————*/

	CALL(MOD,			1,			change_ws1),
	CALL(MOD|SHIFT,		1,			moveto_ws1),

	CALL(MOD,			2,			change_ws2),
	CALL(MOD|SHIFT,		2,			moveto_ws2),

	CALL(MOD,			3,			change_ws3),
	CALL(MOD|SHIFT,		3,			moveto_ws3),

	CALL(MOD,			4,			change_ws4),
	CALL(MOD|SHIFT,		4,			moveto_ws4),

	CALL(MOD,			5,			change_ws5),
	CALL(MOD|SHIFT,		5,			moveto_ws5),

	CALL(MOD,			6,			change_ws6),
	CALL(MOD|SHIFT,		6,			moveto_ws6),

	CALL(MOD,			7,			change_ws7),
	CALL(MOD|SHIFT,		7,			moveto_ws7),

	CALL(MOD,			8,			change_ws8),
	CALL(MOD|SHIFT,		8,			moveto_ws8),

	CALL(MOD,			9,			change_ws9),
	CALL(MOD|SHIFT,		9,			moveto_ws9),

};
