# sxwm - *The (truly) Simple Xorg Window Manager*
Performance greater than DWM, and easier to config than i3wm

<a href="https://user-images.githubusercontent.com/username/sxwm-screen1.jpg"><img src="https://user-images.githubusercontent.com/username/sxwm-screen1.jpg" width="45%" align="right"></a>

Here we have a **minimal**, **tiling**, and **configurable** window manager.

- **Tiling & Floating**: Seamlessly switch between layouts.
- **Workspaces**: Workspaces work with your bar.
- **Bars work too**: BAR BAR BAR?! Why not try [sxbar](https://github.com/uint23/sxbar)
- **Lightweight**: Single C file plus a small header and config.
- **Easy Config**: All settings in `config`.
- **SUPER Fast**: Sometimes **0.2M** sometimes **0.3M**, idk ~~but still destroys DWM~~ (this is a lie because dwm also has font rendering but still :] )
- **BEST WM**: Basically DWM on roids (and without the elitism).
- **Minimal Codebase**: ~1000 LOC
<br>

<a href="TODO"><img src="TODO" width="45%" align="right"></a>

- **Master-Stack**: Use the DWM native and super productive layout.
- **Keyboard-driven**: Full coverage via `MOD` + keys.
- **Mouse Support**: Focus under cursor, move & resize with modifiers.
- **Zero Dependencies**: Only requires `libX11`.
- **Compiles on a Toaster**: As long as it has a decently modern compiler.

<br clear="right">

---

## Default Configuration

All options reside in `config` (which is just a header-file) with clear comments.
Keybindings are also easy to understand and quick to implement.
No more bindsym or... *\*shudders\** C code...
```c
CMD(terminal,  "st");
CMD(browser,   "firefox");

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

	CALL(MOD,			equal,		inc_gaps),
	CALL(MOD,			minus,		dec_gaps),

	CALL(MOD,			space,		toggle_floating),
	CALL(MOD|SHIFT,		space,		toggle_floating_global),
   .....
```

---

## Default Keybindings

**Window Management**

| Combo                        | Action                    |
| ---------------------------- | ------------------------- |
| `Mouse`                      | Focus under cursor        |
| `MOD` + `Left Mouse`         | Move window by mouse      |
| `MOD` + `Right Mouse`        | Resize window by mouse    |
| `MOD` + `j` / `k`            | Focus next / previous     |
| `MOD` + `Shift` + `j` / `k`  | Move in master stack      |
| `MOD` + `Space`              | Toggle floating           |
| `MOD` + `Shift` + `Space`    | Toggle all floating       |
| `MOD` + `=` / `-`            | Inc/Dec gaps              |
| `MOD` + `f`                  | Fullscreen toggle         |
| `MOD` + `c`                  | Center window             |
| `MOD` + `q`                  | Close focused window      |
| `MOD` + `1-9`                | Switch workspace 1–9      |
| `MOD` + `Shift` + `1-9`      | Move window to WS 1–9     |

**Programs**

| Combo                | Action     | Programs (by default)   |
| -------------------- | ---------- | --------- |
| `MOD` + `Return`     | Terminal   | `st`      |
| `MOD` + `b`          | Browser    | `firefox` |

---

## Dependencies

- `libX11` (Xorg client library)
- GCC / Clang & Make

---

## Makefile Targets

Below are the available `make` targets for streamlining common tasks:

| Target           | Description                                                         |
| ---------------- | ------------------------------------------------------------------- |
| `make` or `make all` | Compile the source files into the `sxwm` binary.                |
| `make clean`     | Remove object files (`*.o`) and build artifacts.                    |
| `make install`   | Install `sxwm` to `$(PREFIX)/bin` (default `/usr/local/bin`).       |
| `make uninstall` | Remove the installed binary from `$(PREFIX)/bin`.                   |
| `make clean-install` | Runs `make clean` then `make install`.                          |

> You can override the install directory by specifying `PREFIX` or `DESTDIR`, for example:
> ```sh
> make install PREFIX=$HOME/.local
> ```

---

## Installation

1. **Clone repository**

   ```bash
   git clone https://github.com/uint23/sxwm.git
   cd sxwm
   ```

2. **Build**

   ```bash
   make
   ```

3. **Install**

   ```bash
   sudo make clean-install
   ```

4. **Run**

   Add to `~/.xinitrc`:
   ```sh
   exec sxwm
   ```

---

## Thanks & Inspiration

- [dwm](https://dwm.suckless.org) - Tiling & source code
- [i3](https://i3wm.org) - Making configuring easy
- [sowm](https://github.com/dylanaraps/sowm) - README inspo :)
- [tinywm](http://incise.org/tinywm.html) - idk, just cool to see how a wm works

---

<p align="center">
  <em>Contributions welcome! Open issues or submit PRs.</em>
</p>
