#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xcursor/Xcursor.h>
#include "defs.h"
#include "parser.h"

int main() {
    // Create a test window
    Client c;
    c.win = XCreateWindow(dpy, RootWindow(dpy, 0), 0, 0, 100, 100, 0, CopyFromParent, InputOutput, CopyFromParent, CWOverrideRedirect, NULL);
    XMapWindow(dpy, c.win);

    // Set the cursor position
    calc_cursor_position(&c);

    // Verify that the cursor position is correct
    int cursor_x = 0;
    int cursor_y = 0;
    XQueryPointer(dpy, c.win, &cursor_x, &cursor_y);

    printf("Cursor position: (%d, %d)\n", cursor_x, cursor_y);

    return 0;
}