#include "Reconstruct.h"

using namespace std;
using namespace cv;

Reconstruct::Reconstruct(string camera_matrix_dir){
	intrinsicMatrix = (CvMat*)cvLoad((camera_matrix_dir + "/Intrinsics.xml").c_str());
	distortionCoeffs = (CvMat*)cvLoad((camera_matrix_dir + "/Distortion.xml").c_str());
	rotationVectors = (CvMat*)cvLoad((camera_matrix_dir + "/Rotation.xml").c_str());
	translationVectors = (CvMat*)cvLoad((camera_matrix_dir + "/Translation.xml").c_str());
	matZScale = (CvMat*)cvLoad((camera_matrix_dir + "/ZScale.xml").c_str());
	if(!matZScale){
		matZScale = cvCreateMat(1,1,CV_32FC1);
		*((float*)CV_MAT_ELEM_PTR(*matZScale, 0, 0)) = 1;
	}

	rotationMatrixLeft = cvCreateMat(3, 3, CV_32FC1);
	rotationMatrixRight = cvCreateMat(3, 3, CV_32FC1);
	translationLeft = cvCreateMat(3, 1, CV_32FC1);
	translationRight = cvCreateMat(3, 1, CV_32FC1);

	CvMat *rotationLeftTemp = cvCreateMat(3, 1, CV_32FC1);
	CvMat *rotationRightTemp = cvCreateMat(3, 1, CV_32FC1);

	for(int i=0;i<3;++i){
		*((float*)CV_MAT_ELEM_PTR(*rotationLeftTemp, i, 0)) = (float)CV_MAT_ELEM(*rotationVectors, float, 0, i);
		*((float*)CV_MAT_ELEM_PTR(*rotationRightTemp, i, 0)) = (float)CV_MAT_ELEM(*rotationVectors, float, 1, i);
		*((float*)CV_MAT_ELEM_PTR(*translationLeft, i, 0)) = (float)CV_MAT_ELEM(*translationVectors, float, 0, i);
		*((float*)CV_MAT_ELEM_PTR(*translationRight, i, 0)) = (float)CV_MAT_ELEM(*translationVectors, float, 1, i);
	}

	cvRodrigues2(rotationLeftTemp, rotationMatrixLeft);
	cvRodrigues2(rotationRightTemp, rotationMatrixRight);
}

Reconstruct::~Reconstruct(){
}


