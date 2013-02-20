/*
 * =====================================================================================
 *
 *       Filename:  selector.cpp
 *
 *    Description: 	Simple GUI to draw rectangles on an image and save the selection 
 *
 *        Version:  1.0
 *        Created:  02/13/2013 11:37:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Brian Lee, bml0070@unt.edu
 *
 * =====================================================================================
 */

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
using namespace cv;

#include <iostream>
#include <fstream>
using namespace std;

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

/*
 * Entry point
 */
int main(int argc, char *argv[]) {

	const char* filename = "lenaN.tif";
	Mat img, img_selection, backup;

	/*
	 * Read the image 
	 * Copy to a backup
	 */
	img = imread(filename, CV_LOAD_IMAGE_COLOR);
	img_selection = img.clone();
	backup = imread(filename, CV_LOAD_IMAGE_COLOR);

	namedWindow(filename, CV_WINDOW_AUTOSIZE);
	setMouseCallback(filename, mouse_callback, (void*) &img);

	while(running) {
		/*
		 * Reset the image so drawing a rectangle is animated
		 */
		backup.copyTo(img);

		if(dragging) {
			/*
			 * Draw the current mouse selection
			 */
			rectangle(img, selection, Scalar(0, 255, 0), 1, 8, 0);
		}
		else if(finished) {
			finished = false;
			
			/*
			 * First verify that the selection is large enough or the program will crash
			 */
			if(selection.width < 10 || selection.height < 10)
				continue;
		
			Mat interest = img_selection(selection);
			imshow("Region of Interest", interest);
		}

		// show the image in a window
		imshow(filename, img);

		/* 
		 * wait 10 milliseconds for keyboard input
		 */
		int keyCode = cvWaitKey(10);
		switch(keyCode) {
			case ESCAPE: {
				running = false;
				break;
			 }				 
			/*
			 * Press h for help
			 */
			case 'h': {
				//show help
				break;
			}
		}
	}

	return 0;
}

void mouse_callback(int event, int x, int y, int flags, void* param) {
	Mat* current_image = (Mat*) param;

	switch(event) {
		case CV_EVENT_MOUSEMOVE: {
			finished = false;
			// while dragging, the width and height of the 
			// selection box will change
			if(dragging) {
				selection.width = x - selection.x;
				selection.height = y - selection.y;
			}
			break;
		}
		case CV_EVENT_LBUTTONDOWN: {
			// left mouse button means start coordinates are selected
			dragging = true;
			selection = cvRect(x, y, 0, 0);
			break;
		}
		case CV_EVENT_LBUTTONUP: {
			// left button up means dragging is done, 
			// draw the selection
			dragging = false;
			finished = true;

			// if the user dragged up or to the left
			// we cant have a negative box, so shift
			// x or y accordingly
			if(selection.width < 0) {
				selection.x += selection.width;
				selection.width *= -1;
			}
			if(selection.height < 0) {
				selection.y += selection.height;
				selection.height *= -1;
			}

			break;
		}
	}
}
