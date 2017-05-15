/*
=========================================
required input:
- video(left right)

output:
- Intrinsics.xml
- Distortion.xml
- Rotation.xml
- Translation.xml

usage:
./TableTennis left.mp4 right.mp4 [intrinsic matrix] [distortion coeffs]
first press 'l', to add line hint. then draw an area of the table, then press 'd'.
=========================================
*/

#include "Camera.h"
#include "Tools.h"

using namespace cv;
using namespace std;

//two cameras
CvCapture *captureLeft, *captureRight;

IplImage *frameLeft, *IsmallLeft, *frameRight, *IsmallRight, *frameTemp;
IplImage *draw;
IplImage *ImaskLeft, *ImaskSmallLeft, *ImaskRight, *ImaskSmallRight, *ImaskTemp, *ImaskSmallTemp;
IplImage *IsmallLeftHSV, *IsmallRightHSV;
CvSize sz, szSmall;
CvRect bbox;

//for computing perspective matrix
CvPoint2D32f ptsLeft[4], ptsRight[4];
CvMat *HLeft = cvCreateMat(3, 3, CV_32F);
CvMat *HRight = cvCreateMat(3, 3, CV_32F);
CvMat *intrinsicMatrix = cvCreateMat(3, 3, CV_32FC1);
CvMat *distortionCoeffs = cvCreateMat(5, 1, CV_32FC1);
CvMat *rotationVectors = cvCreateMat(2, 3, CV_32FC1);
CvMat *translationVectors = cvCreateMat(2, 3, CV_32FC1);
bool calibrated = false;

int displayMode = DISPLAY_MODE_FIRST_FRAME;

//for extracting table area
//int ma = 125, lo = 123, hi = 127, vlo = 40, vhi = 215;
int vlo = 0, vhi = 150;
int ma[2], lo[2], hi[2];
int lo0[3], hi0[3], lo1[3], hi1[3];

//record points drawn by user
vector<CvPoint> points, pointsRight;

void AllocImages(IplImage *frame){

	sz = cvGetSize(frame);
	szSmall.width = sz.width / SCALE; szSmall.height = sz.height / SCALE;

	IsmallLeft = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallRight = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallLeftHSV = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallRightHSV = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	draw = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	ImaskLeft = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmallLeft = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
	ImaskRight = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmallRight = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
	ImaskTemp = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmallTemp = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);

	frameTemp = cvCreateImage(sz, frame->depth, frame->nChannels);
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
		cvZero(ImaskSmallLeft);
		drawMask(points, ImaskSmallLeft);
		drawMask(points, frame);
		points.clear();
		break;
	}
}

void onMouseRight(int event, int x, int y, int flag, void *ustc){
	IplImage *frame = (IplImage*)ustc;
	CvPoint pt = cvPoint(x, y);
	switch (event){
	case CV_EVENT_LBUTTONDOWN:
		pointsRight.push_back(pt);
		cvCircle(frame, pt, 5, CV_RGB(255, 255, 255));
		break;
	case CV_EVENT_RBUTTONDOWN:
		cvZero(ImaskSmallRight);
		drawMask(pointsRight, ImaskSmallRight);
		drawMask(pointsRight, frame);
		pointsRight.clear();
		break;
	}
}

