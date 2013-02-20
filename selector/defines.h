#ifndef DEFINES_H
#define DEFINES_H

#define ESCAPE 27

/*
 * Keep track of when mouse is dragging on the named window
 */
bool dragging = false;
/*
 * Set to true when dragging stops
 */
bool finished = false;
/*
 * Set to false when the GUI needs to be closed
 */
bool running = true;
/*
 * When true, show the help menu.
 */
bool help = false;
/*
 * Contains coordinates for the selected image
 */
Rect selection;
/*
 * Called when the named window gets a mouse event
 */
void mouse_callback(int event, int x, int y, int flags, void* param);

#endif
