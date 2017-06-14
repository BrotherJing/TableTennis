/*
=========================================
required input:
- video

output:
- xxx.sequence.xml
- xxx.info.xml
- xxx.bbox.txt(each line is [frame x y w h])

usage:
./bg xxx.mp4 [skip]
=========================================
*/

#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include "Main.h"
#include "Codebook.h"
#include "Track.h"

using namespace cv;
using namespace std;

IplImage *frame, *Ismall, *IsmallSmooth, *draw;
IplImage *Imask, *ImaskSmall;
IplImage *IsmallHSV;
CvSize sz, szSmall;

//for finding connected components
int numComponents = MAX_COMPONENT;
CvRect bboxes[MAX_COMPONENT];

//for bg subtraction
codeBook **codebooks;
unsigned BOUNDS_DEFAULT[3] = {10, 10, 10};
int MIN_MOD_DEFAULT[3] = {20, 20, 20};
int MAX_MOD_DEFAULT[3] = {20, 20, 20};

//for tracking
Tracker *tracker;

int currentState = STATE_PLAY;
int frameCount = 0;
int firstSuccessFrame = 0;
int lastSuccessFrame = 0;
int longestSuccessFrames = 0, startFrame=0, endFrame=0;
int originFrame = 0;
vector<CvPoint> longestPointSequence;
vector<CvPoint> pointSequence;
vector<CvRect> longestBboxSequence, bboxSequence;

void allocImages(IplImage *frame){
	sz = cvGetSize(frame);
	szSmall.width = sz.width / SCALE; szSmall.height = sz.height / SCALE;

	Ismall = cvCreateImage(szSmall, frame->depth, frame->nChannels);
	IsmallSmooth = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	IsmallHSV = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	Imask = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	ImaskSmall = cvCreateImage(szSmall, IPL_DEPTH_8U, 1);

	draw = cvCreateImage(szSmall, frame->depth, frame->nChannels);

	codebooks = new codeBook*[szSmall.height];
	for(int i=0;i<szSmall.height;++i){
		codebooks[i] = new codeBook[szSmall.width];
	}
}

void releaseImages(){
}

void updateLongestSeq(){
	if(lastSuccessFrame - firstSuccessFrame > longestSuccessFrames){
		startFrame = firstSuccessFrame;
		endFrame = lastSuccessFrame;
		longestSuccessFrames = lastSuccessFrame - firstSuccessFrame;
		longestPointSequence = pointSequence;
		longestBboxSequence = bboxSequence;
	}
	pointSequence.clear();
	bboxSequence.clear();
}

