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

const int SCALE = 2;
const int CHANNELS = 3;

const int TRAIN_BG_MODEL_ITER = 50;
const int CLEAR_STALE_PER_ITER = 50;

typedef struct ce{
	uchar learnHigh[CHANNELS];
	uchar learnLow[CHANNELS];
	uchar max[CHANNELS];
	uchar min[CHANNELS];
	int t_last_update;
	int stale;
}code_element;

typedef struct code_book{
	code_element **cb;
	int numEntries;
	int t;
}codeBook;

void codebook_tick(codeBook &c);
int update_codebook(uchar *p, codeBook &c, unsigned *cbBounds, int numChannels);
int clear_stale_entries(codeBook &c);
uchar background_diff(uchar *p, codeBook &c, int numChannels, int *minMod, int *maxMod);

void codebook_tick_img(IplImage *frame, codeBook **codebooks);
void update_codebook_img(IplImage *frame, codeBook **codebooks, unsigned *cbBounds);
void clear_stale_entries_img(IplImage *frame, codeBook **codebooks);
void background_diff_img(IplImage *frame, IplImage *mask, codeBook **codebooks, int *minMod, int *maxMod);

void find_connected_component(IplImage *mask, int *num=NULL, CvRect *bbox=NULL);
void draw_connected_components(IplImage *frame, int n, CvRect *bbox);

//for tracking
#define DIS(a,b) sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y))
const float LOWPASS_FILTER_RATE = 0.6;

const int DEFAULT_WIDTH = 32/SCALE;
const int DEFAULT_HEIGHT = 32/SCALE;

class Tracker{
public:
	CvRect context;
	CvRect bbox;
	CvPoint center, last;

	Tracker(CvRect);
	void set(CvRect);
};

bool trackBall(Tracker *tracker, CvRect *bbs, int cnt);

#endif
