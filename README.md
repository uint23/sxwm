<div align="center">
  <img src="logo.png" width="50%">
  <br>
  <b>A very SeXy WM</b>
  <br><br>
  <img src="https://img.shields.io/github/v/release/uint23/sxwm?style=flat-square">
  <img src="https://img.shields.io/github/license/uint23/sxwm?style=flat-square">
</div>

---

## Contributions and Issues

Please read [the contribution guide](docs/CONTRIBUTIONS.md) and [the developer docs](docs/sxwm-dev.md).

---

## Features and Configuration

Check [the man page markdown](docs/sxwm.md) for relevant info.

---

## Dependencies

- `libX11`
- `Xinerama`
- `XCursor`
- `cc` (the C compiler)
- GNU Make (`gmake`)

## Platform-specific installation

### Debian-based distros (Debian, Ubuntu, Linux Mint etc.)
```
sudo apt update
sudo apt install libx11-dev libxcursor-dev libxinerama-dev build-essential
```

### Arch-based distros (Arch Linux, Manjaro, CachyOS etc.)

```
sudo pacman -Syy
sudo pacman -S libx11 libxinerama gcc make
```

### Gentoo

```
sudo emerge --ask x11-libs/libX11 x11-libs/libXinerama sys-devel/gcc sys-devel/make
sudo emaint -a sync
```

### Void Linux

```
sudo xbps-install -S
sudo xbps-install libX11-devel libXinerama-devel libXcursor-devel gcc make
```

### RHEL-based distros (Fedora, RHEL, Alma Linux, Rocky Linux etc.)

```
sudo dnf update
sudo dnf install libX11-devel libXcursor-devel libXinerama-devel gcc make
```

### openSUSE

```
sudo zypper refresh
sudo zypper install libX11-devel libXinerama-devel gcc make
```

### Alpine Linux

```
doas apk update
doas apk add libx11-dev libxinerama-dev libxcursor-dev gcc make musl-dev linux-headers
```

### NixOS

```
buildInputs = [
  pkgs.xorg.libX11
  pkgs.xorg.libXinerama
  pkgs.libgcc
  pkgs.gnumake
];
sudo nixos-rebuild switch
```

### Slackware

```
slackpkg update
slackpkg install gcc make libX11 libXinerama
```

### FreeBSD

```
sudo pkg update
sudo pkg install gcc gmake libX11 libXinerama
```

### OpenBSD

```
doas pkg_add gmake
```

You will also need the X sets (`xbase`, `xfonts`, `xserv` and `xshare`) installed.

### Termux

```
pkg install x11-repo
pkg update
pkg install clang make xcb-util-keysyms xorgproto libxcursor libx11 libxinerama libandroid-wordexp
```

---

## Build and Install

> [!NOTE]
> 
> I don't maintain any packages. Use with caution!

### Arch Linux (AUR)

```sh
yay -S sxwm
```

#### OR for latest features

```sh
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
make
sudo/doas make clean install
```

### Run

Add to your `~/.xinitrc`:
```sh
exec sxwm
```
or use the `sxwm.desktop` file.

---
## Makefile Targets

| Target                | Description                                              |
|-----------------------|----------------------------------------------------------|
| `make` / `make all`   | Build the sxwm binary using detected X11 paths and platform-specific flags                                  |
| `make clean`          | Remove build artifacts (`$(BUILDDIR)` and binary)                                   |
| `make distclean`      | Same as clean, plus remove `compile_flags.txt` used for editor support |
| `make install`        | Install sxwm to `$(PREFIX)/bin` (default `/usr/local`) and man/page/config files |
| `make uninstall`      | Remove installed binary, man page, and default config                                  |
| `make clean install`  | Clean build artifacts then install                                       |
| `make clangd`         | Generate `compile_flags.txt` for editor/IDE integration |

> [!NOTE]
> 
> - The Makefile auto-detects the platform (Linux, BSD, Termux/Android) and sets `CPPFLAGS` and `LDFLAGS` accordingly.
> - You can override compiler/linker flags without modifying the Makefile using the various USER_* flags:
>   ```sh
>   make USER_CFLAGS="-O2 -Wall" USER_LDFLAGS="-L/opt/lib" USER_LDLIBS="-lm"
>   ```
> - You can change the install prefix:
>   ```sh
>   make install PREFIX=$HOME/.local
>   ```
> - The build directory can be customized via `BUILDDIR` (default `build`).

---

## Thanks and Inspiration

- [dwm](https://dwm.suckless.org) - Tiling and source code
- [i3](https://i3wm.org) - Easy configuration
- [sowm](https://github.com/dylanaraps/sowm) - README inspiration
- [tinywm](http://incise.org/tinywm.html) - Minimal X11 WM

---

<p align="center">
  <em>uint [2025]</em>
</p>
