//Code from : http://opencv-srf.blogspot.ro/, opencv tutorials
//Used to determine exact range for 'hue', 'saturation' and 'value' of an one colored object

#include "stdafx.h"

#include <cv.h>
#include <highgui.h>

int lowerH = 0;
int lowerS = 0;
int lowerV = 0;

int upperH = 180;
int upperS = 256;
int upperV = 256;

//This function threshold the HSV image and create a binary image
IplImage* GetThresholdedImage(IplImage* imgHSV){

	IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);
	cvInRangeS(imgHSV, cvScalar(lowerH, lowerS, lowerV), cvScalar(upperH, upperS, upperV), imgThresh);

	return imgThresh;

}

//This function create two windows and 6 trackbars for the "Ball" window
void setwindowSettings(){
	cvNamedWindow("Video");
	cvNamedWindow("Ball");

	cvCreateTrackbar("LowerH", "Ball", &lowerH, 180, NULL);
	cvCreateTrackbar("UpperH", "Ball", &upperH, 180, NULL);

	cvCreateTrackbar("LowerS", "Ball", &lowerS, 256, NULL);
	cvCreateTrackbar("UpperS", "Ball", &upperS, 256, NULL);

	cvCreateTrackbar("LowerV", "Ball", &lowerV, 256, NULL);
	cvCreateTrackbar("UpperV", "Ball", &upperV, 256, NULL);
}

int main(){
	CvCapture* capture = 0;

	capture = cvCaptureFromCAM(0);
	if (!capture){
		printf("Capture failure\n");
		return -1;
	}

	IplImage* frame = 0;

	setwindowSettings();

	//iterate through each frames of the video
	while (true){

		frame = cvQueryFrame(capture);
		if (!frame)  break;
		frame = cvCloneImage(frame);

		IplImage* imgHSV = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
		cvCvtColor(frame, imgHSV, CV_BGR2HSV); //Change the color format from BGR to HSV

		IplImage* imgThresh = GetThresholdedImage(imgHSV);

		cvShowImage("Ball", imgThresh);
		cvShowImage("Video", frame);

		//Clean up used images
		cvReleaseImage(&imgHSV);
		cvReleaseImage(&imgThresh);
		cvReleaseImage(&frame);

		//Wait 80mS
		int c = cvWaitKey(80);
		//If 'ESC' is pressed, break the loop
		if ((char)c == 27) break;

	}

	cvDestroyAllWindows();
	cvReleaseCapture(&capture);

	return 0;
}
