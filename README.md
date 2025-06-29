<div align="center">
  <img src="images/sxwm_logo.png" width="50%">
  <br>
  <b>Minimal. Fast. Configurable. Tiling Window Manager for X11</b>
  <br>
  <sub>Abhinav Prasai (2025)</sub>
  <br><br>
  <img src="https://img.shields.io/github/v/release/uint23/sxwm?style=flat-square">
  <img src="https://img.shields.io/github/license/uint23/sxwm?style=flat-square">
</div>

---

## Table of Contents
- [Launch Args](#launch-args)
- [Features](#features)
- [Screenshots](#screenshots)
- [Configuration](#configuration)
- [Keybindings](#keybindings)
  - [Example Bindings](#example-bindings)
  - [Default Keybindings](#default-keybindings)
- [Dependencies](#dependencies)
- [Build & Install](#build--install)
- [Makefile Targets](#makefile-targets)
- [Thanks & Inspiration](#thanks--inspiration)

---

## Launch Args

### `-v` or `--version`
Displays the version of `sxwm`

### `-b` or `--backup`
Allows user to use backup keybinds with `sxwm`

---

## Features

- **Tiling & Floating**: Switch seamlessly between layouts.
- **Workspaces**: 9 workspaces, fully integrated with your bar.
- **Scratchpads**: Floating windows you can summon/hide instantly.
- **Window Swallowing**: Native window swallowing support.
- **Live Config Reload**: Change your config and reload instantly with a keybind.
- **Easy Configuration**: Human-friendly `sxwmrc` file, no C required.
- **Master-Stack Layout**: DWM-inspired productive workflow.
- **Mouse Support**: Move, swap, and resize windows with the mouse.
- **Zero Dependencies**: Only `libX11` and `Xinerama` required.
- **Lightweight**: Single C file, minimal headers, compiles in seconds.
- **Bar Friendly**: Works great with [sxbar](https://github.com/uint23/sxbar).
- **Xinerama Support**: Multi-monitor ready.
- **Fast**: Designed for speed and low resource usage.

---

## Screenshots
See on the [website](https://uint23.xyz/sxwm.html)

---

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
| `resize_master_amount`   | Integer | `1`       | Percent to increase/decrease master width.                                  |
| `resize_stack_amount`    | Integer | `20`      | How many pixels to increase/decrease stack windows by                       |
| `snap_distance`          | Integer | `5`       | Distance (px) before a floating window snaps to edge.                       |
| `motion_throttle`        | Integer | `60`      | Target FPS for mouse drag actions.                                          |
| `should_float`           | String  | `"st"`    | Always-float rule. Multiple entries should be comma-seperated. Optionally, entries can be enclosed in quotes.|
| `new_win_focus`          | Bool    | `true`    | Whether openening new windows should also set focus to them or keep on current window.|
| `warp_cursor`            | Bool    | `true`    | Warp the cursor to the middle of newly focused windows                      |
| `exec`                   | String  | `Nothing` | Command to run on startup (e.g., `sxbar`, `picom`, "autostart", etc.).      |
| `can_swallow`            | String  | `st`      | Windows that can swallow.                                                   |
| `can_be_swallowed`       | String  | `mpv`     | Windows that can be swallowed.                                              |
| `new_win_master`         | Bool    | `false`   | New windows will open as master window.                                              |

---

## Keybindings

### Syntax

- **Modifiers**: `mod`, `shift`, `ctrl`, `alt`, `super`
- **Key**: Case-insensitive keysym (e.g., `Return`, `q`, `1`)
- **Action**: Either an external command (in quotes) or internal function.
- **move**: Move to that worspace
- **swap**: Swap window to that workspace
- **n**: Workspace / Scratchpad number
- **create**: Creates a scratchpad on that slot
- **toggle**: toggles the visibility of that scratchpad
- **remove**: Removes the scratchpad on that slot

```sh
bind : modifier + modifier + ... + key : action
```

```sh
scratchpad : modifier + ... + key : create n
scratchpad : modifier + ... + key : toggle n
scratchpad : modifier + ... + key : remove n
```

```sh
workspace : modifier + modifier + ... + key : move n
workspace : modifier + modifier + ... + key : swap n
```

### Available Functions

| Function Name        | Description                                                  |
|----------------------|--------------------------------------------------------------|
| `close_window`       | Closes the focused window.                                   |
| `decrease_gaps`      | Shrinks gaps.                                                |
| `focus_next`         | Moves focus forward in the stack.                            |
| `focus_prev`         | Moves focus backward in the stack.                           |
| `focus_next_mon`     | Switches focus to the next monitor.                          |
| `focus_prev_mon`     | Switches focus to the previous monitor.                      |
| `move_next_mon`      | Moves the focused window to the next monitor.                |
| `move_prev_mon`      | Moves the focused window to the previous monitor.            |
| `increase_gaps`      | Expands gaps.                                                |
| `master_next`        | Moves focused window down in master/stack order.             |
| `master_prev`        | Moves focused window up in master/stack order.               |
| `quit`               | Exits `sxwm`.                                                |
| `reload_config`      | Reloads config.                                              |
| `master_increase`    | Expands master width.                                        |
| `master_decrease`    | Shrinks master width.                                        |
| `toggle_floating`    | Toggles floating state of current window.                    |
| `global_floating`    | Toggles floating state for all windows.                      |
| `fullscreen`         | Fullscreen the focused window.                               |
| `centre_window`      | Centre the focused window.                                   |

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

### Window Management
| Combo                       | Action                             |
| --------------------------- | ---------------------------------- |
| `Mouse`                     | Focus on click under cursor        |
| `MOD` + `Left Mouse`        | Move window with mouse             |
| `MOD` + `Right Mouse`       | Resize window with mouse           |
| `MOD` + `j` / `k`           | Focus next / previous              |
| `MOD` + `Shift` + `j` / `k` | Move window in master stack        |
| `MOD` + `,` / `.`           | Focus prev / next monitor          |
| `MOD` + `Shift` + `,` / `.` | Move window to prev / next monitor |
| `MOD` + `h` / `l`           | Resize master area (decr/incr)     |
| `MOD` + `Ctrl` + `h` / `l`  | Resize stack area (decr/incr)      |
| `MOD` + `=` / `-`           | Increase / decrease gaps           |
| `MOD` + `Space`             | Toggle floating                    |
| `MOD` + `Shift` + `Space`   | Toggle all floating                |
| `MOD` + `Shift` + `f`       | Toggle fullscreen for focused window  |
| `MOD` + `Shift` + `q`       | Close focused window               |
| `MOD` + `Shift` + `e`       | Quit sxwm                          |
| `MOD` + `r`                 | Reload configuration               |
| `MOD` + `c`                 | Centre window                      |

### Scratchpads

| Combo                              | Action                           |
| ---------------------------------- | -------------------------------- |
| `MOD` + `Alt` + `1–4`              | Create scratchpad 1–5            |
| `MOD` + `Ctrl` + `1–4`             | Toggle scratchpad 1–5            |
| `MOD` + `Alt` + `Shift` + `1–4`    | Remove scratchpad 1–5            |

### Workspaces

| Combo                   | Action                       |
| ----------------------- | ---------------------------- |
| `MOD` + `1–9`           | Switch to workspace 1–9      |
| `MOD` + `Shift` + `1–9` | Move window to workspace 1–9 |

### Applications

| Combo            | Action        | Program     |
| ---------------- | ------------- | ----------- |
| `MOD` + `Return` | Open terminal | `st`        |
| `MOD` + `b`      | Open browser  | `firefox`   |
| `MOD` + `p`      | Run launcher  | `dmenu_run` |

---

## Dependencies

- `libX11` (Xorg client library)
- `Xinerama`
- `XCursor`
- GCC or Clang & Make

<details>
<summary>Debian / Ubuntu / Linux Mint</summary>
<pre><code>sudo apt update
sudo apt install libx11-dev libxcursor-dev libxinerama-dev build-essential</code></pre>
</details>

<details>
<summary>Arch Linux / Manjaro</summary>
<pre><code>sudo pacman -Syy
sudo pacman -S libx11 libxinerama gcc make</code></pre>
</details>

<details>
<summary>Gentoo</summary>
<pre><code>sudo emerge --ask x11-libs/libX11 x11-libs/libXinerama sys-devel/gcc sys-devel/make
sudo emaint -a sync
</code></pre>
</details>

<details>
<summary>Void Linux</summary>
<pre><code>sudo xbps-install -S
sudo xbps-install libX11-devel libXinerama-devel gcc make</code></pre>
</details>

<details>
<summary>Fedora / RHEL / AlmaLinux / Rocky</summary>
<pre><code>sudo dnf update
sudo dnf install libX11-devel libXcursor-devel libXinerama-devel gcc make</code></pre>
</details>

<details>
<summary>OpenSUSE (Leap / Tumbleweed)</summary>
<pre><code>sudo zypper refresh
sudo zypper install libX11-devel libXinerama-devel gcc make</code></pre>
</details>

<details>
<summary>Alpine Linux</summary>
<pre><code>doas apk update
doas apk add libx11-dev libxinerama-dev gcc make musl-dev</code></pre>
</details>

<details>
<summary>NixOS</summary>
<pre><code>buildInputs = [
  pkgs.xorg.libX11
  pkgs.xorg.libXinerama
  pkgs.libgcc
  pkgs.gnumake
];
sudo nixos-rebuild switch
</code></pre>
</details>

<details>
<summary>Slackware</summary>
<pre><code>slackpkg update
slackpkg install gcc make libX11 libXinerama</code></pre>
</details>

<details>
<summary>OpenBSD</summary>
<pre><code>doas pkg_add gmake</code></pre>
You will also need the X sets (<code>xbase</code>, <code>xfonts</code>, <code>xserv</code> and <code>xshare</code>) installed.
When you make the code, use <code>gmake</code> instead of <code>make</code> (which will be BSD make). Use the following command to build: <code>gmake CFLAGS="-I/usr/X11R6/include -Wall -Wextra -O3 -Isrc" LDFLAGS="-L/usr/X11R6/lib -lX11 -lXinerama -lXcursor"</code>
</details>

<details>
<summary>FreeBSD</summary>
<pre><code># If you use doas or su instead of sudo, modify the following commands accordingly.
sudo pkg update
sudo pkg install gcc gmake libX11 libXinerama</code></pre>
</details>

---

## Build & Install

### Arch Linux (AUR)

```sh
yay -S sxwm
# OR for latest features:
yay -S sxwm-git
```

### Void Linux

```sh
sudo xbps-install -S sxwm
```

### Build from Source

```sh
git clone --depth=1 https://github.com/uint23/sxwm.git
cd sxwm/
# Replace make with gmake on FreeBSD
make
sudo make clean install
```

### Run

Add to your `~/.xinitrc`:
```sh
exec sxwm
```

---
## Makefile Targets

| Target                | Description                                              |
|-----------------------|----------------------------------------------------------|
| `make` / `make all`   | Build the `sxwm` binary                                  |
| `make clean`          | Remove build artifacts                                   |
| `make install`        | Install `sxwm` to `$(PREFIX)/bin` (default `/usr/local`) |
| `make uninstall`      | Remove installed binary                                  |
| `make clean install`  | Clean then install                                       |

> Override install directory with `PREFIX`:
> ```sh
> make install PREFIX=$HOME/.local
> ```

---

## Thanks & Inspiration

- [dwm](https://dwm.suckless.org) — Tiling & source code
- [i3](https://i3wm.org) — Easy configuration
- [sowm](https://github.com/dylanaraps/sowm) — README inspiration
- [tinywm](http://incise.org/tinywm.html) — Minimal X11 WM

---

<p align="center">
  <em>Contributions welcome, Please read CONTRIBUTIONS.md for more info!</em>
</p>
