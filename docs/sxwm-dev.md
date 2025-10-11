# sxwm developer docs

## Table of Contents

* [Headers](#headers)
* [sxwm.c](#sxwmc)

  * [Function Table](#function-table)
  * [Functions](#functions)

## Headers

| Header                      | Job                                                             |
| --------------------------- | --------------------------------------------------------------- |
| `signal.h`                  | Signal handling functions and definitions                       |
| `unistd.h`                  | Standard POSIX functionality                                    |
| `X11/Xatom.h`               | Definitions for Atom types                                      |
| `X11/Xlib.h`                | Core X Window System protocol and routines for creating windows |
| `X11/Xproto.h`              | Protocol definitions for X Window System communication          |
| `X11/Xutil.h`               | Used for getting hints about windows                            |
| `X11/extensions/Xinerama.h` | Xinerama extension definitions for multi-monitor setups         |
| `X11/Xcursor/Xcursor.h`     | Modern cursor handling                                          |
| `linux/limits.h`            | Linux specific limits for various data types                    |
|                             |                                                                 |
| `defs.h`                    | Project structs, macros, constants                              |
| `extern.h`                  | External functions to call to control wm                        |
| `parser.h`                  | Config file parser components                                   |

## sxwm.c

### Function Table

| Name                                                                  | Params (short)                             | Returns  | Summary                                             |
| --------------------------------------------------------------------- | ------------------------------------------ | -------- | --------------------------------------------------- |
| [add_client](#add_client)                                             | (Window w, int ws)                         | Client * | Allocate, link, and initialize a managed client.    |
| [apply_fullscreen](#apply_fullscreen)                                 | (Client *c, Bool on)                       | void     | Enter or exit fullscreen and update EWMH.           |
| [centre_window](#centre_window)                                       | (void)                                     | void     | Center focused floating window on its monitor.      |
| [change_workspace](#change_workspace)                                 | (int ws)                                   | void     | Switch visible workspace; remap and retile.         |
| [check_parent](#check_parent)                                         | (pid_t p, pid_t c)                         | int      | Return c if p is an ancestor of c, else 0.          |
| [clean_mask](#clean_mask)                                             | (int mask)                                 | int      | Clear Lock, NumLock, Mode_switch bits.              |
| [close_focused](#close_focused)                                       | (void)                                     | void     | Send WM_DELETE or kill focused window.              |
| [dec_gaps](#dec_gaps)                                                 | (void)                                     | void     | Decrease gaps and retile.                           |
| [find_client](#find_client)                                           | (Window w)                                 | Client * | Find a client by top-level window.                  |
| [find_toplevel](#find_toplevel)                                       | (Window w)                                 | Window   | Walk up to the toplevel window.                     |
| [focus_next](#focus_next)                                             | (void)                                     | void     | Focus next mapped client on current monitor.        |
| [focus_prev](#focus_prev)                                             | (void)                                     | void     | Focus previous mapped client on current monitor.    |
| [focus_next_mon](#focus_next_mon)                                     | (void)                                     | void     | Focus first client on next monitor or warp cursor.  |
| [focus_prev_mon](#focus_prev_mon)                                     | (void)                                     | void     | Focus first client on prev monitor or warp cursor.  |
| [get_monitor_for](#get_monitor_for)                                   | (Client *c)                                | int      | Monitor index for client center point.              |
| [get_parent_process](#get_parent_process)                             | (pid_t c)                                  | pid_t    | Read /proc to get parent pid.                       |
| [get_pid](#get_pid)                                                   | (Window w)                                 | pid_t    | Read _NET_WM_PID from window.                       |
| [get_workspace_for_window](#get_workspace_for_window)                 | (Window w)                                 | int      | Resolve workspace by class rules.                   |
| [grab_button](#grab_button)                                           | (Mask btn, Mask mod, Window w, Bool, Mask) | void     | XGrabButton helper.                                 |
| [grab_keys](#grab_keys)                                               | (void)                                     | void     | Grab all configured keybindings.                    |
| [hdl_button](#hdl_button)                                             | (XEvent *e)                                | void     | ButtonPress handler (swap/move/resize/focus).       |
| [hdl_button_release](#hdl_button_release)                             | (XEvent *e)                                | void     | Finish swap or drag; ungrab pointer.                |
| [hdl_client_msg](#hdl_client_msg)                                     | (XEvent *e)                                | void     | Handle EWMH messages (desktop/state).               |
| [hdl_config_ntf](#hdl_config_ntf)                                     | (XEvent *e)                                | void     | Root ConfigureNotify: refresh monitors and retile.  |
| [hdl_config_req](#hdl_config_req)                                     | (XEvent *e)                                | void     | ConfigureRequest: honor for floating/fullscreen.    |
| [hdl_dummy](#hdl_dummy)                                               | (XEvent *e)                                | void     | No-op handler.                                      |
| [hdl_destroy_ntf](#hdl_destroy_ntf)                                   | (XEvent *e)                                | void     | Client destroy: unlink, unswallow, refocus.         |
| [hdl_keypress](#hdl_keypress)                                         | (XEvent *e)                                | void     | Dispatch keybinding actions.                        |
| [hdl_mapping_ntf](#hdl_mapping_ntf)                                   | (XEvent *e)                                | void     | Refresh keymap and regrab keys.                     |
| [hdl_map_req](#hdl_map_req)                                           | (XEvent *e)                                | void     | Classify new window, add_client, map, swallow, etc. |
| [hdl_motion](#hdl_motion)                                             | (XEvent *e)                                | void     | Pointer motion during drag; snap and clamp.         |
| [hdl_property_ntf](#hdl_property_ntf)                                 | (XEvent *e)                                | void     | React to _NET_CURRENT_DESKTOP, struts, state.       |
| [hdl_unmap_ntf](#hdl_unmap_ntf)                                       | (XEvent *e)                                | void     | Mark unmapped and refresh layout.                   |
| [inc_gaps](#inc_gaps)                                                 | (void)                                     | void     | Increase gaps and retile.                           |
| [init_defaults](#init_defaults)                                       | (void)                                     | void     | Populate default_config and copy to user_config.    |
| [is_child_proc](#is_child_proc)                                       | (pid_t parent, pid_t child)                | Bool     | Check PPID chain within a limit.                    |
| [move_master_next](#move_master_next)                                 | (void)                                     | void     | Rotate master to tail.                              |
| [move_master_prev](#move_master_prev)                                 | (void)                                     | void     | Move tail to master.                                |
| [move_next_mon](#move_next_mon)                                       | (void)                                     | void     | Move focused to next monitor.                       |
| [move_prev_mon](#move_prev_mon)                                       | (void)                                     | void     | Move focused to previous monitor.                   |
| [move_to_workspace](#move_to_workspace)                               | (int ws)                                   | void     | Send focused to ws and retile current.              |
| [move_win_down](#move_win_down)                                       | (void)                                     | void     | Nudge focused floating down.                        |
| [move_win_left](#move_win_left)                                       | (void)                                     | void     | Nudge focused floating left.                        |
| [move_win_right](#move_win_right)                                     | (void)                                     | void     | Nudge focused floating right.                       |
| [move_win_up](#move_win_up)                                           | (void)                                     | void     | Nudge focused floating up.                          |
| [other_wm](#other_wm)                                                 | (void)                                     | void     | Probe SubstructureRedirect to detect other WM.      |
| [other_wm_err](#other_wm_err)                                         | (Display *d, XErrorEvent *ee)              | int      | Print and exit if another WM is running.            |
| [parse_col](#parse_col)                                               | (const char *hex)                          | long     | Parse hex color and allocate pixel.                 |
| [quit](#quit)                                                         | (void)                                     | void     | Close display and cursors; stop main loop.          |
| [reload_config](#reload_config)                                       | (void)                                     | void     | Free config data, reparse, regrab, retile.          |
| [remove_scratchpad](#remove_scratchpad)                               | (int n)                                    | void     | Detach scratchpad n and remap its client.           |
| [resize_master_add](#resize_master_add)                               | (void)                                     | void     | Grow master width on focused monitor.               |
| [resize_master_sub](#resize_master_sub)                               | (void)                                     | void     | Shrink master width on focused monitor.             |
| [resize_stack_add](#resize_stack_add)                                 | (void)                                     | void     | Grow custom height for focused stack client.        |
| [resize_stack_sub](#resize_stack_sub)                                 | (void)                                     | void     | Shrink custom height for focused stack client.      |
| [resize_win_down](#resize_win_down)                                   | (void)                                     | void     | Resize focused floating window down.                |
| [resize_win_left](#resize_win_left)                                   | (void)                                     | void     | Resize focused floating window left.                |
| [resize_win_right](#resize_win_right)                                 | (void)                                     | void     | Resize focused floating window right.               |
| [resize_win_up](#resize_win_up)                                       | (void)                                     | void     | Resize focused floating window up.                  |
| [run](#run)                                                           | (void)                                     | void     | Main event loop.                                    |
| [reset_opacity](#reset_opacity)                                       | (Window w)                                 | void     | Remove window opacity property.                     |
| [scan_existing_windows](#scan_existing_windows)                       | (void)                                     | void     | Manage already viewable children on startup.        |
| [select_input](#select_input)                                         | (Window w, Mask masks)                     | void     | XSelectInput wrapper.                               |
| [send_wm_take_focus](#send_wm_take_focus)                             | (Window w)                                 | void     | Send WM_TAKE_FOCUS if supported.                    |
| [setup](#setup)                                                       | (void)                                     | void     | Init display, atoms, config, grabs, handlers.       |
| [setup_atoms](#setup_atoms)                                           | (void)                                     | void     | Intern atoms and publish EWMH on root.              |
| [set_frame_extents](#set_frame_extents)                               | (Window w)                                 | void     | Publish border widths via _NET_FRAME_EXTENTS.       |
| [set_input_focus](#set_input_focus)                                   | (Client *c, Bool raise, Bool warp)         | void     | Focus client and update EWMH active window.         |
| [set_opacity](#set_opacity)                                           | (Window w, double op)                      | void     | Write _NET_WM_WINDOW_OPACITY.                       |
| [set_win_scratchpad](#set_win_scratchpad)                             | (int n)                                    | void     | Assign focused to scratchpad slot n.                |
| [set_wm_state](#set_wm_state)                                         | (Window w, long state)                     | void     | Write ICCCM WM_STATE.                               |
| [snap_coordinate](#snap_coordinate)                                   | (int pos, int size, int scr, int snap)     | int      | Snap coord to edges within distance.                |
| [spawn](#spawn)                                                       | (const char * const *argv)                 | void     | Run pipeline argv split by "                        |
| [startup_exec](#startup_exec)                                         | (void)                                     | void     | Run autostart commands from config.                 |
| [swallow_window](#swallow_window)                                     | (Client *swallower, Client *swallowed)     | void     | Hide swallower and show child.                      |
| [swap_clients](#swap_clients)                                         | (Client *a, Client *b)                     | void     | Swap two list nodes in current workspace.           |
| [switch_previous_workspace](#switch_previous_workspace)               | (void)                                     | void     | Change to previous_workspace.                       |
| [tile](#tile)                                                         | (void)                                     | void     | Tiling layout with gaps and per-monitor master.     |
| [toggle_floating](#toggle_floating)                                   | (void)                                     | void     | Toggle focused floating state.                      |
| [toggle_floating_global](#toggle_floating_global)                     | (void)                                     | void     | Toggle all clients floating on/off.                 |
| [toggle_fullscreen](#toggle_fullscreen)                               | (void)                                     | void     | Toggle fullscreen on focused.                       |
| [toggle_monocle](#toggle_monocle)                                     | (void)                                     | void     | Toggle monocle layout.                              |
| [toggle_scratchpad](#toggle_scratchpad)                               | (int n)                                    | void     | Map/unmap scratchpad n and focus.                   |
| [unswallow_window](#unswallow_window)                                 | (Client *c)                                | void     | Restore swallower and unlink relation.              |
| [update_borders](#update_borders)                                     | (void)                                     | void     | Paint borders and publish active window.            |
| [update_client_desktop_properties](#update_client_desktop_properties) | (void)                                     | void     | Write _NET_WM_DESKTOP for every client.             |
| [update_modifier_masks](#update_modifier_masks)                       | (void)                                     | void     | Detect NumLock and Mode_switch masks.               |
| [update_mons](#update_mons)                                           | (void)                                     | void     | Query Xinerama and rebuild monitor array.           |
| [update_net_client_list](#update_net_client_list)                     | (void)                                     | void     | Publish client windows to _NET_CLIENT_LIST.         |
| [update_struts](#update_struts)                                       | (void)                                     | void     | Read dock struts and reserve monitor edges.         |
| [update_workarea](#update_workarea)                                   | (void)                                     | void     | Publish per-monitor workareas.                      |
| [warp_cursor](#warp_cursor)                                           | (Client *c)                                | void     | Move pointer to window center on root.              |
| [window_has_ewmh_state](#window_has_ewmh_state)                       | (Window w, Atom state)                     | Bool     | Test membership in _NET_WM_STATE.                   |
| [window_set_ewmh_state](#window_set_ewmh_state)                       | (Window w, Atom state, Bool add)           | void     | Add/remove EWMH state atom.                         |
| [window_should_float](#window_should_float)                           | (Window w)                                 | Bool     | Match should_float rules.                           |
| [window_should_start_fullscreen](#window_should_start_fullscreen)     | (Window w)                                 | Bool     | Match start_fullscreen rules.                       |
| [xerr](#xerr)                                                         | (Display *d, XErrorEvent *ee)              | int      | Ignore benign X errors.                             |
| [xev_case](#xev_case)                                                 | (XEvent *e)                                | void     | Dispatch via evtable by type.                       |
| [main](#main)                                                         | (int ac, char **av)                        | int      | CLI: -v/--version; else start WM.                   |

---

### Functions

Below, each entry shows the compact signature and a detailed description. Use anchors in the table to jump here.

#### add_client

```c
(Window w, int ws) -> Client *
```

> Returns NULL on allocation failure.

Create and register a client in workspace ws. Select inputs, grab mouse
buttons, set protocols (WM_DELETE_WINDOW), capture initial geometry, set
monitor by cursor position, set _NET_WM_DESKTOP, and raise the window.
Sets global focus if it is the first client in the current workspace.

#### apply_fullscreen

```c
(Client *c, Bool on) -> void
```

Enter: save geometry, clear floating, set fullscreen, remove borders,
resize to monitor bounds, raise, set EWMH state. Exit: restore geometry
and border, clear state, recompute monitor if tiled, then retile and
repaint borders.

#### centre_window

```c
(void) -> void
```

If the focused client exists, is mapped and floating, move it to the
center of its current monitor workarea and apply via XMoveWindow.

#### change_workspace

```c
(int ws) -> void
```

Abort if ws is invalid or already current. Grab server. Remember which
scratchpads are visible and unmap them. Unmap all non-scratchpad windows
in the current workspace. Switch current_ws, remap windows in the target
workspace, move visible scratchpads into the new workspace, and map them.
Retile, choose focus (prefer visible scratchpad, else first window on the
current monitor), set input focus, update _NET_CURRENT_DESKTOP and
per-client _NET_WM_DESKTOP. Ungrab server and sync. Maintain
previous_workspace and in_ws_switch flags.

#### check_parent

```c
(pid_t p, pid_t c) -> int
```

Walk the process tree via get_parent_process until p or 0 is reached.
Return c if p is an ancestor of c, else 0.

#### clean_mask

```c
(int mask) -> int
```

Strip LockMask, NumLock, and Mode_switch bits so key matching is stable.

#### close_focused

```c
(void) -> void
```

If focused exists, clear scratchpad binding if any. If the client supports
WM_DELETE_WINDOW, send a ClientMessage. Otherwise unmap and kill the
client. Layout updates are handled by subsequent notifications.

#### dec_gaps

```c
(void) -> void
```

If user_config.gaps > 0, decrement and retile, then repaint borders.

#### find_client

```c
(Window w) -> Client *
```

Linear search across all workspaces for a client with top-level window w.

#### find_toplevel

```c
(Window w) -> Window
```

Walk up the X window tree via XQueryTree until the root parent is reached
and return that top-level window id.

#### focus_next

```c
(void) -> void
```

From the current focus (or head), loop forward (wrap) to the next mapped
client on current_mon. If found, set focus and call set_input_focus.

#### focus_prev

```c
(void) -> void
```

From the current focus (or head), walk backward (wrap) to the previous
mapped client on current_mon. If found, set focus and call set_input_focus.

#### focus_next_mon

```c
(void) -> void
```

If multiple monitors, focus the first mapped, non-fullscreen client on the
next monitor. If none exist, warp the pointer to the next monitor center.

#### focus_prev_mon

```c
(void) -> void
```

Same as focus_next_mon but targeting the previous monitor.

#### get_monitor_for

```c
(Client *c) -> int
```

Compute the client center and return the monitor whose bounds contain it.
Fallback to 0 if none match.

#### get_parent_process

```c
(pid_t c) -> pid_t
```

Read /proc/%u/stat and parse the parent PID for process c. Return 0 on
failure.

#### get_pid

```c
(Window w) -> pid_t
```

Read _NET_WM_PID from the window as a 32-bit XA_CARDINAL and return it, or
0 if missing.

#### get_workspace_for_window

```c
(Window w) -> int
```

Check XClassHint against user_config.open_in_workspace rules. If a class
or name matches, return that workspace id. Else return current_ws.

#### grab_button

```c
(Mask button, Mask mod, Window w, Bool owner_events, Mask masks) -> void
```

Call XGrabButton with async mode on root and sync on non-root windows.

#### grab_keys

```c
(void) -> void
```

Ungrab all keys, then for each binding in user_config.binds, compute
keycode and grab with guard modifier combinations so bindings remain
usable with Lock, NumLock, or Mode_switch active.

#### hdl_button

```c
(XEvent *xev) -> void
```

ButtonPress handler. On tiled windows:

* Mod+Shift+Left: begin swap drag (DRAG_SWAP).
* Mod+Left or Mod+Right: toggle_floating to allow drag.
* Plain Left: focus the clicked window.

On floating windows, begin DRAG_MOVE (left) or DRAG_RESIZE (right),
recording drag origin and grabbing the pointer.

#### hdl_button_release

```c
(XEvent *xev) -> void
```

If in DRAG_SWAP and a target is set, swap clients, retile, and repaint.
Always ungrab the pointer and clear drag state.

#### hdl_client_msg

```c
(XEvent *xev) -> void
```

Handle _NET_CURRENT_DESKTOP by calling change_workspace. Handle
_NET_WM_STATE actions for _NET_WM_STATE_FULLSCREEN: add/remove/toggle via
apply_fullscreen and raise on entry.

#### hdl_config_ntf

```c
(XEvent *xev) -> void
```

On root ConfigureNotify, refresh monitors, retile, and repaint borders.

#### hdl_config_req

```c
(XEvent *xev) -> void
```

Honor ConfigureRequest for floating or fullscreen clients by forwarding
geometry. Ignore for tiled clients (layout is enforced by WM).

#### hdl_dummy

```c
(XEvent *xev) -> void
```

No operation.

#### hdl_destroy_ntf

```c
(XEvent *xev) -> void
```

Unlink the destroyed client. If it swallowed another, remap it. If it was
swallowed, remap the swallower. Pick a new focus on the same monitor if
possible. Update _NET_CLIENT_LIST, retile current workspace, repaint, and
refocus if applicable.

#### hdl_keypress

```c
(XEvent *xev) -> void
```

Match keycode+mods (cleaned) against user_config.binds and dispatch:

* TYPE_CMD: spawn argv
* TYPE_FUNC: call function
* TYPE_WS_CHANGE / TYPE_WS_MOVE
* TYPE_SP_REMOVE / TYPE_SP_TOGGLE / TYPE_SP_CREATE
  Update _NET_CLIENT_LIST when workspace topology changes.

#### hdl_mapping_ntf

```c
(XEvent *xev) -> void
```

Refresh keyboard mapping, detect modifier masks, and regrab keys.

#### hdl_map_req

```c
(XEvent *xev) -> void
```

Ignore invisible or override-redirect windows. If already managed and on
current workspace, ensure mapped and optionally focus. Otherwise:
classify window type, decide floating or tiled (consider utility/dialog,
modal, size hints, transient), enforce max clients, choose target
workspace via rules, call add_client, set WM_STATE, center floating, set
borders, attempt swallowing (class rules + parent PID), honor requested
fullscreen, map if on current workspace, and update borders/focus.

#### hdl_motion

```c
(XEvent *xev) -> void
```

Throttle by motion_throttle. Identify monitor under pointer. For DRAG_SWAP
track hovered tiled target and show swap border. For DRAG_MOVE compute new
position, apply snapping inside monitor workarea, auto-toggle to floating
if moved far enough, and move window. For DRAG_RESIZE grow from bottom
right and clamp to monitor workarea.

#### hdl_property_ntf

```c
(XEvent *xev) -> void
```

On root:

* _NET_CURRENT_DESKTOP: read and change_workspace.
* _NET_WM_STRUT_PARTIAL: update_struts, retile, repaint.

On client:

* _NET_WM_STATE: mirror fullscreen flag using apply_fullscreen.

#### hdl_unmap_ntf

```c
(XEvent *xev) -> void
```

If not switching workspaces, mark the client as unmapped. Update client
list, retile, and repaint.

#### inc_gaps

```c
(void) -> void
```

Increment gaps and retile, then repaint borders.

#### init_defaults

```c
(void) -> void
```

Fill default_config (modkey, gaps, border colors via parse_col, movement
steps, per-monitor master widths, rule arrays, throttles, snap distance,
and flags). Copy to user_config.

#### is_child_proc

```c
(pid_t parent_pid, pid_t child_pid) -> Bool
```

Walk PPID chain up to a limit using /proc. Return True if parent_pid is
found. Print simple diagnostics on failures.

#### move_master_next

```c
(void) -> void
```

Rotate the head node to the tail of the current workspace list. Retile,
optionally warp to the old focused window, send WM_TAKE_FOCUS, repaint.

#### move_master_prev

```c
(void) -> void
```

Move the tail node to head (becomes new master). Retile, optional warp,
send focus, repaint.

#### move_next_mon

```c
(void) -> void
```

If focused and multiple monitors, reassign focused to next monitor. If
floating, center and clamp inside the target monitor. Retile and optionally
warp cursor. Repaint.

#### move_prev_mon

```c
(void) -> void
```

Same as move_next_mon but previous monitor.

#### move_to_workspace

```c
(int ws) -> void
```

Unmap focused, unlink from current list, push to target workspace head,
update _NET_WM_DESKTOP, retile current workspace, and refocus its head.

#### move_win_down

```c
(void) -> void
```

For a focused floating client, move by move_window_amt down via XMoveWindow.

#### move_win_left

```c
(void) -> void
```

For a focused floating client, move left by move_window_amt.

#### move_win_right

```c
(void) -> void
```

For a focused floating client, move right by move_window_amt.

#### move_win_up

```c
(void) -> void
```

For a focused floating client, move up by move_window_amt.

#### other_wm

```c
(void) -> void
```

Temporarily set error handler to other_wm_err, attempt to select
SubstructureRedirectMask on root to detect another WM, then restore.

#### other_wm_err

```c
(Display *d, XErrorEvent *ee) -> int
```

> Always exits with failure.

Print a message and exit because another WM has the redirect. Returns 0
but does not reach caller.

#### parse_col

```c
(const char *hex) -> long
```

Parse and allocate an XColor from a hex string. On failure, return the
default white pixel. OR 0xff into the high byte to set an alpha-like
component and return the pixel.

#### quit

```c
(void) -> void
```

Close display and free cursors. Print a message and clear running flag.
Optional mass-kill code is commented out.

#### reload_config

```c
(void) -> void
```

Free binding commands and rule arrays, wipe user_config, re-init defaults,
re-parse config (fallback to defaults on error), regrab keys and buttons
on root and clients, rebuild root grabs, update desktop props and client
list, sync, retile, and repaint.

#### remove_scratchpad

```c
(int n) -> void
```

If scratchpad slot n holds a client, remap it and clear the slot. Update
client list and borders.

#### resize_master_add

```c
(void) -> void
```

Increase user_config.master_width for the focused monitor by
resize_master_amt (bounded). Retile and repaint.

#### resize_master_sub

```c
(void) -> void
```

Decrease user_config.master_width for the focused monitor by
resize_master_amt (bounded). Retile and repaint.

#### resize_stack_add

```c
(void) -> void
```

If focused is a stack client (tiled and not master), increase its
custom_stack_height by resize_stack_amt and retile.

#### resize_stack_sub

```c
(void) -> void
```

If focused is a stack client, decrease custom_stack_height (bounded to a
minimum) and retile.

#### resize_win_down

```c
(void) -> void
```

If focused is floating, grow height by resize_window_amt, clamp to monitor
workarea, and apply via XResizeWindow.

#### resize_win_left

```c
(void) -> void
```

If focused is floating, shrink width by resize_window_amt (bounded), and
apply.

#### resize_win_right

```c
(void) -> void
```

If focused is floating, grow width by resize_window_amt (bounded), and
apply.

#### resize_win_up

```c
(void) -> void
```

If focused is floating, shrink height by resize_window_amt (bounded), and
apply.

#### run

```c
(void) -> void
```

Set running flag and loop on XNextEvent, dispatching through xev_case
until running is cleared.

#### reset_opacity

```c
(Window w) -> void
```

Delete _NET_WM_WINDOW_OPACITY property to restore default compositor
opacity.

#### scan_existing_windows

```c
(void) -> void
```

Query root children. For each viewable, non-override-redirect child,
synthesize a MapRequest and feed it to hdl_map_req so sxwm begins
managing existing windows on startup.

#### select_input

```c
(Window w, Mask masks) -> void
```

Wrapper for XSelectInput.

#### send_wm_take_focus

```c
(Window w) -> void
```

If the window supports WM_TAKE_FOCUS, send the ClientMessage so clients
that require it can accept focus.

#### setup

```c
(void) -> void
```

Open display, set root, setup_atoms, probe for other WM, load defaults and
parse config, compute modifier masks, grab keys, run autostart, load and
set cursors, cache screen size, update monitors, select root events,
grab root mouse buttons, initialize event table, scan existing windows,
and ignore SIGCHLD to prevent zombies.

#### setup_atoms

```c
(void) -> void
```

Intern all required EWMH/ICCCM atoms, create _NET_SUPPORTING_WM_CHECK,
publish number of desktops, names, current desktop, supported atom list
on root, and compute initial workarea.

#### set_frame_extents

```c
(Window w) -> void
```

Write border width on all four sides to _NET_FRAME_EXTENTS.

#### set_input_focus

```c
(Client *c, Bool raise_win, Bool warp) -> void
```

If c is mapped: set global focus, call XSetInputFocus on toplevel, send
WM_TAKE_FOCUS, optionally raise if floating, publish _NET_ACTIVE_WINDOW,
repaint borders, and optionally warp pointer. If c is NULL: focus root
and clear _NET_ACTIVE_WINDOW. Flush.

#### set_opacity

```c
(Window w, double opacity) -> void
```

Clamp opacity to [0.0, 1.0], convert to 32-bit fixed point, and write
_NET_WM_WINDOW_OPACITY.

#### set_win_scratchpad

```c
(int n) -> void
```

Assign focused window to scratchpad slot n by storing and unmapping it.
If a different client was already in the slot, remap it and clear the
slot first.

#### set_wm_state

```c
(Window w, long state) -> void
```

Write ICCCM WM_STATE with state and None icon window.

#### snap_coordinate

```c
(int pos, int size, int screen_size, int snap_dist) -> int
```

Snap left edge to 0 or right edge to screen_size when within snap_dist,
otherwise return pos unchanged.

#### spawn

```c
(const char * const *argv) -> void
```

Split argv by "|" into N commands, create N-1 pipes, fork N children,
wire stdin/stdout with dup2, execvp each stage, close fds in parent, and
free temporary allocations. Close the X connection fd in children.

#### startup_exec

```c
(void) -> void
```

For each non-NULL user_config.to_run[i], build argv, call spawn, and free
argv pieces.

#### swallow_window

```c
(Client *swallower, Client *swallowed) -> void
```

If eligible and not already linked: unmap swallower, link both ways, copy
geometry/floating state, move/resize swallowed if floating, retile and
repaint.

#### swap_clients

```c
(Client *a, Client *b) -> void
```

Swap two nodes in the singly-linked list of the current workspace,
handling the adjacent case and the general case.

#### switch_previous_workspace

```c
(void) -> void
```

Call change_workspace(previous_workspace).

#### tile

```c
(void) -> void
```

Recompute struts, then for each monitor: build list of visible, non-
floating, non-fullscreen clients on that monitor. If monocle layout enabled, configure
every window to take all the space then leave. Otherwise, places master at left
with width master_width[m], stack on right with gaps. Compute stack
heights using per-client custom_stack_height or auto-split; enforce
minimums and absorb rounding remainder at bottom. Configure windows,
optionally raise floating windows, and repaint borders. Skip layout if a
single fullscreen client exists.

#### toggle_floating

```c
(void) -> void
```

Toggle focused floating state. If leaving fullscreen, clear it and restore
borders. When entering floating, capture current server-side geometry.
When leaving floating, recompute monitor. Retile, repaint, and if floating,
raise and refocus.

#### toggle_floating_global

```c
(void) -> void
```

Flip global_floating. If any tiled exists, set all to floating (storing
current geometry) and raise them; otherwise leave state. Retile and repaint.

#### toggle_fullscreen

```c
(void) -> void
```

Toggle fullscreen on focused via apply_fullscreen.

#### toggle_monocle

```c
(void) -> void
```

Enable/disable the monocle layout. Most of the logic is near the top of [tile](#tile)

#### toggle_scratchpad

```c
(int n) -> void
```

Move scratchpad n into current workspace if needed. Update _NET_WM_DESKTOP,
set its monitor to the focused monitor, and map or unmap. On mapping,
focus it. Retile, repaint, and update client list.

#### unswallow_window

```c
(Client *c) -> void
```

If c has a swallower, unlink the relation, remap the swallower, focus it,
then retile and repaint.

#### update_borders

```c
(void) -> void
```

Set border pixel for each client on the current workspace to focused or
unfocused color. Publish _NET_ACTIVE_WINDOW with focused id if any.

#### update_client_desktop_properties

```c
(void) -> void
```

For every client in every workspace, write its _NET_WM_DESKTOP value.

#### update_modifier_masks

```c
(void) -> void
```

Read X modifier mapping; detect which modifier bit holds Num_Lock and
Mode_switch keycodes. Update numlock_mask and mode_switch_mask.

#### update_mons

```c
(void) -> void
```

Free old monitor array, query Xinerama for screens if active, else fall
back to a single monitor covering the display. Define cursor for each
screen root.

#### update_net_client_list

```c
(void) -> void
```

Flatten all client windows and write _NET_CLIENT_LIST on root.

#### update_struts

```c
(void) -> void
```

Reset reserves on monitors. For each DOCK window, read
_NET_WM_STRUT_PARTIAL and expand per-monitor reserves based on the dock
location. Update workarea afterward.

#### update_workarea

```c
(void) -> void
```

Publish per-monitor workareas (x, y, w, h) to _NET_WORKAREA.

#### warp_cursor

```c
(Client *c) -> void
```

Move pointer to the window center on root and sync.

#### window_has_ewmh_state

```c
(Window w, Atom state) -> Bool
```

Read _NET_WM_STATE and test for membership of state.

#### window_set_ewmh_state

```c
(Window w, Atom state, Bool add) -> void
```

Read existing _NET_WM_STATE, rebuild the list without state, and append
state if add is True. Write the result or delete the property if empty.

#### window_should_float

```c
(Window w) -> Bool
```

Check XClassHint and match name/class against user_config.should_float.

#### window_should_start_fullscreen

```c
(Window w) -> Bool
```

Check XClassHint and match name/class against user_config.start_fullscreen.

#### xerr

```c
(Display *d, XErrorEvent *ee) -> int
```

Ignore common benign X errors (BadWindow, BadDrawable, BadMatch) and
return 0.

#### xev_case

```c
(XEvent *xev) -> void
```

If type is in range, dispatch to evtable[type]. Otherwise print an error.

#### main

```c
(int ac, char **av) -> int
```

> EXIT_SUCCESS

If `-v` or `--version`, print version, author, and license info. Otherwise
call setup(), print "sxwm: starting...", then run() and return success.

---
