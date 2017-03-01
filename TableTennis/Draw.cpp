#include "Camera.h"
#include "Tools.h"

using namespace std;

void drawMask(vector<CvPoint> &points, IplImage *mask){
	CvPoint *pts[] = { &points[0] };
	int npts[] = { points.size() };
	cvFillPoly(mask, pts, npts, 1, cvScalar(255, 255, 255), CV_AA);
}

void drawConnectedComponents(CvSeq *contours, IplImage *mask){
	for (CvSeq *c = contours; c != NULL; c=c->h_next){
		cvDrawContours(mask, c, CVX_WHITE, CVX_WHITE, -1, CV_FILLED, 8);
	}
}