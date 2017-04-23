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

class BgSubtractor{
public:
	int train_bg_model_iter;
	//for finding connected components
	int numComponents;
	CvRect *bboxes;
	std::vector<cv::Rect> bboxes_vec;
	static unsigned BOUNDS_DEFAULT[3];
	static int MIN_MOD_DEFAULT[3];
	static int MAX_MOD_DEFAULT[3];
	//for bg subtraction
	codeBook **codebooks;

	BgSubtractor(CvSize size, int train_iter, int numComponents);
	bool process(IplImage *frame, IplImage *draw=NULL);
	bool process(cv::Mat &frame, cv::Mat &draw);
	bool process(cv::Mat &frame);
private:
	int frameCount;
	CvSize size;
	IplImage *mask;
};

void codebook_tick(codeBook &c);
int update_codebook(uchar *p, codeBook &c, unsigned *cbBounds, int numChannels);
int clear_stale_entries(codeBook &c);
uchar background_diff(uchar *p, codeBook &c, int numChannels, int *minMod, int *maxMod);

void codebook_tick_img(IplImage *frame, codeBook **codebooks);
void update_codebook_img(IplImage *frame, codeBook **codebooks, unsigned *cbBounds);
void clear_stale_entries_img(IplImage *frame, codeBook **codebooks);
void background_diff_img(IplImage *frame, IplImage *mask, codeBook **codebooks, int *minMod, int *maxMod);

void find_connected_component(IplImage *mask, int *num=NULL, CvRect *bbox=NULL);
void find_connected_component2(IplImage *mask, int *num, std::vector<cv::Rect> &bbox);
void draw_connected_components(IplImage *frame, int n, CvRect *bbox);
void draw_connected_components2(IplImage *frame, std::vector<cv::Rect> &bbox);

#endif