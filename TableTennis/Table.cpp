#include "Camera.h"
#include "Tools.h"

using namespace std;

void findTableArea(IplImage *frame, IplImage *mask, int lo, int hi, int vlo, int vhi){
	IplImage *hsv = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	IplImage *hue = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	IplImage *sat = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	IplImage *v = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	IplImage *maskTemp = cvCreateImage(cvGetSize(frame), mask->depth, 1);

	cvCvtColor(frame, hsv, CV_BGR2HSV);
	cvSplit(hsv, hue, sat, v, 0);
	cvInRangeS(hue, cvScalar(lo), cvScalar(hi), mask);
	cvInRangeS(v, cvScalar(vlo), cvScalar(vhi), maskTemp);
	cvAnd(mask, maskTemp, mask);

	cvReleaseImage(&hsv);
	cvReleaseImage(&hue);
	cvReleaseImage(&v);
	cvReleaseImage(&sat);
	cvReleaseImage(&maskTemp);
}

void findTable(IplImage *mask, CvRect *tableBBox){
	static CvMemStorage *mem_storage = NULL;
	static CvSeq *contours = NULL;
	CvRect golden = getGoldenRect(cvGetSize(mask));
	cvMorphologyEx(mask, mask, 0, 0, CV_MOP_OPEN, MORPH_OPEN_ITER);//open
	if (mem_storage == NULL){
		mem_storage = cvCreateMemStorage(0);
	}else{
		cvClearMemStorage(mem_storage);
	}
	//find contours
	CvContourScanner scanner = cvStartFindContours(mask, mem_storage,
		sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	CvSeq *c;
	while ((c = cvFindNextContour(scanner)) != NULL){
		double tmparea = fabs(cvContourArea(c));
		if (tmparea<TABLE_AREA_THRESHOLD||//area too small
			getRectScore(cvBoundingRect(c), golden)<0.2f){
			cvSubstituteContour(scanner, NULL);
		}
	}
	contours = cvEndFindContours(&scanner);
	IplImage *copy = cvCreateImage(cvGetSize(mask), IPL_DEPTH_8U, 1);
	cvZero(mask);
	drawConnectedComponents(contours, mask);
	cvDilate(mask, mask, 0, MORPH_DILATE_ITER);
	cvCopy(mask, copy);

	int n = cvFindContours(copy, mem_storage, &contours);
	CvRect bbox = cvBoundingRect(contours);
	tableBBox->x = bbox.x;
	tableBBox->y = bbox.y;
	tableBBox->width = bbox.width;
	tableBBox->height = bbox.height;

	cvReleaseImage(&copy);
}

CvRect getGoldenRect(CvSize size){
	int width = size.width, height = size.height;
	return cvRect(width * 3 / 11, height * 3 / 11, width * 5 / 11, height * 5 / 11);
}

float getRectScore(CvRect candidate, CvRect golden){
	int left = max(candidate.x, golden.x);
	int right = min(candidate.x + candidate.width, golden.x + golden.width);
	int top = max(candidate.y, golden.y);
	int bottom = min(candidate.y + candidate.height, golden.y + golden.height);
	if (left > right || top > bottom){
		return 0;
	}
	return (right - left)*(bottom - top)*1.0f / candidate.width*candidate.height;
}

void findEdges(IplImage *frame, IplImage *mask){
	IplImage *hsv = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	IplImage *hue = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	IplImage *sat = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	IplImage *v = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	IplImage *mask1 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
	IplImage *mask2 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
	
	/*cvCvtColor(frame, hsv, CV_BGR2HSV);
	cvSplit(hsv, hue, sat, v, 0);
	cvInRangeS(sat, cvScalarAll(0), cvScalarAll(80), mask1);
	cvInRangeS(v, cvScalarAll(150), cvScalarAll(255), mask2);
	cvAnd(mask1, mask2, mask1);
	cvAnd(mask1, mask, mask);*/
	cvCanny(frame, mask1, 50, 150);
	cvAnd(mask1, mask, mask);

	cvReleaseImage(&hsv);
	cvReleaseImage(&hue);
	cvReleaseImage(&v);
	cvReleaseImage(&sat);
	cvReleaseImage(&mask1);
	cvReleaseImage(&mask2);
}

bool findVertices(IplImage *frame, CvPoint2D32f *pts){
	bool success = true;
	static CvMemStorage *storage;
	if (storage == NULL){
		storage = cvCreateMemStorage(0);
	}
	else{
		cvClearMemStorage(storage);
	}
	CvFont font = cvFont(1.0);
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0);
	char lineStr[10];
	CvSeq *lineseq;

	lineseq = cvHoughLines2(frame, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 360, 10, 50, 10);

	//remove overlapping lines.
	//cvZero(frame);
	CvPoint **lines = new CvPoint*[lineseq->total];
	double **k_b = new double*[lineseq->total];
	for (int i = 0; i<lineseq->total; ++i){
		lines[i] = new CvPoint[2];
		k_b[i] = new double[2];
	}
	int total = 0;
	bool found = false;
	for (int i = 0; i<lineseq->total; ++i){
		found = false;
		CvPoint* line = (CvPoint*)cvGetSeqElem(lineseq, i);
		//double theta2 = atan((line[0].x - line[1].x)*1.0 / (line[1].y - line[0].y));
		//double rho2 = line[0].x*cos(theta2) + line[0].y*sin(theta2);
		double k2 = (line[0].y - line[1].y)*1.0 / (line[0].x - line[1].x);
		double b2 = line[0].y - line[0].x*k2;
		for (int j = 0; j<total; ++j){
			CvPoint *prevLine = lines[j];
			//double theta1 = atan((prevLine[0].x - prevLine[1].x)*1.0 / (prevLine[1].y - prevLine[0].y));
			//double rho1 = prevLine[0].x*cos(theta1) + prevLine[0].y*sin(theta1);
			double k1 = (prevLine[0].y - prevLine[1].y)*1.0 / (prevLine[0].x - prevLine[1].x);
			double b1 = prevLine[0].y - prevLine[0].x*k1;
			if (fabs(atan(k1) - atan(k2))<CV_PI/180&&fabs(b1 - b2)<10){//too close to each other
			//if (fabs(theta1 - theta2)<CV_PI/180&&fabs(rho1 - rho2)<5){//too close to each other
				found = true;
				double len1 = (prevLine[0].y - prevLine[1].y)*(prevLine[0].y - prevLine[1].y) + (prevLine[0].x - prevLine[1].x)*(prevLine[0].x - prevLine[1].x);
				double len2 = (line[0].y - line[1].y)*(line[0].y - line[1].y) + (line[0].x - line[1].x)*(line[0].x - line[1].x);
				if (len1<len2){
					prevLine[0] = line[0];
					prevLine[1] = line[1];
					k_b[j][0] = k2;
					k_b[j][1] = b2;
				}
				break;
			}
		}
		if (!found){
			lines[total][0] = line[0];
			lines[total][1] = line[1];
			k_b[total][0] = k2;
			k_b[total][1] = b2;
			total++;
		}
	}
	for (int i = 0; i < total; ++i){
		cvLine(frame, lines[i][0], lines[i][1], CVX_WHITE, 1, CV_AA, 0);
		sprintf(lineStr, "%d", i);
		cvPutText(frame, lineStr, cvPoint((lines[i][0].x + lines[i][1].x) / 2, (lines[i][0].y + lines[i][1].y) / 2), &font, CVX_WHITE);
		//cvLine(frame, cvPoint(0, k_b[i][1]), cvPoint(-k_b[i][1] / k_b[i][0], 0), CVX_WHITE, 1, CV_AA, 0);
		CvPoint p1 = cvPoint(0, k_b[i][1]);// cvPoint(0, k_b[i][1] / sin(k_b[i][0]));
		CvPoint p2 = cvPoint(-k_b[i][1] / k_b[i][0], 0);// cvPoint(k_b[i][1] / cos(k_b[i][0]), 0);
		//cvLine(frame, p1, p2, CVX_WHITE, 1, CV_AA, 0);
		//printPoint(&p1); printPoint(&p2);
	}
	cvShowImage("MaskLeft", frame);
	int idx[4];
	float x12, y12, x23, y23, x34, y34, x41, y41;
	for (int i = 0; i < 4; ++i){
		char c = cvWaitKey(0);
		idx[i] = c - '0';
		if (idx[i]<0 || idx[i]>total){
			success = false;
			goto END;
		}
		cvLine(frame, lines[idx[i]][0], lines[idx[i]][1], CVX_WHITE, 2, CV_AA, 0);
		cvShowImage("MaskLeft", frame);
	}
	cout << idx[0] << idx[1] << idx[2] << idx[3] << endl;

	x12 = (float)(-(k_b[idx[1]][1] - k_b[idx[0]][1]) / (k_b[idx[1]][0] - k_b[idx[0]][0]));
	y12 = (float)(k_b[idx[0]][0] * x12 + k_b[idx[0]][1]);
	pts[0] = cvPoint2D32f(x12, y12);
	x23 = (float)(-(k_b[idx[2]][1] - k_b[idx[1]][1]) / (k_b[idx[2]][0] - k_b[idx[1]][0]));
	y23 = (float)(k_b[idx[1]][0] * x23 + k_b[idx[1]][1]);
	pts[1] = cvPoint2D32f(x23, y23);
	x34 = (float)(-(k_b[idx[3]][1] - k_b[idx[2]][1]) / (k_b[idx[3]][0] - k_b[idx[2]][0]));
	y34 = (float)(k_b[idx[2]][0] * x34 + k_b[idx[2]][1]);
	pts[2] = cvPoint2D32f(x34, y34);
	x41 = (float)(-(k_b[idx[0]][1] - k_b[idx[3]][1]) / (k_b[idx[0]][0] - k_b[idx[3]][0]));
	y41 = (float)(k_b[idx[3]][0] * x41 + k_b[idx[3]][1]);
	pts[3] = cvPoint2D32f(x41, y41);

	for (int i = 0; i < 4; ++i){
		cvCircle(frame, cvPoint((int)pts[i].x, (int)pts[i].y), 5, CVX_WHITE);
	}
END:
	for (int i = 0; i < lineseq->total; i++){ delete lines[i]; delete k_b[i]; }
	delete[] lines;
	delete[] k_b;
	return success;
}