bool dirty = false;
bool isDrawing = false;
CvPoint lt, rb;
void onMouse(int event, int x, int y, int flag, void *ustc){
	if(currentState!=STATE_PAUSE)return;
	CvPoint pt = cvPoint(x, y);
	if(event==CV_EVENT_LBUTTONDOWN){
		isDrawing = true;
		lt = pt;
	}else if(event==CV_EVENT_MOUSEMOVE){
		if(isDrawing){
			pt.x = lt.x + (pt.x - lt.x)/2;
			pt.y = lt.y + (pt.y - lt.y)/2;
			cvCopy(IsmallSmooth, draw);
			cvRectangle(draw, lt, pt, CVX_WHITE, 1, CV_AA);
			cvShowImage("display", draw);
		}
	}else if(event==CV_EVENT_LBUTTONUP){
		isDrawing = false;

		cvCopy(IsmallSmooth, draw);
		rb.x = lt.x + (pt.x - lt.x)/2;
		rb.y = lt.y + (pt.y - lt.y)/2;
		int x = lt.x;
		int y = lt.y;
		int w = rb.x-lt.x;
		int h = rb.y-lt.y;
		if(tracker==NULL){
			tracker = new Tracker(cvRect(x, y, w, h));
		}else{
			tracker->set(cvRect(x, y, w, h));
		}
		//cvCircle(draw, pt, 2, CVX_RED, 2);
		cvRectangle(draw, lt, rb, CVX_RED, 1, CV_AA);
		cvShowImage("display", draw);
		cout<<"frame"<<frameCount<<": "<<"("<<x<<","<<y<<")"<<endl;
		if(lastSuccessFrame<frameCount-1){//last frame is not successful..
			updateLongestSeq();
			firstSuccessFrame = frameCount;
		}
		lastSuccessFrame = frameCount;
		if(dirty){
			pointSequence.pop_back();
			bboxSequence.pop_back();
		}
		pointSequence.push_back(cvPoint(tracker->center.x, tracker->center.y));
		bboxSequence.push_back(tracker->bbox);
		dirty = true;
	}
}
/*void onMouse(int event, int x, int y, int flag, void *ustc){
	if(currentState!=STATE_PAUSE)return;
	CvPoint pt = cvPoint(x, y);
	if(event==CV_EVENT_LBUTTONDOWN){
		cvCopy(IsmallSmooth, draw);
		if(tracker==NULL){
			tracker = new Tracker(cvRect(pt.x - DEFAULT_WIDTH/2, pt.y - DEFAULT_HEIGHT/2, DEFAULT_WIDTH, DEFAULT_HEIGHT));
		}else{
			tracker->set(cvRect(pt.x - DEFAULT_WIDTH/2, pt.y - DEFAULT_HEIGHT/2, DEFAULT_WIDTH, DEFAULT_HEIGHT));
		}
		//cvCircle(draw, pt, 2, CVX_RED, 2);
		cvRectangle(draw, cvPoint(tracker->bbox.x, tracker->bbox.y), cvPoint(tracker->bbox.x+tracker->bbox.width, tracker->bbox.y+tracker->bbox.height), CVX_GREEN, 2);
		cvShowImage("display", draw);
		cout<<"frame"<<frameCount<<": "<<"("<<x<<","<<y<<")"<<endl;
		if(lastSuccessFrame<frameCount-1){//last frame is not successful..
			updateLongestSeq();
			firstSuccessFrame = frameCount;
		}
		lastSuccessFrame = frameCount;
		if(dirty){
			pointSequence.pop_back();
		}
		pointSequence.push_back(pt);
		bboxSequence.push_back(tracker->bbox);
		dirty = true;
	}
}*/

