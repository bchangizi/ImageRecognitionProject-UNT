#ifndef DEFINES_H
#define DEFINES_H

/*
 * Keep track of when mouse is dragging on the named window.
 * dragging will be true after left button is pressed,
 * and false when the left button is released.  
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
 * If use_edges is true, display the edge map
 * instead of the original image.
 */
bool use_edges = false;

/*
 * If reduce_noise is true, apply a median blur
 * (simple noise reduction).
 */
bool reduce_noise = false;

/*
 * If max_contrast is true, convert the image
 * to its maximum contrast.
 */
bool max_contrast = false;

/*
 * Contains coordinates for the selected image.
 */
Rect selection;

/*
 * Called when the named window gets a mouse event.
 */
void mouse_callback(int event, int x, int y, int flags, void* param);

/*
 * Break the image into its channels and equalize their histograms.
 */
void maximize_contrast(Mat &img);

/*
 * Show the Sobel edge map of the image
 */
void detect_edges(Mat &img);

/*
 * Ridiculous hack to put text on newlines...
 * Each array index is effectively a new line.
 */
const char *help_array[] = {"ESC -- quit.",
							"h   -- toggle this menu",
							"e   -- toggle edge map",
							"r   -- reduce noise (median blur)",
							"c   -- maximize contrast"};
/*
 * Length of help_array
 */
#define HELP_LENGTH 5

#endif
