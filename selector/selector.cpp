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
	setMouseCallback(filename, mouseCallback, (void*) &img);

	reduceNoise = maxContrast = useEdges = true;

	/*
	 * Search is the matrix that we search for in the current image
	 */
	Mat search;

	/*
	 * Simulate video stream
	 */
	while(running) {
		/*
		 * Reset the image so drawing a rectangle is animated
		 */
		backup.copyTo(img);

	// if(useEdges) {
		// }

		/*
		 * Show help LAST or it will be affected by noise reduction/edge detection
		 */
		if(help) {
			/*
			 * Edge map uses grayscale. 
			 * Set the font color to white so the help menu is visible.
			 */
			Scalar color = (useEdges) ? Scalar(255, 255, 2525) : Scalar(0, 0, 255);
			/*
			 * Create newlines by drawing 30 pixels lower
			 */
			for(int k = 0; k < HELP_LENGTH; k++) {
				putText(img, helpArray[k], Point(10, 30 + (k * 30)), 
					FONT_HERSHEY_COMPLEX_SMALL, 0.8, color, 1, CV_AA);
			}
		}

		if(dragging) {
			/*
			 * Draw the current mouse selection
			 */
			Scalar rect_color = (useEdges) ? Scalar(255, 255, 2525) : Scalar(0, 255, 0);
			rectangle(img, selection, rect_color, 1, 8, 0);
		}
		else if(finished) {
			finished = false;
			
			/*
			 * First verify that the selection is large enough or the program will crash
			 */
			if(selection.width < 10 || selection.height < 10)
				continue;

			Mat subimage = img(selection);
			Rect result;

			// Try to find subimage in backup, and store the result in result
			findSelection(backup, subimage, result);

			if(result.width > 20 && result.height && 20) {
				namedWindow("Region of Interest", CV_WINDOW_AUTOSIZE);
				imshow("Region of Interest", backup(result));
			}
		}

		// show the image in a window
		imshow(filename, img);

		/* 
		 * wait 10 milliseconds for keyboard input
		 */
		int keyCode = cvWaitKey(10);

		switch(keyCode) {
			/*
			 * Press q to quit
			 */
			case 'q': {
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
				useEdges = !useEdges;
				break;
		  	}
			/*
			 * Press r to toggle noise reduction
			 */
			case 'r': {
				reduceNoise = !reduceNoise;
				break;
			}
			/*
			 * Press c to toggle contrast maximization
			 */
			case 'c': {
				maxContrast = !maxContrast;
				break;
			}
		}
	}

	destroyAllWindows();

	return 0;
}

void mouseCallback(int event, int x, int y, int flags, void* param) {
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
void maximizeContrast(Mat &img) {
	Mat channels[3];
	split(img, channels);
	for(int k = 0; k < 3; k++)
		equalizeHist(channels[k], channels[k]);
	merge(channels, 3, img);
}

/*
 * Given a large image &img, find &subimage.
 * Use the SURF algorithm to find unique points.
 *
 * For best results, compute edges after
 * noise reduction and contrast maximization.
 */
void findSelection(Mat &img, Mat &subimage, Rect &result) {
	Mat canvas;
	img.copyTo(canvas);

	maximizeContrast(img);
	medianBlur(img, img, 3);

	/*
	 * Scene is the image
	 * Object is the selection 
	 */
	Mat scene, object;
	cvtColor(img, scene, CV_BGR2GRAY);
	cvtColor(subimage, object, CV_BGR2GRAY);
	Canny(scene, scene, 120, 480, 3);
	Canny(object, object, 120, 480, 3);

	/*
	 * Calculate key points using SURF
	 */
	SurfFeatureDetector surf(400);
	vector<KeyPoint> scene_points, object_points;
	surf.detect(scene, scene_points);
	surf.detect(object, object_points);

	/*
	 * Calculate descriptors
	 */
	SurfDescriptorExtractor extractor;
	Mat scene_descriptor, object_descriptor;
	extractor.compute(scene, scene_points, scene_descriptor);
	extractor.compute(object, object_points, object_descriptor);

	scene_descriptor.convertTo(scene_descriptor, CV_32F);
	object_descriptor.convertTo(object_descriptor, CV_32F);

	/*
	 * Match the descriptor vectors using FLANN
	 */
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	matcher.match(scene_descriptor, object_descriptor, matches);

	double max_dist = 0, min_dist = 50;

	for(int i = 0; i < scene_descriptor.rows; i++) {
		double dist = matches[i].distance;
		if(dist < min_dist)
			min_dist = dist;
		if(dist> max_dist)
			max_dist = dist;
	}

	/*
	 * Find the matches that are less than 5 * min_dist apart
	 */
	vector<DMatch> good;
	for(int i = 0; i < scene_descriptor.rows; i++) {
		if(matches[i].distance < 2 * min_dist) {
			good.push_back(matches[i]);
		}
	}

	/*
	 * Put the good matches into the scene and object vectors
	 */
	float curr_x, curr_y;
	float max_x = 0, max_y = 0;
	float min_x = scene.cols, min_y = scene.rows;

	for(int i = 0; i < good.size(); i++) {
		curr_x = scene_points[good[i].queryIdx].pt.x;
		curr_y = scene_points[good[i].queryIdx].pt.y;
		if(curr_x * curr_y != 0) {
			max_x = (curr_x > max_x) ? curr_x : max_x;
			max_y = (curr_y > max_y) ? curr_y : max_y;

			min_x = (curr_x < min_x) ? curr_x : min_x;
			min_y = (curr_y < min_y) ? curr_y : min_y;
		}
	}

	cout << "Possible coordinates of match: [" << min_x << "," << min_y << "] to ["
		 << max_x << "," << max_y << "]" << endl;

	result.x = min_x;
	result.y = min_y;
	result.height = max_y - min_y;
	result.width= max_x - min_x;

	cout << "Image width: " << min_x + max_y << "\nImage height: " << min_y + max_y << endl;

	Mat img_matches;

	// for(int k = 0; k < good.size(); k++) {
	// 	circle(canvas, scene_points[good[k].queryIdx].pt, 10, Scalar::all(255), 1, 8, 0);
	// }

	// namedWindow("Matches", CV_WINDOW_AUTOSIZE);
	// imshow("Matches", canvas);
}
