#include "Main.h"

void find_connected_component(IplImage *mask, int *num, CvRect *bbox){
	static CvMemStorage *mem_storage = NULL;
	static CvSeq *contours, *c;
	cvMorphologyEx(mask, mask, 0, 0, CV_MOP_OPEN, 1);
	cvMorphologyEx(mask, mask, 0, 0, CV_MOP_CLOSE, 1);
	if(mem_storage==NULL){
		mem_storage = cvCreateMemStorage(0);
	}else{
		cvClearMemStorage(mem_storage);
	}
	cvFindContours(mask, mem_storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	cvZero(mask);
	if(num!=NULL){
		int N=*num, numFilled=0, i;
		for(i=0, c=contours;c!=NULL;c=c->h_next,++i){
			if(i<N){
				cvDrawContours(mask, c, CVX_WHITE, CVX_WHITE, -1, CV_FILLED, 8);
				if(bbox!=NULL){
					bbox[i] = cvBoundingRect(c);
				}
				numFilled++;
			}
		}
		*num = numFilled;
	}
}

void draw_connected_components(IplImage *frame, int n, CvRect *bbox){
	for(int i=0;i<n;++i){
		cvRectangle(frame, cvPoint(bbox[i].x, bbox[i].y), cvPoint(bbox[i].x+bbox[i].width, bbox[i].y+bbox[i].height), CVX_RED, 1);
	}
}