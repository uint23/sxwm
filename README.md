![logo](logo.png)

See [the config file](default_sxwmrc) for all the features

## Build & Install
> I don't maintain any packages. Use with caution!

### Dependencies

- Xlib
- Xinerama
- XCursor
- CC
- Make

### Arch Linux (AUR)

```sh
yay -S sxwm
```

or

```sh
yay -S sxwm-git
```

### Void Linux

```sh
sudo xbps-install -S sxwm
```

### Build from Source

To see the exact build packages needed for your distribution, [see this
file](docs/SPECIFIC_BUILD.md)

```sh
git clone https://github.com/uint23/sxwm.git
cd sxwm/
make
make install
```

### Run

You can add to your `~/.xinitrc`:
```sh
exec sxwm
```

You can also use the `sxwm.desktop` file with a display manager

## Contributions & Issues

Please read [the contribution guide](docs/CONTRIBUTIONS.md)
Please read [the developer docs](docs/sxwm-dev.md)

## Thanks & Inspiration

- [dwm](https://dwm.suckless.org) - Tiling & source code
- [i3](https://i3wm.org) - Easy configuration
- [sowm](https://github.com/dylanaraps/sowm) - README inspiration
- [tinywm](http://incise.org/tinywm.html) - Minimal X11 WM

