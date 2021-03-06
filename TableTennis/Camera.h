#ifndef HEADER_CONSTANTS
#define HEADER_CONSTANTS

#define cvQueryHistValue_1D( hist, idx0 ) \
    ((float)cvGetReal1D( (hist)->bins, (idx0)))

#define DEBUG_MODE

#include <cstdio>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <limits.h>

//scale down the video frame to display window. 2 means 0.5x of original size.
const int SCALE = 2;

const int DISPLAY_MODE_FIRST_FRAME = 0;
const int DISPLAY_MODE_PLAY = 1;
const int DISPLAY_MODE_DRAW = 2;
const int DISPLAY_MODE_PAUSE = 3;
const int DISPLAY_MODE_DRAW_LINE = 4;

const int KEY_ENTER = 10;
const int KEY_RETURN = 13;
const int KEY_ESC = 27;

const CvScalar CVX_WHITE = CV_RGB(0xff, 0xff, 0xff);
const CvScalar CVX_RED = CV_RGB(0xff, 0x00, 0x00);
const CvScalar CVX_BLACK = CV_RGB(0x00, 0x00, 0x00);

const int TABLE_AREA_THRESHOLD = 400;
const int MORPH_OPEN_ITER = 1;
const int MORPH_DILATE_ITER = 16;

const int TABLE_HEIGHT = 2740;
const int TABLE_WIDTH = 1525;
const int TABLE_IMG_SCALE = 5;

#endif