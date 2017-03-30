#ifndef HEADER_CODEBOOK
#define HEADER_CODEBOOK

#include <opencv2/opencv.hpp>

typedef struct ce{
#ifdef CHANNELS
	uchar learnHigh[CHANNELS];
	uchar learnLow[CHANNELS];
	uchar max[CHANNELS];
	uchar min[CHANNELS];
#else
	uchar learnHigh[3];
	uchar learnLow[3];
	uchar max[3];
	uchar min[3];
#endif
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

#endif