CvPoint lines[4][2], linesRight[4][2];
int nLine=0, nLineRight=0;
bool isDrawing;
void onDrawLine(int event, int x, int y, int flag, void *ustc){
	IplImage *frame = (IplImage*)ustc;
	CvPoint pt = cvPoint(x, y);
	if(nLine>=4)return;
	if(event==CV_EVENT_LBUTTONDOWN){
		isDrawing = true;
		lines[nLine][0] = pt;
	}else if(event==CV_EVENT_MOUSEMOVE){
		if(isDrawing){
			cvCopy(frame, draw);
			cvLine(draw, lines[nLine][0], pt, CVX_RED, 1, CV_AA, 0);
			cvShowImage("CameraLeft", draw);
		}
	}else if(event==CV_EVENT_LBUTTONUP){
		isDrawing = false;
		lines[nLine][1] = pt;
		cvLine(frame, lines[nLine][0], lines[nLine][1], CVX_RED, 1, CV_AA, 0);
		cvShowImage("CameraLeft", frame);
		nLine++;
	}
}
void onDrawLineRight(int event, int x, int y, int flag, void *ustc){
	IplImage *frame = (IplImage*)ustc;
	CvPoint pt = cvPoint(x, y);
	if(nLineRight>=4)return;
	if(event==CV_EVENT_LBUTTONDOWN){
		isDrawing = true;
		linesRight[nLineRight][0] = pt;
	}else if(event==CV_EVENT_MOUSEMOVE){
		if(isDrawing){
			cvCopy(frame, draw);
			cvLine(draw, linesRight[nLineRight][0], pt, CVX_RED, 1, CV_AA, 0);
			cvShowImage("CameraRight", draw);
		}
	}else if(event==CV_EVENT_LBUTTONUP){
		isDrawing = false;
		linesRight[nLineRight][1] = pt;
		cvLine(frame, linesRight[nLineRight][0], linesRight[nLineRight][1], CVX_RED, 1, CV_AA, 0);
		cvShowImage("CameraRight", frame);
		nLineRight++;
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

	captureLeft = cvCreateFileCapture(argv[1]);
	captureRight = cvCreateFileCapture(argv[2]);

	if(argc>3){
		Mat intrinsicMatrixDouble((CvMat*)cvLoad(argv[3]));
		Mat distortionCoeffsDouble((CvMat*)cvLoad(argv[4]));
		Mat intrinsicMatrixSingle, distortionCoeffsSingle;
		intrinsicMatrixDouble.convertTo(intrinsicMatrixSingle, CV_32FC1);
		distortionCoeffsDouble.convertTo(distortionCoeffsSingle, CV_32FC1);
		CvMat temp1 = intrinsicMatrixSingle, temp2 = distortionCoeffsSingle;
		cvCopy(&temp1, intrinsicMatrix);
		cvCopy(&temp2, distortionCoeffs);
	}

	nextFrame();

	AllocImages(frameLeft);
	
	cvResize(frameLeft, IsmallLeft);
	cvResize(frameRight, IsmallRight);
	cvShowImage("CameraLeft", IsmallLeft);
	cvShowImage("CameraRight", IsmallRight);
	displayMode = DISPLAY_MODE_DRAW;
	setMouseCallback("CameraLeft", onMouse, IsmallLeft);
	setMouseCallback("CameraRight", onMouseRight, IsmallRight);

	while (1){
		if (displayMode == DISPLAY_MODE_PLAY){
			if (!nextFrame())break;
			cvShowImage("CameraLeft", IsmallLeft);
			cvShowImage("CameraRight", IsmallRight);
			//findTableArea(frameLeft, ImaskLeft, lo, hi, vlo, vhi);
			findTableArea(frameLeft, ImaskLeft, lo[0], hi[0], vlo, vhi);
			findTable(ImaskLeft, &bbox);
			findEdges(frameLeft, ImaskLeft);
			bool left = findVertices(ImaskSmallLeft, ptsLeft, lines, nLine);
			cvShowImage("MaskLeft", ImaskSmallLeft);
			//findTableArea(frameRight, ImaskRight, lo, hi, vlo, vhi);
			findTableArea(frameRight, ImaskRight, lo[1], hi[1], vlo, vhi);
			findTable(ImaskRight, &bbox);
			findEdges(frameRight, ImaskRight);
			cvResize(ImaskRight, ImaskSmallRight);
			bool right = findVertices(ImaskSmallRight, ptsRight, linesRight, nLineRight);
			cvShowImage("MaskRight", ImaskSmallRight);
			if (left&&right&&!calibrated){
				calibrateCameraUseGuess(ptsLeft, ptsRight, cvGetSize(ImaskLeft), intrinsicMatrix, distortionCoeffs, rotationVectors, translationVectors);
				calibrated = true;
			}
			if (calibrated){
				cvUndistort2(frameLeft, frameTemp, intrinsicMatrix, distortionCoeffs);
				cvResize(frameTemp, IsmallLeftHSV);
				cvShowImage("CameraLeft", IsmallLeftHSV);
				cvUndistort2(frameRight, frameTemp, intrinsicMatrix, distortionCoeffs);
				cvResize(frameTemp, IsmallLeftHSV);
				cvShowImage("CameraRight", IsmallLeftHSV);
				char c = cvWaitKey(0);
				if (c == KEY_ENTER||c==KEY_RETURN){
					cvSave("Intrinsics.xml", intrinsicMatrix);
					cvSave("Distortion.xml", distortionCoeffs);
					cvSave("Rotation.xml", rotationVectors);
					cvSave("Translation.xml", translationVectors);
					cout << "camera matrix saved!" << endl;
				}
				else calibrated = false;
			}
		}else{
			cvShowImage("CameraLeft", IsmallLeft);
			cvShowImage("CameraRight", IsmallRight);
		}

		char c = cvWaitKey(20);
		if (c == KEY_ESC)break;
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
				setMouseCallback("CameraRight", onMouseRight, IsmallRight);
			} else{
				displayMode = DISPLAY_MODE_PLAY;
				cvResize(frameLeft, IsmallLeft);
				cvResize(frameRight, IsmallRight);
				cvCvtColor(IsmallLeft, IsmallLeftHSV, CV_BGR2HSV);
				cvCvtColor(IsmallRight, IsmallRightHSV, CV_BGR2HSV);
				//getColorRangeN(captureLeft, ImaskSmallLeft, 10, lo, hi);
				//getColorRangeN(captureRight, ImaskSmallRight, 10, lo1, hi1);
				//getColorRange(IsmallLeftHSV, ImaskSmallLeft, &ma[0], &lo[0], &hi[0]);
				ma[0]=125;lo[0]=120;hi[0]=130;
				//getColorRange(IsmallRightHSV, ImaskSmallRight, &ma[1], &lo[1], &hi[1]);
				ma[1]=ma[0];lo[1]=lo[0];hi[1]=hi[0];
			}
			break;
		case 'l'://draw lines
			displayMode = DISPLAY_MODE_DRAW_LINE;
			setMouseCallback("CameraLeft", onDrawLine, IsmallLeft);
			setMouseCallback("CameraRight", onDrawLineRight, IsmallRight);
			cvWaitKey(0);
			displayMode = DISPLAY_MODE_DRAW;
			setMouseCallback("CameraLeft", onMouse, IsmallLeft);
			setMouseCallback("CameraRight", onMouseRight, IsmallRight);
			break;
		case 'c'://go to next frame and pause
			if (displayMode == DISPLAY_MODE_PAUSE){
				cvCvtColor(IsmallLeft, IsmallLeftHSV, CV_BGR2HSV);
				//getColorRange(IsmallLeftHSV, ImaskSmallLeft, &ma, &lo, &hi);
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
