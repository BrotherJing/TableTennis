#ifndef HEADER_BG_SUBTRACTOR
#define HEADER_BG_SUBTRACTOR

#include <cstdio>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <limits.h>

using namespace std;

const CvScalar CVX_WHITE = CV_RGB(0xff, 0xff, 0xff);
const CvScalar CVX_RED = CV_RGB(0xff, 0x00, 0x00);
const CvScalar CVX_GREEN = CV_RGB(0x00, 0xff, 0x00);
const CvScalar CVX_BLACK = CV_RGB(0x00, 0x00, 0x00);

const int STATE_PLAY = 1;
const int STATE_PAUSE = 2;
const int STATE_WAIT = 3;

const int KEY_ENTER = 10;
const int KEY_RETURN = 13;
const int KEY_ESC = 27;
const int KEY_SPACE = 32;

//scale down the video frame to display window. 2 means 0.5x of original size.
const double SCALE = 2;
#define CHANNELS 3

//how many frames to use to model the background codebook
const int TRAIN_BG_MODEL_ITER = 50;
//how often to update the codebook
const int CLEAR_STALE_PER_ITER = 50;
//maximum connected components output by the background subtractor
const int MAX_COMPONENT = 20;

const int DEFAULT_WIDTH = 32/SCALE;
const int DEFAULT_HEIGHT = 32/SCALE;

#endif
