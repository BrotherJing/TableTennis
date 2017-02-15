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

const int SCALE = 4;

const int DISPLAY_MODE_FIRST_FRAME = 0;
const int DISPLAY_MODE_PLAY = 1;
const int DISPLAY_MODE_DRAW = 2;
const int DISPLAY_MODE_PAUSE = 3;

#endif