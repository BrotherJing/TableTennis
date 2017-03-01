#include "Camera.h"
#include "Tools.h"

using namespace cv;
using namespace std;

//two cameras
CvCapture *captureLeft, *captureRight;

IplImage *frameLeft, *IsmallLeft, *frameRight, *IsmallRight;
IplImage *ImaskLeft, *ImaskSmallLeft, *ImaskRight, *ImaskSmallRight, *ImaskTemp, *ImaskSmallTemp;
IplImage *IsmallLeftHSV;
CvSize sz, szSmall;
CvRect bbox;

//for computing perspective matrix
CvPoint2D32f ptsLeft[4], ptsRight[4];
CvMat *HLeft = cvCreateMat(3, 3, CV_32F);
CvMat *HRight = cvCreateMat(3, 3, CV_32F);

int displayMode = DISPLAY_MODE_FIRST_FRAME;

//for extracting table area
int ma = 125, lo = 123, hi = 127, vlo = 40, vhi = 215;

//record points drawn by user
vector<CvPoint> points;

void AllocImages(IplImage *frame){

	sz = cvGetSize(frame);
	szSmall.width = sz.width / SCALE; szSmall.height = sz.height / SCALE;

	IsmallLeft = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallRight = cvCreateImage(szSmall, frame->depth, frame->nChannels);
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

bool nextFrame(){
	frameLeft = cvQueryFrame(captureLeft);
	frameRight = cvQueryFrame(captureRight);
	if (!frameLeft || !frameRight)return false;
	if (displayMode == DISPLAY_MODE_FIRST_FRAME){
		displayMode = DISPLAY_MODE_PLAY;
		return true;
	}
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
			cvShowImage("CameraLeft", IsmallLeft);
			cvShowImage("CameraRight", IsmallRight);
			findTableArea(frameLeft, ImaskLeft, lo, hi, vlo, vhi);
			findTable(ImaskLeft, &bbox);
			findEdges(frameLeft, ImaskLeft);
			cvResize(ImaskLeft, ImaskSmallLeft);
			bool left = findVertices(ImaskSmallLeft, ptsLeft);
			cvShowImage("MaskLeft", ImaskSmallLeft);

			findTableArea(frameRight, ImaskRight, lo, hi, vlo, vhi);
			findTable(ImaskRight, &bbox);
			findEdges(frameRight, ImaskRight);
			cvResize(ImaskRight, ImaskSmallRight);
			bool right = findVertices(ImaskSmallRight, ptsRight);
			cvShowImage("MaskRight", ImaskSmallRight);
			if (left&&right){
				//TODO
			}
		}

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
