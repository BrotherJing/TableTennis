#ifndef HEADER_TRACKER
#define HEADER_TRACKER

#include <opencv2/opencv.hpp>

//for tracking
#define DIS(a,b) sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y))
const float LOWPASS_FILTER_RATE = 0.;

class Tracker{
public:
	CvRect context;
	CvRect bbox;
	CvPoint center, last;
	int bbox_idx;//index of the bbox in the proposals.

	Tracker(CvRect);
	void set(CvRect);
};

bool trackBall(Tracker *tracker, CvRect *bbs, int cnt);

#endif