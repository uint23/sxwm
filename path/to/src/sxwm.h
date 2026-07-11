#ifndef SXWM_H
#define SXWM_H

// ... (rest of the definitions remain the same)

// Function to calculate the correct cursor position based on the window size and position
void calc_cursor_position(Client *c);

// Variables to store the cursor position
int cursor_x;
int cursor_y;

#endif