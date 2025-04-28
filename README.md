<div align="center">
  <img src="images/sxwm_logo.png" width="30%">
  <h2>sxwm</h2>
  <b>Minimal. Fast. Configurable. Tiling Window Manager for X11</b>
  <br>
  <sub>By Abhinav Prasai (2025)</sub>
  <br><br>
  <img src="https://img.shields.io/github/v/release/uint23/sxwm?style=flat-square">
  <img src="https://img.shields.io/github/license/uint23/sxwm?style=flat-square">
</div>

---

## Features

- **Tiling & Floating**: Switch seamlessly between layouts.
- **Workspaces**: 9 workspaces, fully integrated with your bar.
- **Live Config Reload**: Change your config and reload instantly with a keybind.
- **Easy Configuration**: Human-friendly `sxwmrc` file, no C required.
- **Master-Stack Layout**: Productive, DWM-inspired workflow.
- **Mouse Support**: Move, swap, resize, and focus windows with the mouse.
- **Zero Dependencies**: Only `libX11` and `Xinerama` required.
- **Lightweight**: Single C file, minimal headers, compiles in seconds.
- **Bar Friendly**: Works great with [sxbar](https://github.com/uint23/sxbar).
- **Xinerama Support**: Multi-monitor ready.
- **Fast**: Designed for speed and low resource usage.

---

## Screenshots

<a href="1"><img src="images/1.png" width="50%" align="right"></a>
<a href="2"><img src="images/2.png" width="50%" align="right"></a>

<br clear="right">

---

## Patch Notes

<details>
<summary><strong>Click to expand</strong></summary>

#### v1.1.1
- **NEW**: Xinerama support, swap windows with Mod + Shift + Drag.
- **FIXED**: New windows in `global_floating` mode spawn centered.

</details>

---

## Default Configuration

All options are in `sxwmrc` (which uses a simple DSL).  
Keybindings are easy to read and edit.
No recompiling, no C code, just edit and reload!

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
- GCC or Clang & Make

---

## Build & Install

### Arch Linux (AUR)

```sh
yay -S sxwm
```

### Build from Source

```sh
git clone --depth=1 https://github.com/uint23/sxwm.git
cd sxwm/
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

| Target                | Description                                         |
| --------------------- | --------------------------------------------------- |
| `make` / `make all`   | Build the `sxwm` binary                            |
| `make clean`          | Remove object files and build artifacts             |
| `make install`        | Install `sxwm` to `$(PREFIX)/bin` (default `/usr/local/bin`) |
| `make uninstall`      | Remove the installed binary                         |
| `make clean install`  | Clean and then install                              |

> Override install directory with `PREFIX` or `DESTDIR`:
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