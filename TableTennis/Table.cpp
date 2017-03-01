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

void findVertices(IplImage *frame, IplImage *tableArea, CvRect *tableBBox){
	static CvMemStorage *storage;
	if (storage == NULL){
		storage = cvCreateMemStorage(0);
	}
	else{
		cvClearMemStorage(storage);
	}
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
		//cvLine(frame, cvPoint(0, k_b[i][1]), cvPoint(-k_b[i][1] / k_b[i][0], 0), CVX_WHITE, 1, CV_AA, 0);
		CvPoint p1 = cvPoint(0, k_b[i][1]);// cvPoint(0, k_b[i][1] / sin(k_b[i][0]));
		CvPoint p2 = cvPoint(-k_b[i][1] / k_b[i][0], 0);// cvPoint(k_b[i][1] / cos(k_b[i][0]), 0);
		//cvLine(frame, p1, p2, CVX_WHITE, 1, CV_AA, 0);
		//printPoint(&p1); printPoint(&p2);
	}
	cvShowImage("CameraLeft", tableArea);
	cvShowImage("MaskLeft", frame);
	cvWaitKey(0);

	//find 4 borders
	IplImage *temp = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
	CvPoint *pts = new CvPoint[4];
	int npts[] = { 4 };
	CvPoint tableCenter = cvPoint(tableBBox->x + (tableBBox->width / 2), tableBBox->y + (tableBBox->height / 2));

	double bestScore = 0.0;
	CvPoint *bestPoint = new CvPoint[4];

	for (int i1 = 0; i1 < total-3; ++i1){
		for (int i2 = i1+1; i2 < total-2; ++i2){
			//double y12 = (k_b[i2][1] / k_b[i2][0] - k_b[i1][1] / k_b[i1][0]) / (tan(k_b[i2][0] - tan(k_b[i1][0])));
			//double x12 = (k_b[i1][1] - y12*sin(k_b[i1][0])) / cos(k_b[i1][0]);
			for (int i3 = i2+1; i3 < total-1; ++i3){
				//double y23 = (k_b[i3][1] / k_b[i3][0] - k_b[i2][1] / k_b[i2][0]) / (tan(k_b[i3][0] - tan(k_b[i2][0])));
				//double x23 = (k_b[i2][1] - y23*sin(k_b[i2][0])) / cos(k_b[i2][0]);
				for (int i4 = i3+1; i4 < total; ++i4){
					//double y34 = (k_b[i4][1] / k_b[i4][0] - k_b[i3][1] / k_b[i3][0]) / (tan(k_b[i4][0] - tan(k_b[i3][0])));
					//double x34 = (k_b[i3][1] - y34*sin(k_b[i3][0])) / cos(k_b[i3][0]);
					//double y41 = (k_b[i1][1] / k_b[i1][0] - k_b[i4][1] / k_b[i4][0]) / (tan(k_b[i1][0] - tan(k_b[i4][0])));
					//double x41 = (k_b[i4][1] - y41*sin(k_b[i4][0])) / cos(k_b[i4][0]);

					double angles[4];
					int idx[] = { 0, 1, 2, 3 };
					angles[0] = computeAngle(tableCenter, k_b[i1][0], k_b[i1][1]);
					angles[1] = computeAngle(tableCenter, k_b[i2][0], k_b[i2][1]);
					angles[2] = computeAngle(tableCenter, k_b[i3][0], k_b[i3][1]);
					angles[3] = computeAngle(tableCenter, k_b[i4][0], k_b[i4][1]);
					sortAngles(angles, idx);
					int x12 = (int)(-(k_b[idx[1]][1] - k_b[idx[0]][1]) / (k_b[idx[1]][0] - k_b[idx[0]][0]));
					int y12 = (int)(k_b[idx[0]][0] * x12 + k_b[idx[0]][1]);
					pts[0] = cvPoint(x12, y12);
					int x23 = (int)(-(k_b[idx[2]][1] - k_b[idx[1]][1]) / (k_b[idx[2]][0] - k_b[idx[1]][0]));
					int y23 = (int)(k_b[idx[1]][0] * x23 + k_b[idx[1]][1]);
					pts[1] = cvPoint(x23, y23);
					int x34 = (int)(-(k_b[idx[3]][1] - k_b[idx[2]][1]) / (k_b[idx[3]][0] - k_b[idx[2]][0]));
					int y34 = (int)(k_b[idx[2]][0] * x34 + k_b[idx[2]][1]);
					pts[2] = cvPoint(x34, y34);
					int x41 = (int)(-(k_b[idx[0]][1] - k_b[idx[3]][1]) / (k_b[idx[0]][0] - k_b[idx[3]][0]));
					int y41 = (int)(k_b[idx[3]][0] * x41 + k_b[idx[3]][1]);
					pts[3] = cvPoint(x41, y41);

					cvZero(frame);
					cvCircle(frame, pts[0], 4, CVX_WHITE, 1, CV_AA, 0);
					cvCircle(frame, pts[1], 4, CVX_WHITE, 1, CV_AA, 0);
					cvCircle(frame, pts[2], 4, CVX_WHITE, 1, CV_AA, 0);
					cvCircle(frame, pts[3], 4, CVX_WHITE, 1, CV_AA, 0);
					cvLine(frame, lines[idx[0]][0], lines[idx[0]][1], CVX_WHITE, 1, CV_AA, 0);
					cvLine(frame, lines[idx[1]][0], lines[idx[1]][1], CVX_WHITE, 1, CV_AA, 0);
					cvLine(frame, lines[idx[2]][0], lines[idx[2]][1], CVX_WHITE, 1, CV_AA, 0);
					cvLine(frame, lines[idx[3]][0], lines[idx[3]][1], CVX_WHITE, 1, CV_AA, 0);
					cvFillPoly(frame, &pts, npts, 0, CVX_WHITE, CV_AA);
					cvShowImage("CameraLeft", tableArea);
					cvShowImage("MaskLeft", frame);
					cvOr(frame, tableArea, temp);
					CvScalar unionArea = cvAvg(temp);
					cvAnd(frame, tableArea, temp);
					CvScalar intersectArea = cvAvg(temp);
					double score = intersectArea.val[0] * 1.0 / unionArea.val[0];
					printPoint(&pts[0]); printPoint(&pts[1]); printPoint(&pts[2]); printPoint(&pts[3]);
					cout << "new score: " << score << " "<<unionArea.val[0]<<" "<<intersectArea.val[0]<<endl;
					cvWaitKey(0);
					if (score > bestScore){
						bestScore = score;
						bestPoint[0] = pts[0];
						bestPoint[1] = pts[1];
						bestPoint[2] = pts[2];
						bestPoint[3] = pts[3];
					}
				}
			}
		}
	}
	//cvFillPoly(frame, &bestPoint, npts, 0, CVX_WHITE, CV_AA);

	for (int i = 0; i < lineseq->total; i++){ delete lines[i]; delete k_b[i]; }
	delete[] lines;
	delete[] k_b;
}

void printPoint(CvPoint *point){
	cout << "(" << point->x << ", " << point->y << ")" << endl;
}

double computeAngle(CvPoint center, double k, double b){
	double k1 = -1.0 / k;
	double b1 = center.y - center.x*k1;
	double x1 = (b1 - b) / (k - k1);
	double y1 = k*x1 + b;
	return atan2(y1 - center.y, x1 - center.x);
}

void sortAngles(double *angles, int *idx){
	double temp, min;
	int temp2, minidx;
	for (int i = 0; i < 3; ++i){
		min = angles[i];
		for (int j = i + 1; j < 4; ++j){
			if (angles[j] < min){
				min = angles[j];
				minidx = j;
			}
		}
		temp = angles[minidx];
		angles[minidx] = angles[i];
		angles[i] = temp;
		temp2 = idx[minidx];
		idx[minidx] = idx[i];
		idx[i] = temp2;
	}
}