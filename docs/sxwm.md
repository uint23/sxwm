## Launch Arguments

### `-v` or `--version`
Displays the current version of `sxwm`


## Configuration

`sxwm` is configured via a simple text file located at `~/.config/sxwmrc`. Changes can be applied instantly by reloading the configuration (`MOD + r`).

The file uses a `key : value` format. Lines starting with `#` are ignored.

### General Options

| Option                   | Type    | Default   | Description                                                                 |
|--------------------------|---------|-----------|-----------------------------------------------------------------------------|
| `mod_key`                | String  | `super`   | Sets the primary modifier key (`alt`, `super`, `ctrl`).                     |
| `gaps`                   | Integer | `10`      | Pixels between windows and screen edges.                                    |
| `border_width`           | Integer | `1`       | Thickness of window borders in pixels.                                      |
| `focused_border_colour`  | Hex     | `#c0cbff` | Border color for the currently focused window.                              |
| `unfocused_border_colour`| Hex     | `#555555` | Border color for unfocused windows.                                         |
| `swap_border_colour`     | Hex     | `#fff4c0` | Border color when selecting a window to swap (`MOD+Shift+Drag`).            |
| `master_width`           | Integer | `60`      | Percentage of the screen width for the master window.                       |
| `motion_throttle`        | Integer | `60`      | Target FPS for mouse drag actions.                                          |
| `resize_master_amount`   | Integer | `1`       | Percent to increase/decrease master width.                                  |
| `resize_stack_amount`    | Integer | `20`      | How many pixels to increase/decrease stack windows by.                      |
| `snap_distance`          | Integer | `5`       | Distance (px) before a floating window snaps to edge.                       |
| `move_window_amount`     | Integer | `10`      | Number of pixels to move the window with keyboard.                          |
| `resize_window_amount`   | Integer | `10`      | Number of pixels to resize the window with keyboard.                        |
| `start_fullscreen`       | String  | `"st"`    | Starts specified windows that should start fullscreened. Enclosed in quotes and comma-seperated.|
| `new_win_focus`          | Bool    | `true`    | Whether openening new windows should also set focus to them or keep on current window.|
| `warp_cursor`            | Bool    | `true`    | Warp the cursor to the middle of newly focused windows.                     |
| `floating_on_top`        | Bool    | `true`    | Whether floating windows should always draw over tiled ones                 |
| `floating_on_top`        | Bool    | `true`    | Whether floating windows should always draw over tiled ones                 |
| `new_win_master`         | Bool    | `false`   | New windows will open as master window.                                              |
| `should_float`           | String  | `"st"`    | Always-float rule. Multiple entries should be comma-seperated. Optionally, entries can be enclosed in quotes.|
| `exec`                   | String  | `Nothing` | Command to run on startup (e.g., `sxbar`, `picom`, "autostart", etc.).      |
| `can_swallow`            | String  | `st`      | Windows that can swallow.                                                   |
| `can_be_swallowed`       | String  | `mpv`     | Windows that can be swallowed.                                              |

---

## Keybindings

### Syntax

- **Modifiers**: `mod`, `shift`, `ctrl`, `alt`, `super` ...
- **Key**: Case-insensitive keysym (e.g., `Return`, `q`, `1`)
> To find the key do `xev | grep "keysym"` and press the key in the box.
- **Action**: Either an external command (in quotes) or internal function.
- **move**: Move to that worspace
- **swap**: Swap window to that workspace
- **_n_**: Workspace / Scratchpad number
- **create**: Creates a scratchpad on that slot
- **toggle**: toggles the visibility of that scratchpad
- **remove**: Removes the scratchpad on that slot

```sh
bind : modifier + ... + key : action
```

```sh
scratchpad : modifier + ... + key : create n
scratchpad : modifier + ... + key : toggle n
scratchpad : modifier + ... + key : remove n
```

```sh
workspace : modifier + ... + key : move n
workspace : modifier + ... + key : swap n
```

### Available Functions

| Function Name        | Description                                                  |
|----------------------|--------------------------------------------------------------|
| `centre_window`      | Centre the focused window.                                   |
| `close_window`       | Closes the focused window.                                   |
| `decrease_gaps`      | Shrinks gaps.                                                |
| `focus_next`         | Moves focus forward in the stack.                            |
| `focus_prev`         | Moves focus backward in the stack.                           |
| `focus_next_mon`     | Switches focus to the next monitor.                          |
| `focus_prev_mon`     | Switches focus to the previous monitor.                      |
| `fullscreen`         | Fullscreen the focused window.                               |
| `global_floating`    | Toggles floating state for all windows.                      |
| `increase_gaps`      | Expands gaps.                                                |
| `master_next`        | Moves focused window down in master/stack order.             |
| `master_prev`        | Moves focused window up in master/stack order.               |
| `master_increase`    | Expands master width.                                        |
| `master_decrease`    | Shrinks master width.                                        |
| `move_next_mon`      | Moves the focused window to the next monitor.                |
| `move_prev_mon`      | Moves the focused window to the previous monitor.            |
| `move_win_up`        | Moves the focused window up (keyboard).                      |
| `move_win_down`      | Moves the focused window down (keyboard).                    |
| `move_win_left`      | Moves the focused window left (keyboard).                    |
| `move_win_right`     | Moves the focused window right (keyboard).                   |
| `quit`               | Exits `sxwm`.                                                |
| `reload_config`      | Reloads config.                                              |
| `resize_win_up`      | Resizes the focused window up (keyboard).                    |
| `resize_win_down`    | Resizes the focused window down (keyboard).                  |
| `resize_win_left`    | Resizes the focused window left (keyboard).                  |
| `resize_win_right`   | Resizes the focused window right (keyboard).                 |
| `stack_increase`     | Increase the height of the focused stack window.             |
| `stack_decrease`     | Decrease the height of the focused stack window.             |
| `switch_previous_workspace` | Switch to the previous workspace.                     |
| `toggle_floating`    | Toggles floating state of current window.                    |
| `toggle_monocle`     | Toggles the monocle layout.                                  |

### Example Bindings

```yaml
# Launch terminal
bind : mod + Return : "st"
# Close window
bind : mod + shift + q : close_window

# Scratchpads
scratchpad : mod + ctrl + Return : create 1
scratchpad : mod + shift + b : toggle 2
scratchpad : mod + alt + b : remove 2

# Switch workspace
workspace : mod + 3 : move 3
# Move window to workspace
workspace : mod + shift + 5 : swap 5
```

---

## Default Keybindings

In `default_sxwmrc`

---
