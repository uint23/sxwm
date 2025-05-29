### Changelog

All notable changes to this project will be documented in this file.

#### v1.6 (current)
- **NEW**: True multi-monitor support
- **FIXED**: Invisible windows of minimized programs
- **FIXED**: Zombie processes spawned from apps
- **FIXED**: Invalid sample config
- **FIXED**: Undefined behaviour in `parse_col`

#### v1.5
- **NEW**: Using XCursor instead of cursor font && new logo.
- **FIXED**: Proper bind resetting on refresh config. && Multi-arg binds now work due to new and improved spawn function
- **CHANGE**: No longer using INIT_WORKSPACE macro, proper workspace handling. New sxwmrc

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
