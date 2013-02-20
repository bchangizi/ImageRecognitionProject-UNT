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

#include "defines.h"

int main(int argc, char *argv[]) {

	const char* filename = "lenaN.tif";
	Mat img, backup;

	/*
	 * Read the image 
	 * Copy to a backup
	 */
	img = imread(filename, CV_LOAD_IMAGE_COLOR);
	backup = img.clone();

	/*
	 * Create the window and add a mouse event handler
	 */
	namedWindow(filename, CV_WINDOW_AUTOSIZE);
	setMouseCallback(filename, mouse_callback, (void*) &img);

	while(running) {
		/*
		 * Reset the image so drawing a rectangle is animated
		 */
		backup.copyTo(img);

		/*
		 * Reduce noise first
		 */
		if(reduce_noise) {
			medianBlur(img, img, 3);
		}

		if(max_contrast) {
			maximize_contrast(img);
		}

		/*
		 * For best results, compute edges after
		 * noise reduction and contrast maximization.
		 */
		if(use_edges) {
			detect_edges(img);
		}

		/*
		 * Show help LAST or it will be affected by noise reduction/edge detection
		 */
		if(help) {
			/*
			 * Create newlines by drawing 30 pixels lower
			 */
			for(int k = 0; k < HELP_LENGTH; k++) {
				putText(img, help_array[k], Point(10, 30 + (k * 30)), 
					FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(0, 0, 255), 1, CV_AA);
			}
		}

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
		
			Mat interest = img(selection);
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
				help = !help;
				break;
			}
			/*
			 * Press e to toggle edge map
			 */
			case 'e': {
				use_edges = !use_edges;
				break;
		  	}
			/*
			 * Press r to toggle noise reduction
			 */
			case 'r': {
				reduce_noise = !reduce_noise;
				break;
			}
			/*
			 * Press c to toggle contrast maximization
			 */
			case 'c': {
				max_contrast = !max_contrast;
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
			/*
			 * while dragging, the width and height of the 
			 * selection box will change
			 */
			if(dragging) {
				selection.width = x - selection.x;
				selection.height = y - selection.y;
			}
			break;
		}
		case CV_EVENT_LBUTTONDOWN: {
			/* 
			 * left mouse button means start coordinates are selected
			 */
			dragging = true;
			selection = cvRect(x, y, 0, 0);
			break;
		}
		case CV_EVENT_LBUTTONUP: {
			/*
			 * left button up means dragging is done, 
			 * draw the selection
			 */
			dragging = false;
			finished = true;

			/*
			 * if the user dragged up or to the left
			 * we cant have a negative box, so shift
			 * x or y accordingly
			 */
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

/*
 * Maximize the contrast by equalizing 
 * the histograms of BGR channels
 */
void maximize_contrast(Mat &img) {
	Mat channels[3];
	split(img, channels);
	for(int k = 0; k < 3; k++)
		equalizeHist(channels[k], channels[k]);
	merge(channels, 3, img);
}

/*
 * Show edges using Sobel edge detection
 */
void detect_edges(Mat &img) {
	/*
	 * Begin Sobel edge detection
	 */
	Mat gradientX, gradientY, absGradientX, absGradientY, gray;

	/*
	 * Copy the blurred image (easier edge detection) and convert to grayscale
	 */
	cvtColor(img, gray, CV_BGR2GRAY);

	/*
	 * OpenCV sobel edge detection, using X and Y gradients.
	 */
	Sobel(gray, gradientX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(gray, gradientY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(gradientX, absGradientX);
	convertScaleAbs(gradientY, absGradientY);
	addWeighted(absGradientX, 0.5, absGradientY, 0.8, 0, img);
}
