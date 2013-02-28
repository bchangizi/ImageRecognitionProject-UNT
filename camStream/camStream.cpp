#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {

	//Create a video capture w/ default attached web cam;
	VideoCapture cam(0);
	if( !cam.isOpened() )  {
		cout << "Failed to open default cam" << endl;
		getchar();
	}
	else {
		//capture a frame from the cam and store it in frame;
		Mat currentCamFrame;
		namedWindow("Press any key to exit", CV_WINDOW_AUTOSIZE );
		while(true) {
			//simply display it
			cam >> currentCamFrame;
			imshow("Press any key to exit", currentCamFrame);
			//30fps
			if( waitKey(33) >= 0 ) break;
		}
	}
	return 0;
}