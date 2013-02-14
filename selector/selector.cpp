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
 * Contains coordinates for the selected image
 */
CvRect selection;
/*
 * Called when the named window gets a mouse event
 */
void mouse_callback(int event, int x, int y, int flags, void* param);
/*
 * Compute the image chunk to show
 * Set 'finished' to true if we are done dragging
 */
void show_selection(IplImage* current_image);

/*
 * Entry point
 */
int main(int argc, char *argv[]) {

	const char* filename = "lenaN.tif";
	IplImage* img;
	IplImage* img_selection;
	IplImage* backup;

	/*
	 * Read the image 
	 * Copy to a backup
	 */
	img = cvLoadImage(filename);
	backup = cvLoadImage(filename);
	img_selection = cvCloneImage(img);

	cvNamedWindow(filename, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(filename, mouse_callback, (void*) img);

	while(1) {

		cvCopyImage(img, img_selection);

		if(dragging) {
			show_selection(img_selection);
		}
		else if(finished) {
			finished = false;
			
			/*
			 * First verify that the selection is large enough or the program will crash
			 */
			if(selection.width < 10 || selection.height < 10)
				continue;

			/*
			 * Create a 'region of interest' from the selection rectangle
			 * Then copy it and show it in a new window.
			 */
			IplImage* interest;
			cvSetImageROI(backup, selection);
			interest = cvCreateImage(cvGetSize(backup), backup->depth, backup->nChannels);
			cvCopy(backup, interest, NULL);
			cvResetImageROI(backup);

			/* 
			 * hack to make the selection rectangle disappear
			 * (copy a backup into the image being drawn)
			 *
			 * Breaks if the window is too small (oops?)
			 */
			cvCopy(backup, img, NULL);
			cvShowImage(filename, img_selection);

			cvNamedWindow("Region of Interest", CV_WINDOW_AUTOSIZE);
			cvShowImage("Region of Interest", interest);
		}

		// show the image in a window
		cvShowImage(filename, img_selection);

		/* 
		 * if ESCAPE is pressed, exit
		 * wait 10 milliseconds for mouse input
		 */
		if(cvWaitKey(10) == ESCAPE)
			break;
	}

	/*
	 * cleanup
	 */
	cvReleaseImage(&img);
	cvReleaseImage(&img_selection);
	cvDestroyWindow(filename);

	return 0;
}

void mouse_callback(int event, int x, int y, int flags, void* param) {

	IplImage* current_image = (IplImage*) param;

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

			show_selection(current_image);
			break;
		}
	}
}

/*
 * Draw a green rectangle on the image
 * 
 * When the user stops moving the mouse, consider the selection finished.
 */
void show_selection(IplImage* current_image) {
	cvRectangle(current_image, cvPoint(selection.x, selection.y), cvPoint(selection.x + selection.width, selection.y + selection.height), cvScalar(0x00, 0xFF, 0x00));
	if(!dragging)
		finished = true;
}
