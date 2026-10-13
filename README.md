![logo](logo.png)

See [`default_sxwmrc`](default_sxwmrc) for a complete list of features and configuration options.

## Build & Install

> [!WARNING]
> I don't maintain any packages. Use third-party packages at your own risk.

### Dependencies

- Xlib
- Xinerama
- XCursor
- C compiler (CC)
- Make

### Arch Linux (AUR)

```sh
yay -S sxwm
```

### Void Linux

```sh
sudo xbps-install -S sxwm
```

### Build from Source

For distribution-specific build dependencies, see the [build dependencies guide](docs/SPECIFIC_BUILD.md).

```sh
git clone https://github.com/uint23/sxwm
cd sxwm/
make
sudo make install
```

## Running sxwm

To start sxwm with `startx`, add the following to your `~/.xinitrc`:

```sh
exec sxwm
```
Alternatively, select **sxwm** from your display manager's session menu.
The `sxwm.desktop` session file is installed automatically.

## Contributions & Issues

Before contributing or reporting an issue, please read the [contribution guide](docs/CONTRIBUTIONS.md).

## Thanks & Inspiration

- [dwm](https://dwm.suckless.org): Tiling and source code
- [i3](https://i3wm.org): Easy configuration
- [sowm](https://github.com/dylanaraps/sowm): README inspiration
- [tinywm](http://incise.org/tinywm.html): Minimal X11 window manager
