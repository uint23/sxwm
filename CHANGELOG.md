### Changelog

All notable changes to this project will be documented in this file.

#### v1.7 (git)
- **ADD**: `_NET_FRAME_EXTENTS` support
- **ADD**: `start_fullscreen` which opens specified windows in fullscreen mode
- **FIX**: Resizing bug on second monitor

#### v1.6 (current)
- **NEW**: True multi-monitor support
- **NEW**: Vertical stack resizing
- **NEW**: Mouse warping
- **NEW**: Floating window rules
- **NEW**: Ctrl key as modifier
- **NEW**: Focus window on creation
- **NEW**: Copy config to `/usr/local/share/sxwmrc`
- **NEW**: Can switch monitors via keyboard
- **NEW**: Can move windows between monitors via keyboard
- **NEW**: Can click on a window to set focus to it
- **NEW**: Window swallowing
- **NEW**: New windows can now open as master window
- **NEW**: Scratchpads
- **NEW**: Window centering
- **CHANGE**: Renamed `focus_previous` to `focus_prev`
- **CHANGE**: Invalid sample config
- **CHANGE**: Parser `$HOME` searching order. XDG Compliance
- **CHANGE**: `-b` or `--backup` option for using backup keybinds
- **FIXED**: Improved parsing now supporting commands with `"` and `'`
- **FIXED**: (mouse warping) Switching to master doesn't automatically shift cursor to it
- **FIXED**: `ctrl` and `shift` key works as a modifier
- **FIXED**: Fixed build error (#64).
- **FIXED**: Removed debug logs
- **FIXED**: Fixed new window getting interrupted by mouse
- **FIXED**: Fixed `should_float` segfalt
- **FIXED**: Invisible windows of minimized programs
- **FIXED**: Zombie processes spawned from apps
- **FIXED**: Undefined behaviour in `parse_col`
- **FIXED**: Added monitor switching functions to call_table (#95)

#### v1.5
- **NEW**: Using XCursor instead of cursor font && new logo.
- **CHANGE**: No longer using INIT_WORKSPACE macro, proper workspace handling. New sxwmrc
- **FIXED**: Proper bind resetting on refresh config. && Multi-arg binds now work due to new and improved spawn function

#### v1.4
- **CHANGE**: Added motion throttle && master width general options

#### v1.3
- **CHANGE**: ulong, u_char uint are gone

#### v1.2
- **NEW**: Parser support
- **FIXED**: Quit syntax && Freeing cursor on exit

#### v1.1
- **NEW**: Xinerama support, swap windows with Mod + Shift + Drag
- **FIXED**: New windows in `global_floating` mode spawn centered
