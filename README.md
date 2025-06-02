> ⚠️ **Note:** I won’t be updating this project for a month or so due to exams.  
> Issues & PRs are welcome, just don't expect a quick response 🥀🥀

> **24/05/25:** I have very _little_ time but I am able to develop some features
> Thank you to the wonderful people who have sumbitted fixes and other PR's

> **01/06/25:** I will be back to exams so I will have little to no time but I'm _finally_ nearly done with them! 

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

## Features

- **Tiling & Floating**: Switch seamlessly between layouts.
- **Workspaces**: 9 workspaces, fully integrated with your bar.
- **Live Config Reload**: Change your config and reload instantly with a keybind.
- **Easy Configuration**: Human-friendly `sxwmrc` file, no C required.
- **Master-Stack Layout**: DWM-inspired productive workflow.
- **Mouse Support**: Move, swap, resize, and focus windows with the mouse.
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
| `snap_distance`          | Integer | `5`       | Distance (px) before a floating window snaps to edge.                       |
| `motion_throttle`        | Integer | `60`      | Target FPS for mouse drag actions.                                          |
| `should_float`           | String  | `"st"`    | Always-float rule. Multiple entries should be comma-seperated. Optionally, entries can be enclosed in quotes.|
| `new_win_focus`          | Bool    | `true`    | Whether openening new windows should also set focus to them or keep on current window.|
| `warp_cursor`            | Bool    | `true`    | Warp the cursor to the middle of newly focused windows                      |

---

## Keybindings

### Syntax

```sh
bind : modifier + modifier + ... + key : action
```

- **Modifiers**: `mod`, `shift`, `ctrl`, `alt`, `super`
- **Key**: Case-insensitive keysym (e.g., `Return`, `q`, `1`)
- **Action**: Either an external command (in quotes) or internal function.

```sh
workspace : modifier + modifier + ... + key : move n
workspace : modifier + modifier + ... + key : swap n
```
- **Modifiers**: `mod`, `shift`, `ctrl`, `alt`, `super`
- **Key**: Case-insensitive keysym (e.g., `Return`, `q`, `1`)
- **move**: Move to that worspace
- **swap**: Swap window to that workspace
- **n**: Workspace number

### Available Functions

| Function Name        | Description                                                  |
|----------------------|--------------------------------------------------------------|
| `close_window`       | Closes the focused window.                                   |
| `decrease_gaps`      | Shrinks gaps.                                                |
| `focus_next`         | Moves focus forward in the stack.                            |
| `focus_previous`     | Moves focus backward in the stack.                           |
| `increase_gaps`      | Expands gaps.                                                |
| `master_next`        | Moves focused window down in master/stack order.             |
| `master_prev`        | Moves focused window up in master/stack order.               |
| `quit`               | Exits `sxwm`.                                                |
| `reload_config`      | Reloads config.                                              |
| `master_increase`    | Expands master width.                                        |
| `master_decrease`    | Shrinks master width.                                        |
| `toggle_floating`    | Toggles floating state of current window.                    |
| `global_floating`    | Toggles floating state for all windows.                      |
| `fullscreen`         | Fullscreen toggle.                                           |

### Example Bindings

```yaml
# Launch terminal
bind : mod + Return : "st"
# Close window
bind : mod + shift + q : close_window

# Switch workspace
workspace : mod + 3 : move 3
# Move window to workspace
workspace : mod + shift + 5 : swap 5
```

---

## Default Keybindings

### Window Management

| Combo                        | Action                    |
| ---------------------------- | ------------------------- |
| Mouse                        | Focus under cursor        |
| `MOD` + Left Mouse           | Move window by mouse      |
| `MOD` + Right Mouse          | Resize window by mouse    |
| `MOD` + `j` / `k`            | Focus next / previous     |
| `MOD` + `Shift` + `j` / `k`  | Move in master stack      |
| `MOD` + `Space`              | Toggle floating           |
| `MOD` + `Shift` + `Space`    | Toggle all floating       |
| `MOD` + `=` / `-`            | Increase/Decrease gaps    |
| `MOD` + `f`                  | Fullscreen toggle         |
| `MOD` + `q`                  | Close focused window      |
| `MOD` + `1-9`                | Switch workspace 1–9      |
| `MOD` + `Shift` + `1-9`      | Move window to WS 1–9     |

### Programs

| Combo                | Action     | Program    |
| -------------------- | ---------- | ---------- |
| `MOD` + `Return`     | Terminal   | `st`       |
| `MOD` + `b`          | Browser    | `firefox`  |
| `MOD` + `p`          | Launcher   | `dmenu_run`|

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
  <em>Contributions welcome! Open issues or submit PRs.</em>
</p>
