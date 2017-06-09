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
		CV_CALIB_FIX_K1|CV_CALIB_FIX_K2|CV_CALIB_FIX_K3|CV_CALIB_USE_INTRINSIC_GUESS|CV_CALIB_FIX_PRINCIPAL_POINT|CV_CALIB_FIX_FOCAL_LENGTH);//
	cout << "ok" << endl;
}

/*
intrinsicMatrix: the matrix to transform to
intrinsicMatrix1: the intrinsic matrix for this camera
*/
void transformUV(CvPoint2D32f *ptsLeft, CvMat *intrinsicMatrix, CvMat *intrinsicMatrix1){
	CvMat *A = cvCreateMat(2,2,CV_32FC1);
	*((float*)CV_MAT_ELEM_PTR(*A, 0, 0)) = (float)CV_MAT_ELEM(*intrinsicMatrix1, float, 0,0);
	*((float*)CV_MAT_ELEM_PTR(*A, 0, 1)) = (float)CV_MAT_ELEM(*intrinsicMatrix1, float, 0,1);
	*((float*)CV_MAT_ELEM_PTR(*A, 1, 0)) = (float)CV_MAT_ELEM(*intrinsicMatrix1, float, 1,0);
	*((float*)CV_MAT_ELEM_PTR(*A, 1, 1)) = (float)CV_MAT_ELEM(*intrinsicMatrix1, float, 1,1);

	for(int i=0;i<4;++i){
		CvMat *B = cvCreateMat(2,1,CV_32FC1);
		*((float*)CV_MAT_ELEM_PTR(*B, 0, 0)) = ptsLeft[i].x - (float)CV_MAT_ELEM(*intrinsicMatrix1, float, 0,2);
		*((float*)CV_MAT_ELEM_PTR(*B, 1, 0)) = ptsLeft[i].y - (float)CV_MAT_ELEM(*intrinsicMatrix1, float, 1,2);
		CvMat *XY = cvCreateMat(2,1,CV_32FC1);
		cvSolve(A,B,XY,CV_SVD);

		float x_new = (float)CV_MAT_ELEM(*XY, float, 0,0)*(float)CV_MAT_ELEM(*intrinsicMatrix, float, 0,0)+
			(float)CV_MAT_ELEM(*XY, float, 1,0)*(float)CV_MAT_ELEM(*intrinsicMatrix, float, 0,1)+
			(float)CV_MAT_ELEM(*intrinsicMatrix, float, 0,2);
		float y_new = (float)CV_MAT_ELEM(*XY, float, 0,0)*(float)CV_MAT_ELEM(*intrinsicMatrix, float, 1,0)+
			(float)CV_MAT_ELEM(*XY, float, 1,0)*(float)CV_MAT_ELEM(*intrinsicMatrix, float, 1,1)+
			(float)CV_MAT_ELEM(*intrinsicMatrix, float, 1,2);
		ptsLeft[i].x = x_new;
		ptsLeft[i].y = y_new;
	}
}

void transformUV(CvPoint2D32f *ptsLeft, CvMat *transformMatrix){
	for(int i=0;i<4;++i){
		CvMat *B = cvCreateMat(3,1,CV_32FC1);
		*((float*)CV_MAT_ELEM_PTR(*B, 0, 0)) = ptsLeft[i].x;
		*((float*)CV_MAT_ELEM_PTR(*B, 1, 0)) = ptsLeft[i].y;
		*((float*)CV_MAT_ELEM_PTR(*B, 2, 0)) = 1;
		CvMat *XY = cvCreateMat(3,1,CV_32FC1);
		cvMatMul(transformMatrix, B, XY);
		ptsLeft[i].x = (float)CV_MAT_ELEM(*XY, float, 0,0);
		ptsLeft[i].y = (float)CV_MAT_ELEM(*XY, float, 1,0);
	}
}

void getTransformMatrix(CvMat *intrinsicMatrix, CvMat *intrinsicMatrix1, CvMat *res){
	cvInvert(intrinsicMatrix1, res);
	cvMatMul(intrinsicMatrix, res, res);
}