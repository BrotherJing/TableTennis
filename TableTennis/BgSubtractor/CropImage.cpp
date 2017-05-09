/*
=========================================
required input:
- video

output:
- image patches
- a list of filename and fake label
- ground truth of class label and bounding box

usage:
./bg xxx.mp4 [output_dir]
=========================================
*/

#include <string>
#include "Main.h"
#include "Codebook.h"
#include "Track.h"
#include "CropUtils.h"

using namespace cv;
using namespace std;

IplImage *frame, *Ismall, *IsmallSmooth, *draw;
IplImage *Imask, *ImaskSmall;
IplImage *IsmallHSV;
CvSize sz, szSmall;

//for image cropping
IplImage **crops;
IplImage *cropsDisplay;
CvRect *cropBBoxes;
int numNeg;

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

	crops = new IplImage*[NUM_POS_PER_FRAME+NUM_NEG_PER_FRAME];
	for(int i=0;i<NUM_POS_PER_FRAME+NUM_NEG_PER_FRAME;++i){
		crops[i] = cvCreateImage(cvSize(PATCH_WIDTH, PATCH_HEIGHT), frame->depth, frame->nChannels);
	}
	cropsDisplay = cvCreateImage(cvSize(PATCH_WIDTH*(sqrt(NUM_POS_PER_FRAME+NUM_NEG_PER_FRAME)+1), PATCH_HEIGHT*(sqrt(NUM_POS_PER_FRAME+NUM_NEG_PER_FRAME)+1)), frame->depth, frame->nChannels);
	cropBBoxes = new CvRect[NUM_POS_PER_FRAME];
}

void releaseImages(){
}

bool dirty = false;//if true, that mean we have drawn a bbox manually
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
		dirty = true;
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
		cvCopy(IsmallSmooth, draw);
		cvRectangle(draw, lt, rb, CVX_RED, 1, CV_AA);
		cvShowImage("display", draw);
	}
}

