#ifndef HEADER_CROP_UTILS
#define HEADER_CROP_UTILS

#include <stdlib.h>//srand, rand
#include <time.h>//time

const int PATCH_WIDTH = 100;
const int PATCH_HEIGHT = 100;

const int NUM_POS_PER_FRAME = 4;
const int NUM_NEG_PER_FRAME = 8;

//maximum scale of the patch compared to the original bbox
const int MAX_SCALE = 3;
const int NEG_SCALE = 2;

void cropImage(IplImage *frame, 
	IplImage **dest, 
	CvRect *bboxes, 
	CvRect rect, 
	int *numNeg, 
	int numPos = NUM_POS_PER_FRAME, 
	int negScale = NEG_SCALE, 
	int maxScale = MAX_SCALE, 
	int outputSize = PATCH_WIDTH);
void stitchImages(IplImage **crops, IplImage *display, CvRect *bboxes, int numNeg, int numPos = NUM_POS_PER_FRAME);

#endif
