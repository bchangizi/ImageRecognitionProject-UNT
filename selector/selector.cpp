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
using namespace cv;

#include <iostream>
#include <fstream>
#include <deque>
#include <qthread.h>
using namespace std;

#include <ctime>
#include <string>
#include <stdlib.h>
#include <math.h>
#include "defines.h"

int main(int argc, char *argv[]) {

	Mat backup;
	bool usingFile = false;

	//Take an output file name from starting args
	string outputVidFile = "video.avi";
	
	VideoCapture cam;
	if(argc == 3) {
		usingFile = true;
		cam = VideoCapture(argv[1]);
		outputVidFile = argv[2];
		if( !outputVidFile.length() )
			outputVidFile = "video.avi";
		//output name matches input vid or an existing file with that name exists.
		if( outputVidFile == argv[1] ) {
			char preventConflict[55] = {0};
			sprintf( preventConflict, "video_%u.avi", (unsigned int)(time(NULL) ) );
			outputVidFile = preventConflict;
		}
	}
	else {
		cam = VideoCapture(0);
	}

	if(!cam.isOpened()) {
		cout << "Failed to open video capture." << endl;
		return 1;
	}

	/*
	 * Create the window and add a mouse event handler
	 */
	namedWindow("Stream", CV_WINDOW_AUTOSIZE);
	setMouseCallback("Stream", mouseCallback, (void*) &currentFrame);


	deque<Mat> pool;

	/*
	 * Output video
	 */
	VideoWriter output;
	//output.open() fails if that file already exists so we need to know it advance and rename the output.
	if( ifstream( outputVidFile ) ) {
		char preventConflict[55] = {0};
		sprintf( preventConflict, "video_%u.avi", (unsigned int)(time(NULL) ) );
		outputVidFile = preventConflict;
	}
	//Open up an output video file
	output.open( outputVidFile, CV_FOURCC('M','J','P','G'), 5, Size(640, 480), true);

	if(!output.isOpened()) {
		cout << "Failed to open video writer." << endl;
		return 1;
	}
		
	/*
	 * Simulate video stream
	 */
	while(running) {

		cam >> currentFrame;		
		Mat& img = currentFrame;
		if( currentFrame.empty() )
			continue;
		//Track our frames
		++frameCounter;

		backup = img.clone();

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
			Scalar color = (useEdges) ? Scalar(255, 255, 255) : Scalar(0, 0, 255);
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
			rectangle( img, selection, color, 1, 8, 0);
		}
		else if(finished ) {			
			//cout << "Before findSelection()" << endl;

			
			//Find
			finished = findSelection( img );
		}		

		/* 
		 * wait 10 milliseconds for keyboard input
		 */
		int keyCode = waitKey(10);
	
		/*
		 * If we are reading from a webcam, use pool size of 60, 
		 * otherwise read from the file normally.
		 *
		 * this prevents the stream from skipping the last 60 frames
		 * when using a video file
		 *
		 * find a better way to do this
		 */
		if( dragging || finished )
			output << img;

		imshow("Stream", img );		
		img.release();		

		/*if(pool.size() >= 60 && !usingFile) {
			imshow("Stream", pool.front());
			output.write(pool.front());
			pool.front().release();
			pool.pop_front();
		} */

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
			//finished = false;
			/*
			 * while dragging, the width and height of the 
			 * selection box will change
			 */
			if(dragging) {
				selection.width = ( x > current_image->cols ) ?  current_image->cols  : x  - selection.x;
				selection.height = ( y > current_image->rows ) ? current_image->rows : y - selection.y;
			}
			break;
		}
		case CV_EVENT_LBUTTONDOWN: {
			/* 
			 * left mouse button means start coordinates are selected
			 */
			if( x > current_image->cols || y > current_image->rows ) break;
			dragging = true;
			finished = false;
			selection = cvRect(x, y, 0, 0);
			break;
		}
		case CV_EVENT_LBUTTONUP: {
			//cout << "MouseUp" <<endl;
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
			if(selection.width < MIN_SIZE || selection.height < MIN_SIZE) {
				cout << "Selection isn't big enough." << endl;
				finished = false;
			} else {
				selectedImage = (*current_image)(selection);
				selectedImageKeypoints.clear();
				surf.detect( selectedImage, selectedImageKeypoints );
				if( selectedImageKeypoints.size() < 4) {
					cout << "Not enough keypoints in selection. Try again!" << endl;
					break;
				}
				surf.compute( selectedImage, selectedImageKeypoints, selectedImageDescriptors );
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
bool findSelection(Mat &image) {
	//cout << "findSelection intro" << endl;
	/*
	 * surf is defined in defines.h
	 */
	if( true) { //Calculate kp and descriptors each call, might find ways around doing this every call, in the future
		surf.detect( currentFrame, currentFrameKeypoints );
		surf.compute( currentFrame, currentFrameKeypoints,  currentFrameDescriptors);
	}

	if( !currentFrameKeypoints.size() )
	{
		cout << "No key points were detected so we can't proceed!" << endl;
		return true;
	}

	cout << currentFrameKeypoints.size() << " keypoints in scene. " << selectedImageKeypoints.size() << " keypoints in object." << endl;

	/*
	 * Match the descriptor vectors using FLANN
	 */
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	if( currentFrameDescriptors.rows > 0 && currentFrameDescriptors.cols > 0 &&
	   selectedImageDescriptors.rows > 0 && selectedImageDescriptors.cols > 0) 
	   matcher.match(selectedImageDescriptors, currentFrameDescriptors, matches);
	else
		return false;

	/*
	 * Defines the largest distance between matching points.
	 */
	double max_dist = 0, min_dist = 100;

	for(int i = 0; i < matches.size(); i++) {
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
	for(int i = 0; i < selectedImageDescriptors.rows; i++) {
		if(matches[i].distance < 3 * min_dist) {
			good.push_back(matches[i]);
		}
	}	

	cout << good.size() << " good matches." << endl;
	/*
	 * No good matches, nothing to do.
	 */
	if( good.size() < 4 ) //findHomography needs atleast 4 good matches to work.
		return true;

	Mat sceneWithMatches = image.clone();
	//drawMatches( selectedImage, selectedImageKeypoints, currentFrame, currentFrameKeypoints, good, sceneWithMatches, Scalar::all(255), Scalar::all(0), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
	/* Feature detection code from opencv here about finding good matches and homography to find a 2d object in 3d space */
	//Localize object
	vector<Point2f> obj, scene;
	for( int i = 0; i < good.size(); ++i) {
		obj.push_back( selectedImageKeypoints[ good[i].queryIdx ].pt );
		scene.push_back( currentFrameKeypoints[ good[i].trainIdx ].pt );
	}
	//cout << "Before homography" << endl;
	//find perspective transform using 'findHomography'
	Mat homographyByProduct = findHomography( obj, scene, CV_RANSAC );

	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint( selectedImage.cols, 0);
	obj_corners[2] = cvPoint( selectedImage.cols, selectedImage.rows);
	obj_corners[3] = cvPoint( 0, selectedImage.rows );

	//Using the coordinates above, we translate that to our scene in order to locate our object's coordinates in the scene.
	//cout << "Before perspectiveTransform" << endl;
	perspectiveTransform( obj_corners, scene_corners, homographyByProduct );
	//cout << "After perspectiveTransform" << endl;
	
	//Draw our rectangle around our possible found object
	line( sceneWithMatches, scene_corners[0] , scene_corners[1] , Scalar(0, 0, 255), 4 );
	line( sceneWithMatches, scene_corners[1] , scene_corners[2] , Scalar(0, 0, 255), 4 );
	line( sceneWithMatches, scene_corners[2] , scene_corners[3] , Scalar(0, 0, 255), 4 );
	line( sceneWithMatches, scene_corners[3] , scene_corners[0] , Scalar(0, 0, 255), 4 );


	//Modify input image w/ one that has rectangle
	sceneWithMatches.copyTo(image);

	return true;
}
