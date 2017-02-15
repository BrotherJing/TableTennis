#include "Camera.h"
#include "Tools.h"

using namespace cv;
using namespace std;

CvCapture *captureLeft, *captureRight;

IplImage *frameLeft, *IsmallLeft, *frameRight, *IsmallRight;
IplImage *ImaskLeft, *ImaskSmallLeft, *ImaskRight, *ImaskSmallRight, *ImaskTemp, *ImaskSmallTemp;
IplImage *hsv[2];
IplImage *hue[2], *sat[2], *v[2];
IplImage *IsmallLeftHSV;
CvSize sz, szSmall;

int ma=125, lo=123, hi=126, vlo = 40, vhi = 215;
int loIdx[3], hiIdx[3];

int displayMode = DISPLAY_MODE_FIRST_FRAME;
vector<CvPoint> points;//record points drawn by user

void AllocImages(IplImage *frame){

	sz = cvGetSize(frame);
	szSmall.width = sz.width / SCALE; szSmall.height = sz.height / SCALE;

	IsmallLeft = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallRight = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	hsv[0] = cvCreateImage(sz, frame->depth, 3);
	hsv[1] = cvCreateImage(sz, frame->depth, 3);

	hue[0] = cvCreateImage(sz, frame->depth, 1);
	hue[1] = cvCreateImage(sz, frame->depth, 1);
	sat[0] = cvCreateImage(sz, frame->depth, 1);
	sat[1] = cvCreateImage(sz, frame->depth, 1);
	v[0] = cvCreateImage(sz, frame->depth, 1);
	v[1] = cvCreateImage(sz, frame->depth, 1);

	IsmallLeftHSV = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	ImaskLeft = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmallLeft = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
	ImaskRight = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmallRight = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
	ImaskTemp = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmallTemp = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
}

void DeallocateImages(){
	cvReleaseImage(&IsmallLeft); cvReleaseImage(&IsmallRight);
	cvReleaseImage(&hue[0]); cvReleaseImage(&hue[1]);
	cvReleaseImage(&hsv[0]); cvReleaseImage(&hsv[1]);
}

void onMouse(int event, int x, int y, int flag, void *ustc){
	IplImage *frame = (IplImage*)ustc;
	CvPoint pt = cvPoint(x, y);
	switch (event){
	case CV_EVENT_LBUTTONDOWN:
		points.push_back(pt);
		cvCircle(frame, pt, 5, CV_RGB(255, 255, 255));
		break;
	case CV_EVENT_RBUTTONDOWN:
		drawMask(points, ImaskSmallLeft);
		drawMask(points, frame);
		points.clear();
		break;
	}
}

void splitFrame(IplImage *frame, int cameraIdx){
	cvCvtColor(frame, hsv[cameraIdx], CV_BGR2HSV);
	cvSplit(hsv[cameraIdx], hue[cameraIdx], sat[cameraIdx], v[cameraIdx], 0);
}

void findTable(){
	splitFrame(frameLeft, 0);
	splitFrame(frameRight, 1);
	cvInRangeS(hue[0], cvScalar(lo), cvScalar(hi), ImaskLeft);
	cvInRangeS(hue[1], cvScalar(lo), cvScalar(hi), ImaskRight);
	cvInRangeS(v[0], cvScalar(40), cvScalar(215), ImaskTemp);
	cvAnd(ImaskLeft, ImaskTemp, ImaskLeft);
	cvInRangeS(v[1], cvScalar(40), cvScalar(215), ImaskTemp);
	cvAnd(ImaskRight, ImaskTemp, ImaskRight);
	cvResize(ImaskLeft, ImaskSmallLeft);
	cvResize(ImaskRight, ImaskSmallRight);
	cvShowImage("MaskLeft", ImaskSmallLeft);
	cvShowImage("MaskRight", ImaskSmallRight);
}

bool nextFrame(){
	frameLeft = cvQueryFrame(captureLeft);
	frameRight = cvQueryFrame(captureRight);
	if (!frameLeft || !frameRight)return false;
	if (displayMode == DISPLAY_MODE_FIRST_FRAME){
		displayMode = DISPLAY_MODE_PLAY;
		return true;
	}
	splitFrame(frameLeft, 0);
	splitFrame(frameRight, 1);
	cvResize(frameLeft, IsmallLeft);
	cvResize(frameRight, IsmallRight);
	return true;
}

int main(int argc, char **argv){

	bool paused = false;
	bool histShown = false;
	cv::Mat m_frame;

	namedWindow("CameraLeft", WINDOW_AUTOSIZE);
	namedWindow("CameraRight", WINDOW_AUTOSIZE);
	namedWindow("MaskLeft", WINDOW_AUTOSIZE);
	namedWindow("MaskRight", WINDOW_AUTOSIZE);
#ifdef DEBUG_MODE
	captureLeft = cvCreateFileCapture("0010R.MP4");
	captureRight = cvCreateFileCapture("0010L.MP4");
#else
	captureLeft = cvCreateFileCapture(argv[1]);
	captureRight = cvCreateFileCapture(argv[2]);
#endif
	nextFrame();

	AllocImages(frameLeft);

	while (1){
		if (displayMode == DISPLAY_MODE_PLAY){
			if (!nextFrame())break;
			findTable();
		}

		cvShowImage("CameraLeft", IsmallLeft);
		cvShowImage("CameraRight", IsmallRight);

		char c = cvWaitKey(20);
		if (c == 27)break;
		switch (c){
		case 'p'://pause
			if (displayMode == DISPLAY_MODE_PLAY)
				displayMode = DISPLAY_MODE_PAUSE;
			else if (displayMode == DISPLAY_MODE_PAUSE)
				displayMode = DISPLAY_MODE_PLAY;
			break;
		case 'd'://draw
			if (displayMode != DISPLAY_MODE_DRAW){
				displayMode = DISPLAY_MODE_DRAW;
				setMouseCallback("CameraLeft", onMouse, IsmallLeft);
			} else{
				displayMode = DISPLAY_MODE_PLAY;
				cvResize(frameLeft, IsmallLeft);
				cvCvtColor(IsmallLeft, IsmallLeftHSV, CV_BGR2HSV);
				getColorRange(IsmallLeftHSV, ImaskSmallLeft, &ma, &lo, &hi);
			}
			break;
		case 'c'://go to next frame and pause
			if (displayMode == DISPLAY_MODE_PAUSE){
				cvCvtColor(IsmallLeft, IsmallLeftHSV, CV_BGR2HSV);
				getColorRange(IsmallLeftHSV, ImaskSmallLeft, &ma, &lo, &hi);
				nextFrame();
			}
			break;
		case 's':
			cvSaveImage("SCREENSHOT.jpg", frameLeft);
		default:break;
		}
	}
	DeallocateImages();
	cvReleaseCapture(&captureLeft); cvReleaseCapture(&captureRight);
	cvDestroyWindow("CameraLeft");
	cvDestroyWindow("CameraRight");
	cvDestroyWindow("MaskLeft");
	cvDestroyWindow("MaskRight");
	return 0;
}
