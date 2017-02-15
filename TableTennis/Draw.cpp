#include "Camera.h"
#include "Tools.h"

using namespace std;

void drawMask(vector<CvPoint> &points, IplImage *mask){
	CvPoint *pts[] = { &points[0] };
	int npts[] = { points.size() };
	cvFillPoly(mask, pts, npts, 1, cvScalar(255, 255, 255), CV_AA);
}