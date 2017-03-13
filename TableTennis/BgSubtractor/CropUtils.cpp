#include "Main.h"
#include "CropUtils.h"

void cropImage(IplImage *frame, IplImage **dest, CvRect *bboxes, CvRect rect, int *numNeg, int numPos, int negScale, int maxScale, int outputSize){

	CvSize frameSize = cvGetSize(frame);
	
	for(int i=0; i<numPos;){
		int longEdge = rect.width>rect.height?rect.width:rect.height;
		int size = rand()%((maxScale-1)*longEdge) + longEdge + 1;
		double ratio = outputSize*1.0/size;
		int padding_left = rand()%(size-longEdge);
		int padding_top = rand()%(size-longEdge);
		int x = rect.x - padding_left;
		int y = rect.y - padding_top;
		if(x<0||y<0||x+size>frameSize.width||y+size>frameSize.height)continue;
		cvSetImageROI(frame, cvRect(x, y, size, size));
		cvResize(frame, dest[i]);
		CvRect bbox = cvRect(int(padding_left*ratio), int(padding_top*ratio), int(rect.width*ratio), int(rect.height*ratio));//bbox of the object in the patch
		bboxes[i] = bbox;
		cvResetImageROI(frame);
		++i;
	}

	*numNeg=0;
	for(int i=0;i<3;++i){
		for(int j=0;j<3;++j){
			if(i==1&&j==1)continue;
			int rel_x = int((-negScale+(1.0+negScale)/2*i)*rect.width);
			int rel_y = int((-negScale+(1.0+negScale)/2*j)*rect.height);
			int x = rect.x + rel_x;
			int y = rect.y + rel_y;
			if(x<0||y<0||x+negScale*rect.width>frameSize.width||y+negScale*rect.height>frameSize.height){
				continue;
			}
			cvSetImageROI(frame, cvRect(x, y, negScale*rect.width, negScale*rect.height));
			cvResize(frame, dest[numPos+*numNeg]);
			cvResetImageROI(frame);
			*numNeg = *numNeg+1;
		}
	}

}

void stitchImages(IplImage **crops, IplImage *display, CvRect *bboxes, int numNeg, int numPos){
	int numCrops = numNeg + numPos;
	int grid = sqrt(NUM_POS_PER_FRAME+NUM_NEG_PER_FRAME)+1;
	int k = 0;
	for(int i=0;i<grid;++i){
		for(int j=0;j<grid;++j){
			if(k==numCrops)return;
			cvSetImageROI(display, cvRect(i*PATCH_WIDTH, j*PATCH_HEIGHT, PATCH_WIDTH, PATCH_HEIGHT));
			cvCopy(crops[k], display);
			if(k<numPos)
				cvRectangle(display, cvPoint(bboxes[k].x, bboxes[k].y), cvPoint(bboxes[k].x+bboxes[k].width, bboxes[k].y+bboxes[k].height), CVX_RED, 1, CV_AA);
			cvResetImageROI(display);
			++k;
		}
	}
}