#include "Camera.h"
#include "Tools.h"

//for debug
void showHist(IplImage *frame, CvArr *mask, bool wait, bool keep){
	CvSize sz = cvGetSize(frame);
	IplImage *channels[3];
	IplImage *histImage[3];
	channels[0] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	channels[1] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	channels[2] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	cvSplit(frame, channels[0], channels[1], channels[2], 0);

	int hist_size = 256;
	int hist_height = 100;
	float range[] = { 0, 255 };
	float *ranges[] = { range };
	CvHistogram *hists[3];
	hists[0] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);
	hists[1] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);
	hists[2] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);
	cvCalcHist(&channels[0], hists[0], 0, mask);
	cvCalcHist(&channels[1], hists[1], 0, mask);
	cvCalcHist(&channels[2], hists[2], 0, mask);
	//cvNormalizeHist(hists[0], 1.0); cvNormalizeHist(hists[1], 1.0); cvNormalizeHist(hists[2], 1.0);

	histImage[0] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	histImage[1] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	histImage[2] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	cvZero(histImage[0]); cvZero(histImage[1]); cvZero(histImage[2]);

	float maxValue = 0;
	for (int i = 0; i < 3; ++i){
		cvGetMinMaxHistValue(hists[i], 0, &maxValue, 0, 0);
		for (int j = 0; j < 256; j++){
			float binValue = cvQueryHistValue_1D(hists[i], j);
			int intensity = cvRound(binValue * hist_height / maxValue);
			cvRectangle(histImage[i], cvPoint(j, hist_height - intensity), cvPoint(j + 1, hist_height), CV_RGB(255, 255, 255), CV_FILLED);
		}
	}

	cvShowImage("CHANNEL 1", histImage[0]);
	cvShowImage("CHANNEL 2", histImage[1]);
	cvShowImage("CHANNEL 3", histImage[2]);

	if (wait)cvWaitKey(0);

	if (!keep){
		cvDestroyWindow("CHANNEL 1");
		cvDestroyWindow("CHANNEL 2");
		cvDestroyWindow("CHANNEL 3");
	}
	for (int i = 0; i < 3; ++i){
		cvReleaseImage(&channels[i]);
		cvReleaseImage(&histImage[i]);
		cvReleaseHist(&hists[i]);
	}
}

void getColorRange(IplImage *frame, CvArr *mask, int *maxIdx, int *loIdx, int *hiIdx){
	CvSize sz = cvGetSize(frame);
	IplImage *channels[3];
	channels[0] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	channels[1] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	channels[2] = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	cvSplit(frame, channels[0], channels[1], channels[2], 0);

	int hist_size = 256;
	int hist_height = 100;
	float range[] = { 0, 255 };
	float *ranges[] = { range };
	CvHistogram *hists[1];
	hists[0] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);
	cvCalcHist(&channels[0], hists[0], 0, mask);
	cvNormalizeHist(hists[0], 1.0);
	cvThreshHist(hists[0], 0.1);

#ifdef DEBUG_MODE
	IplImage *histImage[1];
	histImage[0] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	cvZero(histImage[0]);
#endif

	int ma, lo=256, hi=0;
	cvGetMinMaxHistValue(hists[0], 0, 0, 0, &ma);
	for (int j = 0; j < 256; j++){
		float binValue = cvQueryHistValue_1D(hists[0], j);
		if (binValue != 0.0){
			if (j < lo)lo = j;
			if (j > hi)hi = j;
		}
#ifdef DEBUG_MODE
		int intensity = cvRound(binValue * hist_height);
		cvRectangle(histImage[0], cvPoint(j, hist_height - intensity), cvPoint(j + 1, hist_height), CV_RGB(255, 255, 255), CV_FILLED);
#endif
	}
	std::cout << "ma:" << ma << ", lo:" << lo << ", hi:" << hi << std::endl;
