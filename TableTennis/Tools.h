#ifndef HEADER_TOOLS
#define HEADER_TOOLS

void showHist(IplImage *frame, CvArr *mask = NULL, bool wait = true, bool keep = false);
void getColorRange(IplImage *frame, CvArr *mask, int *maxIdx, int *loIdx, int *hiIdx); 
void getColorRangeN(CvCapture *capture, CvArr *mask, int n, int lo[3], int hi[3]);
void drawMask(std::vector<CvPoint> &points, IplImage *mask);

#endif