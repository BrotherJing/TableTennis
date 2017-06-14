/*
=========================================
remap 3d coordinates to 2d image plane to check correctness.

required matrix file:
- Intrinsics.xml
- Distortion.xml
- Rotation.xml
- Translation.xml
- sequence3D.xml

required input:
- sequence info(left, right)
- video(left right)

usage:
./remaptest seq3D.xml infoLeft.xml infoRight.xml left.mp4 right.mp4
=========================================
*/

#include "Main.h"

using namespace cv;
using namespace std;

CvCapture *capture, *captureRight;
IplImage *frame, *Ismall, *frameRight, *IsmallRight;
IplImage *Imask, *ImaskSmall;
CvSize sz, szSmall;

int currentState = STATE_PLAY;
int frameCount = 0;
int startFrameLeft = 0, startFrameRight = 0;

int ptrLeft=0, ptrRight=0;

CvMat *seqLeft, *seqRight;
CvMat *seq3D;
CvMat *infoLeft, *infoRight;
CvMat *intrinsicMatrix;
CvMat *distortionCoeffs;
CvMat *rotationVectors, *rotationMatrixLeft, *rotationMatrixRight;
CvMat *translationVectors, *translationLeft, *translationRight;
CvMat *matZScale;

void allocImages(IplImage *frame){
	sz = cvGetSize(frame);
	szSmall.width = sz.width / SCALE; szSmall.height = sz.height / SCALE;

	Ismall = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallRight = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	Imask = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmall = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
}

void releaseImages(){
}

void loadMatrices(char **argv){
	seq3D = (CvMat*)cvLoad(argv[1]);
	intrinsicMatrix = (CvMat*)cvLoad("Intrinsics.xml");
	distortionCoeffs = (CvMat*)cvLoad("Distortion.xml");
	rotationVectors = (CvMat*)cvLoad("Rotation.xml");
	translationVectors = (CvMat*)cvLoad("Translation.xml");
	matZScale = (CvMat*)cvLoad("ZScale.xml");
	if(!matZScale){
		matZScale = cvCreateMat(1,1,CV_32FC1);
		*((float*)CV_MAT_ELEM_PTR(*matZScale, 0, 0)) = -1;
	}
	infoLeft = (CvMat*)cvLoad(argv[2]);
	infoRight = (CvMat*)cvLoad(argv[3]);

	translationLeft = cvCreateMat(3, 1, CV_32FC1);
	translationRight = cvCreateMat(3, 1, CV_32FC1);

	rotationMatrixLeft = cvCreateMat(3, 1, CV_32FC1);
	rotationMatrixRight = cvCreateMat(3, 1, CV_32FC1);

	for(int i=0;i<3;++i){
		*((float*)CV_MAT_ELEM_PTR(*rotationMatrixLeft, i, 0)) = (float)CV_MAT_ELEM(*rotationVectors, float, 0, i);
		*((float*)CV_MAT_ELEM_PTR(*rotationMatrixRight, i, 0)) = (float)CV_MAT_ELEM(*rotationVectors, float, 1, i);
		*((float*)CV_MAT_ELEM_PTR(*translationLeft, i, 0)) = (float)CV_MAT_ELEM(*translationVectors, float, 0, i);
		*((float*)CV_MAT_ELEM_PTR(*translationRight, i, 0)) = (float)CV_MAT_ELEM(*translationVectors, float, 1, i);
	}

	seqLeft = cvCreateMat(seq3D->rows, 2, CV_32FC1);
	seqRight = cvCreateMat(seq3D->rows, 2, CV_32FC1);

	for(int i=0;i<seq3D->rows;++i){
		*((float*)CV_MAT_ELEM_PTR(*seq3D, i, 2)) /= (float)CV_MAT_ELEM(*matZScale, float, 0, 0);
	}

	cvProjectPoints2(seq3D, rotationMatrixLeft, translationLeft, intrinsicMatrix, distortionCoeffs, seqLeft);
	cvProjectPoints2(seq3D, rotationMatrixRight, translationRight, intrinsicMatrix, distortionCoeffs, seqRight);
}

void alignSequences(){
	startFrameLeft = (int)(CV_MAT_ELEM(*infoLeft, int, 0, 0));
	int originalFrameLeft = (int)(CV_MAT_ELEM(*infoLeft, int, 1, 0));
	
	startFrameRight = (int)(CV_MAT_ELEM(*infoRight, int, 0, 0));
	int originalFrameRight = (int)(CV_MAT_ELEM(*infoRight, int, 1, 0));
	
	if(originalFrameLeft - startFrameLeft > originalFrameRight - startFrameRight){
		startFrameLeft = originalFrameLeft - (originalFrameRight - startFrameRight);
	}else{
		startFrameRight = originalFrameRight - (originalFrameLeft - startFrameLeft);
	}
}

int main(int argc, char **argv){

	loadMatrices(argv);
	alignSequences();

	namedWindow("display", WINDOW_AUTOSIZE);
	namedWindow("displayRight", WINDOW_AUTOSIZE);
	capture = cvCreateFileCapture(argv[4]);
	captureRight = cvCreateFileCapture(argv[5]);
	frame = cvQueryFrame(capture);
	frameRight = cvQueryFrame(captureRight);
	if(!frame||!frameRight)return 1;
	allocImages(frame);

	cvResize(frame, Ismall);
	cvResize(frameRight, IsmallRight);
	cvShowImage("display", Ismall);
	cvShowImage("displayRight", IsmallRight);

	for(int i=0;i<startFrameLeft-1;++i){
		cvQueryFrame(capture);
	}
	for(int i=0;i<startFrameRight-1;++i){
		cvQueryFrame(captureRight);
	}

	for(int i=0;i<seq3D->rows;++i){
		frame = cvQueryFrame(capture);
		frameRight = cvQueryFrame(captureRight);
		if(!frame||!frameRight)return 1;

		CvPoint left, right;
		left.x = (int)(*((float*)CV_MAT_ELEM_PTR(*seqLeft, i, 0)));
		left.y = (int)(*((float*)CV_MAT_ELEM_PTR(*seqLeft, i, 1)));
		right.x = (int)(*((float*)CV_MAT_ELEM_PTR(*seqRight, i, 0)));
		right.y = (int)(*((float*)CV_MAT_ELEM_PTR(*seqRight, i, 1)));
		cvCircle(frame, left, 4, CVX_RED, 2);
		cvCircle(frameRight, right, 4, CVX_RED, 2);

		cvResize(frame, Ismall);
		cvResize(frameRight, IsmallRight);
		cvShowImage("display", Ismall);
		cvShowImage("displayRight", IsmallRight);

		char c = cvWaitKey(100);
		if (c == KEY_ESC)break;
		if (c == KEY_SPACE)cvWaitKey(0);
	}
	return 0;
}