int main(int argc, char **argv){
	
	CvFont font = cvFont(1.0);
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0);
	char frameCountStr[10];
	ofstream oFileBBox;

	string filename = string(argv[1]);
	size_t l = filename.rfind('/')+1, r = filename.find('.');
	filename = string(filename).substr(l, r-l);
	cout<<filename<<endl;
	oFileBBox.open((filename+".bbox.txt").c_str(), ios::out | ios::trunc);
	namedWindow("display", WINDOW_AUTOSIZE);
	setMouseCallback("display", onMouse);

	CvCapture *capture = cvCreateFileCapture(argv[1]);
	frame = cvQueryFrame(capture);
	if (!frame)return 1;
	allocImages(frame);

	int skip = 0;
	if(argc>2){
		skip = atoi(argv[2]);
	}

	cvResize(frame, Ismall);
	cvShowImage("display", Ismall);

	while (1){
		frameCount++;
		frame = cvQueryFrame(capture);
		if (!frame)break;
		//cvCvtColor(frame, frame, CV_BGR2RGB);
		if(skip>0&&frameCount<skip)continue;

		cvResize(frame, IsmallSmooth);
		//cvSmooth(Ismall, IsmallSmooth, CV_MEDIAN, 3);
		cvCvtColor(IsmallSmooth, IsmallHSV, CV_BGR2YCrCb);
		codebook_tick_img(IsmallHSV, codebooks);
		if(frameCount-skip<TRAIN_BG_MODEL_ITER){
			update_codebook_img(IsmallHSV, codebooks, BOUNDS_DEFAULT);
		}else{
			if(frameCount%CLEAR_STALE_PER_ITER==0){
				clear_stale_entries_img(IsmallHSV, codebooks);
			}
			background_diff_img(IsmallHSV, ImaskSmall, codebooks, MIN_MOD_DEFAULT, MAX_MOD_DEFAULT);
			numComponents = MAX_COMPONENT;
			find_connected_component(ImaskSmall, &numComponents, bboxes);
			draw_connected_components(IsmallSmooth, numComponents, bboxes);
		}

		if(frameCount-skip>TRAIN_BG_MODEL_ITER){
			if(tracker==NULL && numComponents==1){
				tracker = new Tracker(bboxes[0]);
			}
			if(!trackBall(tracker, bboxes, numComponents)){
				currentState = STATE_PAUSE;
				dirty = false;
			}else{
				//int x = bboxes[0].x+bboxes[0].width/2;
				//int y = bboxes[0].y+bboxes[0].height/2;
				cvRectangle(IsmallSmooth, cvPoint(tracker->bbox.x, tracker->bbox.y), cvPoint(tracker->bbox.x+tracker->bbox.width, tracker->bbox.y+tracker->bbox.height), CVX_GREEN, 2);
				cout<<"frame"<<frameCount<<": "<<"("<<tracker->center.x<<","<<tracker->center.y<<")"<<endl;
				if(lastSuccessFrame<frameCount-1){//last frame is not successful..
					updateLongestSeq();
					firstSuccessFrame = frameCount;
				}
				lastSuccessFrame = frameCount;
				pointSequence.push_back(tracker->center);
				bboxSequence.push_back(tracker->bbox);
			}
		}
		sprintf(frameCountStr, "%d", frameCount);
		cvPutText(IsmallSmooth, frameCountStr, cvPoint(0, cvGetSize(IsmallSmooth).height), &font, CVX_WHITE);
		cvShowImage("display", IsmallSmooth);
		cvShowImage("mask", ImaskSmall);
		
		char c = cvWaitKey(20);
		if (c == KEY_ESC)break;
		if(currentState==STATE_PAUSE ||
			currentState==STATE_PLAY && c==KEY_SPACE){
			currentState = STATE_PAUSE;
			char c1 = cvWaitKey(0);
			if(c1==KEY_RETURN||c1==KEY_ENTER){
				originFrame = frameCount;
				cout<<"frame "<<frameCount<<endl;
			}else if(c1=='n'){
			}else if(c1==KEY_ESC){
				break;
			}else{
				currentState = STATE_PLAY;
			}
		}
	}

	updateLongestSeq();

	cout<<"longest streak: "<<startFrame<<"-"<<endFrame<<endl;
	cout<<"length: "<<longestPointSequence.size()<<endl;
	cout<<"original frame: "<<originFrame<<endl;

	CvMat *result = cvCreateMat(longestPointSequence.size(), 3, CV_32SC1);
	CvMat *info = cvCreateMat(3, 1, CV_32SC1);
	for(int i=0;i<longestPointSequence.size();++i){
		*((int*)CV_MAT_ELEM_PTR(*result, i, 0)) = i+startFrame-originFrame;
		*((int*)CV_MAT_ELEM_PTR(*result, i, 1)) = longestPointSequence[i].x*SCALE;
		*((int*)CV_MAT_ELEM_PTR(*result, i, 2)) = longestPointSequence[i].y*SCALE;
		oFileBBox<<startFrame+i<<' '<<longestBboxSequence[i].x<<' '<<longestBboxSequence[i].y<<' '
			<<longestBboxSequence[i].width<<' '<<longestBboxSequence[i].height<<endl;
	}
	*((int*)CV_MAT_ELEM_PTR(*info, 0, 0)) = startFrame;
	*((int*)CV_MAT_ELEM_PTR(*info, 1, 0)) = originFrame;
	*((int*)CV_MAT_ELEM_PTR(*info, 2, 0)) = endFrame;
	cvSave((filename+".sequence.xml").c_str(), result);
	cvSave((filename+".sequence_info.xml").c_str(), info);
	cout<<filename<<" result saved!"<<endl;
	oFileBBox.close();

	releaseImages();
	cvReleaseCapture(&capture);
	cvDestroyWindow("display");
	return 0;
}
