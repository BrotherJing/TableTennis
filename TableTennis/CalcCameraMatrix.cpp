#include "Camera.h"
#include "Tools.h"

using namespace std;

void calibrateCamera(CvPoint2D32f *ptsLeft, CvPoint2D32f *ptsRight, CvSize imageSize, CvMat *intrinsicMatrix, CvMat *distortionMatrix, CvMat *rotationVectors, CvMat *translationVectors){
	CvMat *objectPoints = cvCreateMat(8, 3, CV_32FC1);
	CvMat *imagePoints = cvCreateMat(8, 2, CV_32FC1);
	CvMat *pointCounts = cvCreateMat(2, 1, CV_32SC1);
	*((float*)CV_MAT_ELEM_PTR(*distortionMatrix, 0, 0)) = 0.0f;
	*((float*)CV_MAT_ELEM_PTR(*distortionMatrix, 1, 0)) = 0.0f;
	*((float*)CV_MAT_ELEM_PTR(*distortionMatrix, 4, 0)) = 0.0f;//k1 k2 k3 = 0
	for (int i = 0; i < 4; ++i){
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i, 0)) = ptsLeft[i].x * SCALE;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i, 1)) = ptsLeft[i].y * SCALE;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i+4, 0)) = ptsRight[i].x * SCALE;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i + 4, 1)) = ptsRight[i].y * SCALE;//resize the points to original image size!!
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
	cvCalibrateCamera2(objectPoints, imagePoints, pointCounts, imageSize, intrinsicMatrix, distortionMatrix, rotationVectors, translationVectors, CV_CALIB_FIX_K1|CV_CALIB_FIX_K2|CV_CALIB_FIX_K3);
	cout << "ok" << endl;
}


void calibrateCameraUseGuess(CvPoint2D32f *ptsLeft, CvPoint2D32f *ptsRight, CvSize imageSize, CvMat *intrinsicMatrix, CvMat *distortionMatrix, CvMat *rotationVectors, CvMat *translationVectors){
	CvMat *objectPoints = cvCreateMat(8, 3, CV_32FC1);
	CvMat *imagePoints = cvCreateMat(8, 2, CV_32FC1);
	CvMat *pointCounts = cvCreateMat(2, 1, CV_32SC1);
	for (int i = 0; i < 4; ++i){
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i, 0)) = ptsLeft[i].x * SCALE;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i, 1)) = ptsLeft[i].y * SCALE;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i+4, 0)) = ptsRight[i].x * SCALE;
		*((float*)CV_MAT_ELEM_PTR(*imagePoints, i + 4, 1)) = ptsRight[i].y * SCALE;//resize the points to original image size!!
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
	cvCalibrateCamera2(objectPoints, imagePoints, pointCounts, imageSize, intrinsicMatrix, distortionMatrix, rotationVectors, translationVectors, 
		CV_CALIB_FIX_K1|CV_CALIB_FIX_K2|CV_CALIB_FIX_K3|CV_CALIB_USE_INTRINSIC_GUESS|CV_CALIB_FIX_PRINCIPAL_POINT|CV_CALIB_FIX_FOCAL_LENGTH);
	cout << "ok" << endl;
}
