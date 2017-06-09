/*
=========================================
required matrix file:
- Intrinsics.xml
- Distortion.xml
- Rotation.xml
- Translation.xml

required input:
- video(left right)

output:
- ZScale.xml

usage:
./GetZScale left.mp4 right.mp4 [transform.xml]
=========================================
*/

#include "Main.h"

const int NEW_SCALE = 1;
const float NET_HEIGHT = 152.5;

using namespace cv;
using namespace std;

//two cameras
CvCapture *captureLeft, *captureRight;

IplImage *frameLeft, *IsmallLeft, *frameRight, *IsmallRight;
CvSize sz, szSmall;

CvMat* seqLeft;
CvMat* seqRight;
CvMat *intrinsicMatrix;
CvMat *distortionCoeffs;
CvMat *rotationVectors, *rotationMatrixLeft, *rotationMatrixRight;
CvMat *translationVectors, *translationLeft, *translationRight;
CvMat *transformMatrix;

//record points drawn by user
vector<CvPoint> points;
CvPoint2D32f pts[4];

void AllocImages(IplImage *frame){

	sz = cvGetSize(frame);
	szSmall.width = sz.width / NEW_SCALE; szSmall.height = sz.height / NEW_SCALE;

	IsmallLeft = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallRight = cvCreateImage(szSmall, frame->depth, frame->nChannels);
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
		cvShowImage("display", frame);
		break;
	case CV_EVENT_RBUTTONDOWN:
		points.clear();
		break;
	}
}

void loadMatrices(int argc, char **argv){
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

	if(argc>3){
		transformMatrix = (CvMat*)cvLoad(argv[3]);
	}
}

CvPoint3D32f uv2xyz(CvPoint2D32f uvLeft,CvPoint2D32f uvRight)  
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

CvPoint2D32f transform(CvPoint uv, CvMat *transformMatrix){
	CvPoint2D32f uv1;
	CvMat *B = cvCreateMat(3,1,CV_32FC1);
	*((float*)CV_MAT_ELEM_PTR(*B, 0, 0)) = uv.x;
	*((float*)CV_MAT_ELEM_PTR(*B, 1, 0)) = uv.y;
	*((float*)CV_MAT_ELEM_PTR(*B, 2, 0)) = 1;
	CvMat *XY = cvCreateMat(3,1,CV_32FC1);
	cvMatMul(transformMatrix, B, XY);
	uv1.x = (float)CV_MAT_ELEM(*XY, float, 0,0);
	uv1.y = (float)CV_MAT_ELEM(*XY, float, 1,0);
	return uv1;
}

int main(int argc, char **argv){

	loadMatrices(argc, argv);

	namedWindow("display", WINDOW_AUTOSIZE);

	captureLeft = cvCreateFileCapture(argv[1]);
	captureRight = cvCreateFileCapture(argv[2]);

	frameLeft = cvQueryFrame(captureLeft);
	frameRight = cvQueryFrame(captureRight);

	AllocImages(frameLeft);

	while(true){
		cvResize(frameLeft, IsmallLeft);
		setMouseCallback("display", onMouse, IsmallLeft);
		cvShowImage("display", IsmallLeft);
		char c = cvWaitKey(0);
		if(c==KEY_ESC)continue;
		else if(points.size()!=2){
			points.clear();
		}else break;
	}
	cout<<points[0].x<<","<<points[0].y<<" "<<points[1].x<<","<<points[1].y<<endl;
	if(argc>3){
		pts[0] = transform(points[0], transformMatrix);
		pts[1] = transform(points[1], transformMatrix);
	}else{
		pts[0] = CvPoint2D32f{points[0].x, points[0].y};
		pts[1] = CvPoint2D32f{points[1].x, points[1].y};
	}
	points.clear();
	while(true){
		cvResize(frameRight, IsmallRight);
		setMouseCallback("display", onMouse, IsmallRight);
		cvShowImage("display", IsmallRight);
		char c = cvWaitKey(0);
		if(c==KEY_ESC)continue;
		else if(points.size()!=2){
			points.clear();
		}else break;
	}
	cout<<points[0].x<<","<<points[0].y<<" "<<points[1].x<<","<<points[1].y<<endl;
	pts[2] = CvPoint2D32f{points[0].x, points[0].y};
	pts[3] = CvPoint2D32f{points[1].x, points[1].y};

	CvPoint3D32f xyz1 = uv2xyz(pts[0], pts[2]);
	CvPoint3D32f xyz2 = uv2xyz(pts[1], pts[3]);

	cout<<xyz1.x<<","<<xyz1.y<<","<<xyz1.z<<endl;
	cout<<xyz2.x<<","<<xyz2.y<<","<<xyz2.z<<endl;

	float zScale = NET_HEIGHT / ((xyz1.z+xyz2.z)/2);

	CvMat *matZScale = cvCreateMat(1, 1, CV_32FC1);
	*((float*)CV_MAT_ELEM_PTR(*matZScale, 0, 0)) = zScale;

	cvSave("ZScale.xml", matZScale);

	DeallocateImages();
	cvReleaseCapture(&captureLeft); cvReleaseCapture(&captureRight);
	cvDestroyWindow("display");
	return 0;
}
