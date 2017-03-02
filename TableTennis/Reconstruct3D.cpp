#include "Camera.h"
#include "Tools.h"

using namespace std;

void calibrateCamera(CvPoint2D32f *ptsLeft, CvPoint2D32f *ptsRight, CvSize imageSize, CvMat *intrinsicMatrix, CvMat *distortionMatrix, CvMat *rotationVectors, CvMat *translationVectors){
	CvMat *objectPoints = cvCreateMat(8, 3, CV_32FC1);
	CvMat *imagePoints = cvCreateMat(8, 2, CV_32FC1);
	CvMat *pointCounts = cvCreateMat(2, 1, CV_32SC1);
	for (int i = 0; i < 4; ++i){
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i, 0)) = ptsLeft[i].x;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i, 1)) = ptsLeft[i].y;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i+4, 0)) = ptsRight[i].x;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i + 4, 1)) = ptsRight[i].y;
	}
	for (int i = 0; i < 2; ++i){
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4, 0)) = 0;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4, 1)) = 0;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4, 2)) = 0;

		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+1, 0)) = TABLE_WIDTH;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+1, 1)) = 0;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+1, 2)) = 0;

		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+2, 0)) = TABLE_WIDTH;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+2, 1)) = TABLE_HEIGHT;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+2, 2)) = 0;

		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i*4+3, 0)) = 0;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i * 4 + 3, 1)) = TABLE_HEIGHT;
		*((float*)CV_MAT_ELEM_PTR(*objectPoints, i * 4 + 3, 2)) = 0;
	}
	*((int*)CV_MAT_ELEM_PTR(*pointCounts, 0, 0)) = 4;
	*((int*)CV_MAT_ELEM_PTR(*pointCounts, 1, 0)) = 4;
	cvCalibrateCamera2(objectPoints, imagePoints, pointCounts, imageSize, intrinsicMatrix, distortionMatrix, rotationVectors, translationVectors);
	cout << "ok" << endl;
}
