/*
=========================================
required matrix file:
- Intrinsics.xml
- Distortion.xml
- Rotation.xml
- Translation.xml

required input:
- sequence 2D(left, right)

output:
- coords.csv
- left.csv
- right.csv
- sequence3D.xml

usage:
./reconstruct seqLeft.xml seqRight.xml
=========================================
*/

#include <string>
#include "Main.h"

using namespace cv;
using namespace std;

IplImage *frame, *Ismall;
IplImage *Imask, *ImaskSmall;
CvSize sz, szSmall;

int currentState = STATE_PLAY;
int frameCount = 0;

int ptrLeft=0, ptrRight=0;

vector<CvPoint3D32f> seq3D;
CvMat* seqLeft;
CvMat* seqRight;
CvMat *intrinsicMatrix;
CvMat *distortionCoeffs;
CvMat *rotationVectors, *rotationMatrixLeft, *rotationMatrixRight;
CvMat *translationVectors, *translationLeft, *translationRight;

void allocImages(IplImage *frame){
	sz = cvGetSize(frame);
	szSmall.width = sz.width / SCALE; szSmall.height = sz.height / SCALE;

	Ismall = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	Imask = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmall = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);
}

void releaseImages(){
}

void loadMatrices(char **argv){
	seqLeft = (CvMat*)cvLoad(argv[1]);
	seqRight = (CvMat*)cvLoad(argv[2]);
	intrinsicMatrix = (CvMat*)cvLoad("Intrinsics.xml");
	distortionCoeffs = (CvMat*)cvLoad("Distortion.xml");
	rotationVectors = (CvMat*)cvLoad("Rotation.xml");
	translationVectors = (CvMat*)cvLoad("Translation.xml");

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

void alignSequences(){
	int startLeft = CV_MAT_ELEM(*seqLeft, int, 0, 0);
	int startRight = CV_MAT_ELEM(*seqRight, int, 0, 0);

	if(startLeft < startRight){
		ptrLeft = startRight - startLeft;
	}else{
		ptrRight = startLeft - startRight;
	}
}

CvPoint3D32f uv2xyz(CvPoint uvLeft,CvPoint uvRight)  
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
    world.z = (float)CV_MAT_ELEM(*XYZ, float, 2,0);
  
    return world;  
}  

int main(int argc, char **argv){

	string filename = string(argv[1]);
	filename = filename.substr(0, filename.find('.')-1);

	ofstream oFile, oFileLeft,oFileRight;
	oFile.open((filename+".coords.csv").c_str(), ios::out | ios::trunc);
	oFileLeft.open((filename+"L.csv").c_str(), ios::out | ios::trunc);
	oFileRight.open((filename+"R.csv").c_str(), ios::out | ios::trunc);
	
	loadMatrices(argv);
	alignSequences();

	while(ptrLeft<seqLeft->rows &&
		ptrRight<seqRight->rows){
		CvPoint3D32f xyz = uv2xyz(cvPoint((int)CV_MAT_ELEM(*seqLeft, int, ptrLeft, 1), (int)CV_MAT_ELEM(*seqLeft, int, ptrLeft, 2)),
			cvPoint((int)CV_MAT_ELEM(*seqRight, int, ptrRight, 1), (int)CV_MAT_ELEM(*seqRight, int, ptrRight, 2)));
		seq3D.push_back(xyz);
		cout<<xyz.x<<","<<xyz.y<<","<<xyz.z<<endl;
		oFile<<xyz.x<<","<<xyz.y<<","<<xyz.z<<endl;
		oFileLeft<<CV_MAT_ELEM(*seqLeft, int, ptrLeft, 1)<<","<<CV_MAT_ELEM(*seqLeft, int, ptrLeft, 2)<<endl;
		oFileRight<<CV_MAT_ELEM(*seqRight, int, ptrRight, 1)<<","<<CV_MAT_ELEM(*seqRight, int, ptrRight, 2)<<endl;
		ptrLeft++;
		ptrRight++;
	}

	CvMat *reusult3D = cvCreateMat(seq3D.size(), 3, CV_32FC1);
	for(int i=0;i<seq3D.size();++i){
		CvPoint3D32f xyz = seq3D[i];
		*((float*)CV_MAT_ELEM_PTR(*reusult3D, i, 0)) = xyz.x;
		*((float*)CV_MAT_ELEM_PTR(*reusult3D, i, 1)) = xyz.y;
		*((float*)CV_MAT_ELEM_PTR(*reusult3D, i, 2)) = xyz.z;
	}
	cvSave((filename+".seq3D.xml").c_str(), reusult3D);

	oFile.close();
	oFileLeft.close();
	oFileRight.close();
	return 0;
}
