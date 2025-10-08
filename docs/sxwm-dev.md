# sxwm developer docs

# Table of Contents
1. [Headers](#headers)
2. [sxwm.c](#sxwmc)
    1. [Functions](#functions)



## Headers

| Header                      | Job                                                                       |
|------------------------------|--------------------------------------------------------------------------|
| `signal.h`                   | Signal handling functions and definitions                                |
| `unistd.h`                   | Standard POSIX functionality                                             |
| `X11/Xatom.h`                | Definitions for Atom types                                               |
| `X11/Xlib.h`                 | Core X Window System protocol and routines for creating windows          |
| `X11/Xproto.h`               | Protocol definitions for X Window System communication                   |
| `X11/Xutil.h`                | Used for getting `Hints` about windows                                   |
| `X11/extensions/Xinerama.h`  | Xinerama extension definitions for multi-monitor setups                  |
| `X11/Xcursor/Xcursor.h`      | Modern cursor handling                                                   |
| `linux/limits.h`             | Linux specific limits for various data types                             |
| | |
| `defs.h`                     | Custom definitions and macros specific to the application or project     |
| `parser.h`                   | Config file parser components                         |

## sxwm.c

### Functions
