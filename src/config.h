/* See LICENSE for more information on use */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "defs.h"

CMD(terminal, "st");
CMD(browser, "firefox");

const Binding binds[] = {
    {Mod4Mask | ShiftMask, XK_e, {.fn = quit}, TYPE_FUNC},
    {Mod4Mask | ShiftMask, XK_q, {.fn = close_focused}, TYPE_FUNC},

    {Mod4Mask, XK_j, {.fn = focus_next}, TYPE_FUNC},
    {Mod4Mask, XK_k, {.fn = focus_prev}, TYPE_FUNC},

    {Mod4Mask | ShiftMask, XK_j, {.fn = move_master_next}, TYPE_FUNC},
    {Mod4Mask | ShiftMask, XK_k, {.fn = move_master_prev}, TYPE_FUNC},

    {Mod4Mask, XK_l, {.fn = resize_master_add}, TYPE_FUNC},
    {Mod4Mask, XK_h, {.fn = resize_master_sub}, TYPE_FUNC},

    {Mod4Mask, XK_equal, {.fn = inc_gaps}, TYPE_FUNC},
    {Mod4Mask, XK_minus, {.fn = dec_gaps}, TYPE_FUNC},

    {Mod4Mask, XK_space, {.fn = toggle_floating}, TYPE_FUNC},
    {Mod4Mask | ShiftMask, XK_space, {.fn = toggle_floating_global}, TYPE_FUNC},
    {Mod4Mask | ShiftMask, XK_f, {.fn = toggle_fullscreen}, TYPE_FUNC},

    {Mod4Mask, XK_Return, {.cmd = terminal}, TYPE_CMD},
    {Mod4Mask, XK_b, {.cmd = browser}, TYPE_CMD},
    {Mod4Mask, XK_p, {.cmd = (const char *[]){"dmenu_run", NULL}}, TYPE_CMD},

    {Mod4Mask, XK_r, {.fn = reload_config}, TYPE_FUNC},

    {Mod4Mask, XK_1, {.ws = 0}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_1, {.ws = 0}, TYPE_MWKSP},
    {Mod4Mask, XK_2, {.ws = 1}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_2, {.ws = 1}, TYPE_MWKSP},
    {Mod4Mask, XK_3, {.ws = 2}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_3, {.ws = 2}, TYPE_MWKSP},
    {Mod4Mask, XK_4, {.ws = 3}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_4, {.ws = 3}, TYPE_MWKSP},
    {Mod4Mask, XK_5, {.ws = 4}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_5, {.ws = 4}, TYPE_MWKSP},
    {Mod4Mask, XK_6, {.ws = 5}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_6, {.ws = 5}, TYPE_MWKSP},
    {Mod4Mask, XK_7, {.ws = 6}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_7, {.ws = 6}, TYPE_MWKSP},
    {Mod4Mask, XK_8, {.ws = 7}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_8, {.ws = 7}, TYPE_MWKSP},
    {Mod4Mask, XK_9, {.ws = 8}, TYPE_CWKSP},
    {Mod4Mask | ShiftMask, XK_9, {.ws = 8}, TYPE_MWKSP},
};