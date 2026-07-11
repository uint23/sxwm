#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xcursor/Xcursor.h>
#include "defs.h"
#include "parser.h"

// Function to parse the configuration
int parser(Config *cfg) {
    // ... (rest of the parser function remains the same)

    // Parse the cursor_x and cursor_y values
    cfg->cursor_x = atoi(rest);
    cfg->cursor_y = atoi(rest);
}