int main(int argc, char **argv){
	
	srand(time(NULL));

	CvFont font = cvFont(1.0);
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0);
	char frameCountStr[10];
	
	string filename = string(argv[1]);
	size_t l = filename.rfind('/')+1, r = filename.find('.');
	filename = string(filename).substr(l, r-l);
	
	char abs_path_buff[PATH_MAX];
	if(!realpath((argv[2]!=NULL?argv[2]:"tabletennis"), abs_path_buff)){
		cout<<"get abs dir error."<<endl;
		return -1;
	}
	string dirname = string(abs_path_buff);
	dirname += "/";

	string prefix = dirname + filename;
	cout<<prefix<<endl;

	ofstream oFileGroundTruth, oFileNames, oFileBBox;
	oFileNames.open((filename+".txt").c_str(), ios::out | ios::trunc);
	oFileGroundTruth.open((filename+".label.txt").c_str(), ios::out | ios::trunc);
	oFileBBox.open((filename+".bbox.txt").c_str(), ios::out|ios::trunc);

	namedWindow("display", WINDOW_AUTOSIZE);
	namedWindow("crops", WINDOW_AUTOSIZE);
	setMouseCallback("display", onMouse);

	CvCapture *capture = cvCreateFileCapture(argv[1]);
	frame = cvQueryFrame(capture);
	if (!frame)return 1;
	allocImages(frame);

	cvResize(frame, Ismall);
	cvShowImage("display", Ismall);

	while (1){
		frameCount++;
		frame = cvQueryFrame(capture);
		if (!frame)break;

		cvResize(frame, IsmallSmooth);
		//cvSmooth(Ismall, IsmallSmooth, CV_MEDIAN, 3);
		cvCvtColor(IsmallSmooth, IsmallHSV, CV_BGR2YCrCb);
		codebook_tick_img(IsmallHSV, codebooks);
		if(frameCount<TRAIN_BG_MODEL_ITER){
			update_codebook_img(IsmallHSV, codebooks, BOUNDS_DEFAULT);
		}else if(frameCount==TRAIN_BG_MODEL_ITER){
			update_codebook_img(IsmallHSV, codebooks, BOUNDS_DEFAULT);
			clear_stale_entries_img(IsmallHSV, codebooks);
		}else{
			background_diff_img(IsmallHSV, ImaskSmall, codebooks, MIN_MOD_DEFAULT, MAX_MOD_DEFAULT);
			numComponents = MAX_COMPONENT;
			find_connected_component(ImaskSmall, &numComponents, bboxes);
			draw_connected_components(IsmallSmooth, numComponents, bboxes);
		}
		sprintf(frameCountStr, "%04d", frameCount);
		cvPutText(IsmallSmooth, frameCountStr, cvPoint(0, cvGetSize(IsmallSmooth).height), &font, CVX_WHITE);

		if(frameCount>TRAIN_BG_MODEL_ITER){//training complete
			if(tracker==NULL && numComponents==1){//find something to track
				tracker = new Tracker(bboxes[0]);
			}
			if(!trackBall(tracker, bboxes, numComponents)){//target lost, draw bbox manually
				currentState = STATE_PAUSE;
				dirty = false;
			}else{//target tracked
				int x = tracker->bbox.x;
				int y = tracker->bbox.y;
				cvRectangle(IsmallSmooth, cvPoint(x, y), cvPoint(x+tracker->bbox.width, y+tracker->bbox.height), CVX_GREEN, 1);
				cropImage(frame, crops, cropBBoxes, cvRect(x*SCALE, y*SCALE, tracker->bbox.width*SCALE, tracker->bbox.height*SCALE), &numNeg);
				stitchImages(crops, cropsDisplay, cropBBoxes, numNeg);
				cvShowImage("display", IsmallSmooth);
				cvShowImage("crops", cropsDisplay);
				char c = cvWaitKey(0);
				if(c==KEY_RETURN||c==KEY_ENTER){
					saveImages(crops, cropBBoxes, oFileNames, oFileGroundTruth, oFileBBox, prefix, frameCountStr, numNeg);
					cout<<"image patches for frame "<<frameCount<<" saved"<<endl;
				}else if(c=='d'){
					currentState = STATE_PAUSE;
					dirty = false;
				}
			}
		}
		
		cvShowImage("display", IsmallSmooth);
		cvShowImage("crops", cropsDisplay);
		cvZero(cropsDisplay);
		
		char c = cvWaitKey(20);
		if (c == KEY_ESC)break;
		if(currentState==STATE_PAUSE ||
			currentState==STATE_PLAY && c==KEY_SPACE){
			currentState = STATE_PAUSE;
			char c1 = cvWaitKey(0);
			if(c1==KEY_RETURN||c1==KEY_ENTER){
				if(dirty){
					cropImage(frame, crops, cropBBoxes, cvRect(tracker->bbox.x*SCALE, tracker->bbox.y*SCALE, tracker->bbox.width*SCALE, tracker->bbox.height*SCALE), &numNeg);
					stitchImages(crops, cropsDisplay, cropBBoxes, numNeg);
					cvShowImage("crops", cropsDisplay);
					c1 = cvWaitKey(0);
					if(c1==KEY_RETURN||c1==KEY_ENTER){
						saveImages(crops, cropBBoxes, oFileNames, oFileGroundTruth, oFileBBox, prefix, frameCountStr, numNeg);
						cout<<"image patches for frame "<<frameCount<<" saved"<<endl;
					}
					dirty = false;
					currentState = STATE_PLAY;
				}
			}else if(c1=='n'){
			}else if(c1==KEY_ESC){
				break;
			}else{
				currentState = STATE_PLAY;
			}
		}
	}

	oFileNames.close();
	oFileGroundTruth.close();
	releaseImages();
	cvReleaseCapture(&capture);
	cvDestroyWindow("display");
	return 0;
}