#ifdef DEBUG_MODE
	cvShowImage("CameraLeft", mask);
	cvShowImage("CHANNEL 1", histImage[0]);
	cvWaitKey(0);
	cvDestroyWindow("CHANNEL 1");
	cvReleaseImage(&histImage[0]);
#endif

	cvReleaseImage(&channels[0]);
	cvReleaseHist(&hists[0]);

	*maxIdx = ma;
	*loIdx = lo;
	*hiIdx = hi;
}

//shit..
void getColorRangeN(CvCapture *capture, CvArr *mask, int n, int lo[3], int hi[3]){
	IplImage *frameRaw;
	IplImage *frameBGR = cvCreateImage(cvGetSize(mask), IPL_DEPTH_8U, 3);;
	IplImage *frame = cvCreateImage(cvGetSize(mask), IPL_DEPTH_8U, 3);
	IplImage *channels[3];
	channels[0] = cvCreateImage(cvGetSize(mask), IPL_DEPTH_8U, 1);
	channels[1] = cvCreateImage(cvGetSize(mask), IPL_DEPTH_8U, 1);
	channels[2] = cvCreateImage(cvGetSize(mask), IPL_DEPTH_8U, 1);

	int hist_size = 256;
	int hist_height = 100;
	float range[] = { 0, 255 };
	float *ranges[] = { range };
	CvHistogram *hists[3];
	hists[0] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);
	hists[1] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);
	hists[2] = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges);

	for (int i = 0; i < n; ++i){
		frameRaw = cvQueryFrame(capture);
		cvResize(frameRaw, frameBGR);
		cvCvtColor(frameBGR, frame, CV_BGR2HSV);
		cvSplit(frame, channels[0], channels[1], channels[2], 0);
		cvCalcHist(&channels[0], hists[0], 1, mask);
		cvCalcHist(&channels[1], hists[1], 1, mask);
		cvCalcHist(&channels[2], hists[2], 1, mask);
	}
	cvNormalizeHist(hists[0], 1.0); cvNormalizeHist(hists[1], 1.0); cvNormalizeHist(hists[2], 1.0);
	//cvThreshHist(hists[0], 0.1); cvThreshHist(hists[1], 0.1); cvThreshHist(hists[2], 0.1);
#ifdef DEBUG_MODE
	IplImage *histImage[3];
	histImage[0] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	histImage[1] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	histImage[2] = cvCreateImage(cvSize(hist_size, hist_height), 8, 3);
	cvZero(histImage[0]); cvZero(histImage[1]); cvZero(histImage[2]);
#endif
	for (int i = 0; i < 3; ++i){
		lo[i] = 256, hi[i] = 0;
		for (int j = 0; j < 256; j++){
			float binValue = cvQueryHistValue_1D(hists[i], j);
			if (binValue != 0.0){
				if (j < lo[i])lo[i] = j;
				if (j > hi[i])hi[i] = j;
			}
#ifdef DEBUG_MODE
			int intensity = cvRound(binValue * hist_height);
			cvRectangle(histImage[i], cvPoint(j, hist_height - intensity), cvPoint(j + 1, hist_height), CV_RGB(255, 255, 255), CV_FILLED);
#endif
		}
		std::cout << "lo:" << lo[i] << " hi:" << hi[i] << std::endl;
	}
#ifdef DEBUG_MODE
	cvShowImage("CHANNEL 1", histImage[0]);
	cvShowImage("CHANNEL 2", histImage[1]);
	cvShowImage("CHANNEL 3", histImage[2]);

	cvWaitKey(0);

	cvDestroyWindow("CHANNEL 1");
	cvDestroyWindow("CHANNEL 2");
	cvDestroyWindow("CHANNEL 3");
#endif
	for (int i = 0; i < 3; ++i){
		cvReleaseImage(&channels[i]);
#ifdef DEBUG_MODE
		cvReleaseImage(&histImage[i]);
#endif
		cvReleaseHist(&hists[i]);
	}
}