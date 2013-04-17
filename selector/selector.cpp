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

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>
#include "qthread.h"
using namespace cv;

#include <iostream>
#include <fstream>
#include <deque>
using namespace std;

#include <QThread>

#include <math.h> 
#include "defines.h"

int main(int argc, char *argv[]) {

	Mat img, backup;
	// Qt installation test code - QThread * thread = new QThread();
	
	VideoCapture cam(0);
	if(!cam.isOpened()) {
		cout << "Failed to open default camera." << endl;
		return 1;
	}

	/*
	 * Create the window and add a mouse event handler
	 */
	namedWindow("Stream", CV_WINDOW_AUTOSIZE);
	setMouseCallback("Stream", mouseCallback, (void*) &img);

	/*
	 * subimage is the matrix that we search for in the current image
	 * subimageDim is the width and height that the selection will be resized to
	 */
	Mat subimage, searchMat;
	
	/*
	 * Location of the searchMat in the image
	 */
	Rect searchRect;

	deque<Mat> pool;

	int numFound = 0, imgCount = 0;

	/*
	 * Output video
	 */
	VideoWriter output;
	output.open("video.avi", CV_FOURCC('M','J','P','G'), 20,
				Size(640, 480), true);

	if(!output.isOpened()) {
		cout << "Failed to open video writer." << endl;
		return 1;
	}
		
	/*
	 * Simulate video stream
	 */
	while(running) {

		if(reduceNoise) {
			medianBlur(img, img, 3);
		}

		if(maxContrast) {
			maximizeContrast(img, img);
		}

		if(useEdges) {
			Canny(img, img, 100, 400);
		}

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
			 * Draw the rectangle
			 */
			Scalar color = (useEdges) ? Scalar(255, 255, 255) : Scalar(0, 255, 0);
			Vec3b dragStart = pool.front().at<Vec3b>(selection.x, selection.y);
			rectangle(pool.front(), selection, color, 1, 8, 0);
		}
		else if(finished) {
			finished = false;
			/*
			 * First verify that the selection is large enough or the program will crash
			 * defines.h defines MIN_SIZE
			 */
			if(selection.width < MIN_SIZE || selection.height < MIN_SIZE)
				continue;

			selection.x = (selection.x < 0) ? 0 : selection.x;
			selection.y = (selection.y < 0) ? 0 : selection.y;

			subimage = img(selection);

			Rect result;
			// Try to find subimage in backup, and store the result in result
			findSelection(backup, subimage, result);

			/*
			 * If findSelection finds a match smaller than MIN_SIZE, ignore it
			 */
			if(result.width > MIN_SIZE && 
			   result.height > MIN_SIZE && 
			   result.width < img.cols && 
			   result.height < img.rows) {

				searchMat = img(result);
				searchRect = result;
			}
		}

		/*
		 * The image may change since the last selection was found,
		 * use the new image to catch cases where the object may be rotating
		 * or otherwise changing orientation.
		 */
		if(searchMat.rows > 20 && searchMat.cols > 20) {
			Rect tempRect = searchRect;
			findSelection(backup, searchMat, searchRect);
			if(tempRect == searchRect) {
			// if(tempRect == searchRect && numFound >= 30) {
				searchMat = Mat();
				searchRect = Rect();
				numFound = 0;
			}
			else {
				cout << "found " << numFound + 1 << " matches." << endl;
				numFound++;
			}
		}

		/*
		 * Draw the rectangle selection (the object found by findSelection)
		 */
		if(searchRect.x && searchRect.y && searchRect.width && searchRect.height) {
			Scalar color = (useEdges) ? Scalar(255, 255, 255) : Scalar(0, 255, 0);
			rectangle(pool.front(), searchRect, color, 1, 8, 0);
		}		

		cam >> img;
		//Make a copy of the image.
		pool.push_back( img.clone() );
		backup = img.clone();
		imgCount++;
		
		/* 
		 * wait 10 milliseconds for keyboard input
		 */
		int keyCode = waitKey(10);
		
		if( pool.size() >= 60 ) {
			
			imshow("Stream", pool.front());
			output.write(pool.front());
			pool.front().release();
			pool.pop_front();
		}

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

/*
 * Called when a mouse event occurs.
 */
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
				selection.width = ( x > current_image->cols ? current_image->cols : x ) - selection.x;
				selection.height = ( y > current_image->rows ? current_image->rows : y )- selection.y;
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
void maximizeContrast(Mat &input, Mat &output) {
	Mat channels[3];
	split(input, channels);
	for(int k = 0; k < 3; k++)
		equalizeHist(channels[k], channels[k]);
	merge(channels, 3, output);
}

/*
 * Given a large image &img, find &subimage.
 * Use the SURF algorithm to find unique points.
 *
 * For best results, compute edges after
 * noise reduction and contrast maximization.
 */
void findSelection(Mat &image, Mat &subimage, Rect &result) {
	/*
	 * surf is defined in defines.h
	 */
	vector<KeyPoint> scenePoints, objectPoints;
	surf.detect(image, scenePoints);
	surf.detect(subimage, objectPoints);

	/*
	 * Calculate descriptors
	 *
	 * this is the slowest part
	 * find a way to speed it up
	 *
	 * extractor is defined in defines.h
	 */
	Mat sceneDesc, objectDesc;
	extractor.compute(image, scenePoints, sceneDesc);
	extractor.compute(subimage, objectPoints, objectDesc);

	/*
	 * Match the descriptor vectors using FLANN
	 */
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	if(sceneDesc.rows > 0 && sceneDesc.cols > 0 &&
	   objectDesc.rows > 0 && objectDesc.cols > 0) 
		matcher.match(sceneDesc, objectDesc, matches);
	else
		return;

	/*
	 * Defines the largest distance between matching points.
	 */
	double max_dist = 0, min_dist = 50;

	for(int i = 0; i < sceneDesc.rows; i++) {
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
	for(int i = 0; i < sceneDesc.rows; i++) {
		if(matches[i].distance < 5 * min_dist) {
			good.push_back(matches[i]);
		}
	}

	/*
	 * Put the good matches into the scene and object vectors
	 */
	float curr_x, curr_y;
	float max_x = 0, max_y = 0;
	float min_x = image.cols, min_y = image.rows;

	/*
	 * No good matches, nothing to do.
	 */
	if(!good.size())
		return;

	/*
	 * Find the outer points of the selection.
	 */
	for(int i = 0; i < good.size(); i++) {
		curr_x = scenePoints[good[i].queryIdx].pt.x;
		curr_y = scenePoints[good[i].queryIdx].pt.y;
		if(curr_x * curr_y != 0) {
			if(curr_x > max_x)
				max_x = curr_x;
			if(curr_y > max_y)
				max_y = curr_y;

			if(curr_x < min_x)
				min_x = curr_x;
			if(curr_y < min_y)
				min_y = curr_y;
		}
	}

	/*
	 * Calculate the "middle" of the cluster of points
	 * Width is the "average" of the selection, 
	 * ((width - x) / 2) + ((height - y) / 2)) / 2
	 */
	float width = 40; 
	float height = 40;
	float mid_x = ((min_x + max_x) / 2);
	float mid_y = ((min_y + max_y) / 2);

	/*
	 * Dimensions of the object that was found.
	 */
	result.x = mid_x - 20;
	result.y = mid_y - 20;
	result.width = width;
	result.height= height;

	// result.x = min_x;
	// result.y = min_y;
	// result.width = max_x - min_x;
	// result.height = max_y - min_y;
}
