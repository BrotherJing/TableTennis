#ifndef HEADER_CROP_UTILS
#define HEADER_CROP_UTILS

#include <stdlib.h>//srand, rand
#include <time.h>//time
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <limits.h>

//output probobility map or not. In general, the image patches and probability maps should be generated together.
const bool OUTPUT_PROB_MAP = true;
//output positive and negative examples from ping pong ball and its surrounding, or negative example from background.
const bool NO_BG = true;

//size of an output image patch
const int PATCH_WIDTH = 100;
const int PATCH_HEIGHT = 100;
//size of an output probability map
const int PROB_MAP_WIDHT = 50;
const int PROB_MAP_HEIGHT = 50;

const int PROB_MAP_PADDING = 0;

//number of patches from one frame
const int NUM_POS_PER_FRAME = 16;
const int NUM_NEG_PER_FRAME = 8;//should be at least 8!

//maximum scale of the patch compared to the original bbox
const int MAX_SCALE = 4;
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

void saveImages(IplImage **crops, 
	CvRect *bboxes, 
	ofstream &filenames, 
	ofstream &groundTruth,
	ofstream &fileBBox,//bbox in the 100*100 patch
	string prefix, 
	char *frameCountStr, 
	int numNeg, 
	int numPos = NUM_POS_PER_FRAME);

void cropNegFromBg(IplImage *frame, IplImage **dest, CvRect *bboxes, int *numNeg, int non_bg_idx);

void saveNegFromBg(IplImage **crops, 
	ofstream &filenames, 
	ofstream &groundTruth, 
	ofstream &fileBBox, 
	string prefix, 
	char *frameCountStr, 
	int numCrops);

#endif
