#include "stdafx.h"
#include <sstream>
#include <string>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <Windows.h>
#include <math.h>
#include "SerialClass.h"

using namespace cv;
using namespace std;

// H,S and V for the wanted object
int Hmin = 95;
int Smin = 95;
int Vmin = 83;

int Hmax = 127;
int Smax = 256;
int Vmax = 149;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

string intToString(int number){


	std::stringstream ss;
	ss << number;
	return ss.str();
}

//This function threshold the HSV image and create a binary image
IplImage* GetThresholdedImg(IplImage* imgHSV){

	IplImage* imgThreshed = cvCreateImage(cvGetSize(imgHSV), /*IPL_DEPTH_8U*/8, 1);
	cvInRangeS(imgHSV, cvScalar(Hmin, Smin, Vmin), cvScalar(Hmax, Smax, Vmax), imgThreshed);

	return imgThreshed;

}

// the tracked positions 
static int posX = 0;
static int posY = 0;



// Serial to Arduino global declarations
Serial *arduin;
char MSB = 0;
char LSB = 0;
// Serial to Arduino global declarations

int main(){

	try {
		
		//capture image from the cam 
		CvCapture* capture = 0;

		capture = cvCaptureFromCAM(1); //select the num of the cam
		if (!capture){
			printf("Capture failure\n");
			return -1;
		}

		// serial to Arduino setup 
		arduin = new Serial("\\\\.\\COM3");    // adjust as needed

		if (arduin->IsConnected())
			printf("We're connected\n"); //for debug
		// serial to Arduino setup 


		//iterate through each frames of the video 
		while (true){

			IplImage* frame = 0;
			frame = cvQueryFrame(capture);
			if (!frame)  break;

			frame = cvCloneImage(frame);

			IplImage* imgHSV = cvCreateImage(cvGetSize(frame), /*IPL_DEPTH_8U*/8, 3);
			cvCvtColor(frame, imgHSV, CV_BGR2HSV); //Change the color format from BGR to HSV

			IplImage* imgThresh = GetThresholdedImg(imgHSV);

			// Calculate the moments to estimate the position of the object
			CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
			cvMoments(imgThresh, moments, 1);

			// The actual moment values
			double m10 = cvGetSpatialMoment(moments, 1, 0);
			double m01 = cvGetSpatialMoment(moments, 0, 1);
			double area = cvGetCentralMoment(moments, 0, 0);
			//printf("moment10 = %.2f\n, moment01 = %.2f\n, area = %.2f\n", moment10, moment01, area);

			// Holding the current positions

			posX = m10 / area;
			posY = m01 / area;

			// Print it out for debugging purposes
			printf("position (%d,%d)\n", posX, posY);
			printf("Position : %d, et taille : %.2f\n", posX - 320, area); // (taille écran = 640)

			//envoie de la position
			LSB = posX & 0xff;
			MSB = (posX >> 8) & 0xff;
			
			arduin->WriteData(&MSB, 1);
			arduin->WriteData(&LSB, 1);

			//envoie de la taille
			LSB = (int)area & 0xff; //printf("%u\n", LSB);
			MSB = ((int)area >> 8) & 0xff; //printf("%u\n", MSB);

			arduin->WriteData(&MSB, 1);
			arduin->WriteData(&LSB, 1);


			//libère la place + attend 1/2 seconde
			delete moments;
			Sleep(500);
			//Wait 200mS
			//int c = cvWaitKey(200);
			//If 'ESC' is pressed, break the loop
			//if ((char)c == 27) break;

		}

		// Serial to Arduino - shutdown
		delete arduin;
		arduin = 0;
		// Serial to Arduino - shutdown

		return 0;
	}
	catch (Exception &e)
	{
		cout << e.msg << endl;
	}
}
