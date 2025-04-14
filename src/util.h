#ifndef UTIL_H
#define UTIL_H
#include <X11/Xlib.h>

void otherwm(void);
int otherwmerr(Display *dpy, XErrorEvent *ee);

#endif
