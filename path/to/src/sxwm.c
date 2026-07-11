#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xcursor/Xcursor.h>
#include "defs.h"
#include "parser.h"

// Function to calculate the correct cursor position based on the window size and position
void calc_cursor_position(Client *c) {
    int mon = c->mon;
    int x = mons[mon].x + mons[mon].reserve_left;
    int y = mons[mon].y + mons[mon].reserve_top;
    int w = mons[mon].w - mons[mon].reserve_left - mons[mon].reserve_right;
    int h = mons[mon].h - mons[mon].reserve_top - mons[mon].reserve_bottom;

    // Calculate the cursor position based on the window size and position
    int cursor_x = x + (w / 2);
    int cursor_y = y + (h / 2);

    // Set the cursor position
    XWarpPointer(dpy, None, c->win, 0, 0, 1, 1, cursor_x, cursor_y);
}

// Function to tile the windows
void tile(void) {
    // ... (rest of the tile function remains the same)

    // gaps when only a single window is tiled
    if (total == 1 && !monocle) {
        for (Client *c = head; c; c = c->next) {
            // ... (rest of the single window gap logic remains the same)

            // Calculate the correct cursor position
            calc_cursor_position(c);
        }
    }
}

// Function to init the default values
void init_defaults(void) {
    // ... (rest of the init_defaults function remains the same)

    // Set the default values for the new function
    user_config.cursor_x = 0;
    user_config.cursor_y = 0;
}

// Function to parse the configuration
int parser(Config *cfg) {
    // ... (rest of the parser function remains the same)

    // Parse the cursor_x and cursor_y values
    cfg->cursor_x = atoi(rest);
    cfg->cursor_y = atoi(rest);
}