CvPoint3D32f Reconstruct::uv2xyz(CvPoint uvLeft,CvPoint uvRight)
{  
    //  [u1]      |X|                     [u2]      |X|  
    //Z*|v1| = Ml*|Y|                   Z*|v2| = Mr*|Y|  
    //  [ 1]      |Z|                     [ 1]      |Z|  
    //            |1|                               |1|  
    CvMat *mLeftRT = cvCreateMat(3,4,CV_32FC1);
    CvMat *mRightRT = cvCreateMat(3,4,CV_32FC1);
    for(int i=0;i<3;++i){
    	*((float*)CV_MAT_ELEM_PTR(*mLeftRT, i, 0)) = (float)CV_MAT_ELEM(*rotationMatrixLeft, float, i, 0);
    	*((float*)CV_MAT_ELEM_PTR(*mLeftRT, i, 1)) = (float)CV_MAT_ELEM(*rotationMatrixLeft, float, i, 1);
    	*((float*)CV_MAT_ELEM_PTR(*mLeftRT, i, 2)) = (float)CV_MAT_ELEM(*rotationMatrixLeft, float, i, 2);
    	*((float*)CV_MAT_ELEM_PTR(*mLeftRT, i, 3)) = (float)CV_MAT_ELEM(*translationLeft, float, i, 0);

    	*((float*)CV_MAT_ELEM_PTR(*mRightRT, i, 0)) = (float)CV_MAT_ELEM(*rotationMatrixRight, float, i, 0);
    	*((float*)CV_MAT_ELEM_PTR(*mRightRT, i, 1)) = (float)CV_MAT_ELEM(*rotationMatrixRight, float, i, 1);
    	*((float*)CV_MAT_ELEM_PTR(*mRightRT, i, 2)) = (float)CV_MAT_ELEM(*rotationMatrixRight, float, i, 2);
    	*((float*)CV_MAT_ELEM_PTR(*mRightRT, i, 3)) = (float)CV_MAT_ELEM(*translationRight, float, i, 0);
    }
    CvMat *mLeftIntrinsic = intrinsicMatrix;
    CvMat *mLeftM = cvCreateMat(3, 4, CV_32FC1);
    cvMatMul(mLeftIntrinsic, mLeftRT, mLeftM);
  
    CvMat *mRightIntrinsic = intrinsicMatrix;
    CvMat *mRightM = cvCreateMat(3, 4, CV_32FC1);
    cvMatMul(mRightIntrinsic, mRightRT, mRightM);
  
    //最小二乘法A矩阵  
    CvMat *A = cvCreateMat(4,3,CV_32FC1);
    *((float*)CV_MAT_ELEM_PTR(*A, 0, 0)) = uvLeft.x * (float)CV_MAT_ELEM(*mLeftM, float, 2,0) - (float)CV_MAT_ELEM(*mLeftM, float, 0,0);
    *((float*)CV_MAT_ELEM_PTR(*A, 0, 1)) = uvLeft.x * (float)CV_MAT_ELEM(*mLeftM, float, 2,1) - (float)CV_MAT_ELEM(*mLeftM, float, 0,1);
    *((float*)CV_MAT_ELEM_PTR(*A, 0, 2)) = uvLeft.x * (float)CV_MAT_ELEM(*mLeftM, float, 2,2) - (float)CV_MAT_ELEM(*mLeftM, float, 0,2);


    *((float*)CV_MAT_ELEM_PTR(*A, 1, 0)) = uvLeft.y * (float)CV_MAT_ELEM(*mLeftM, float, 2,0) - (float)CV_MAT_ELEM(*mLeftM, float, 1,0);
    *((float*)CV_MAT_ELEM_PTR(*A, 1, 1)) = uvLeft.y * (float)CV_MAT_ELEM(*mLeftM, float, 2,1) - (float)CV_MAT_ELEM(*mLeftM, float, 1,1);
    *((float*)CV_MAT_ELEM_PTR(*A, 1, 2)) = uvLeft.y * (float)CV_MAT_ELEM(*mLeftM, float, 2,2) - (float)CV_MAT_ELEM(*mLeftM, float, 1,2);
  	
  	*((float*)CV_MAT_ELEM_PTR(*A, 2, 0)) = uvRight.x * (float)CV_MAT_ELEM(*mRightM, float, 2,0) - (float)CV_MAT_ELEM(*mRightM, float, 0,0);
  	*((float*)CV_MAT_ELEM_PTR(*A, 2, 1)) = uvRight.x * (float)CV_MAT_ELEM(*mRightM, float, 2,1) - (float)CV_MAT_ELEM(*mRightM, float, 0,1);
  	*((float*)CV_MAT_ELEM_PTR(*A, 2, 2)) = uvRight.x * (float)CV_MAT_ELEM(*mRightM, float, 2,2) - (float)CV_MAT_ELEM(*mRightM, float, 0,2);
  
  	*((float*)CV_MAT_ELEM_PTR(*A, 3, 0)) = uvRight.y * (float)CV_MAT_ELEM(*mRightM, float, 2,0) - (float)CV_MAT_ELEM(*mRightM, float, 1,0);
  	*((float*)CV_MAT_ELEM_PTR(*A, 3, 1)) = uvRight.y * (float)CV_MAT_ELEM(*mRightM, float, 2,1) - (float)CV_MAT_ELEM(*mRightM, float, 1,1);
  	*((float*)CV_MAT_ELEM_PTR(*A, 3, 2)) = uvRight.y * (float)CV_MAT_ELEM(*mRightM, float, 2,2) - (float)CV_MAT_ELEM(*mRightM, float, 1,2);
  
    //最小二乘法B矩阵  
    CvMat *B = cvCreateMat(4,1,CV_32FC1);
    *((float*)CV_MAT_ELEM_PTR(*B, 0, 0)) = (float)CV_MAT_ELEM(*mLeftM, float, 0,3) - uvLeft.x * (float)CV_MAT_ELEM(*mLeftM, float, 2,3);
    *((float*)CV_MAT_ELEM_PTR(*B, 1, 0)) = (float)CV_MAT_ELEM(*mLeftM, float, 1,3) - uvLeft.y * (float)CV_MAT_ELEM(*mLeftM, float, 2,3);
    *((float*)CV_MAT_ELEM_PTR(*B, 2, 0)) = (float)CV_MAT_ELEM(*mRightM, float, 0,3) - uvRight.x * (float)CV_MAT_ELEM(*mRightM, float, 2,3);
    *((float*)CV_MAT_ELEM_PTR(*B, 3, 0)) = (float)CV_MAT_ELEM(*mRightM, float, 1,3) - uvRight.y * (float)CV_MAT_ELEM(*mRightM, float, 2,3);
  
    CvMat *XYZ = cvCreateMat(3,1,CV_32FC1);  
    cvSolve(A,B,XYZ,CV_SVD);
  
    //世界坐标系中坐标  
    CvPoint3D32f world;  
    world.x = (float)CV_MAT_ELEM(*XYZ, float, 0,0);
    world.y = (float)CV_MAT_ELEM(*XYZ, float, 1,0);
    world.z = (float)CV_MAT_ELEM(*XYZ, float, 2,0)*(float)CV_MAT_ELEM(*matZScale, float, 0, 0);
  
    return world;  
}  
