/*
usage:
./camshift <video> <ground_truth_path>
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "Codebook.h"
//#include "Kalman.h"

using namespace std;
using namespace cv;

const double LOW_PASS_FILTER = 0.9;
const int BG_TRAIN_ITER = 50;

Mat extractTargetModel(Mat frame, Rect trackWindow){
	int vMin = 150;
    int vMax = 256;
	int sMin = 0;
	Mat hsv, mask;
	cvtColor(frame, hsv, CV_BGR2HSV);
	inRange(hsv, Scalar(0, sMin, MIN(vMin, vMax)), Scalar(180, 256, MAX(vMin, vMax)), mask);

	Mat ROIMask(mask, trackWindow);
	Mat hist;

	float rangeH[] = {0, 180};
	float rangeS[] = {0, 255};
	const float* range[] = {rangeH, rangeS};
	const int channels[] = {0,1};
	const int histSize[] = {200,100};
	Mat ROI(hsv, trackWindow);
	calcHist(&ROI, 1, channels, ROIMask, hist, 2, histSize, range);
	normalize(hist, hist, 0, 255, CV_MINMAX);

	return hist;
}

Rect running_avg(Rect last, Rect current){
	int new_width = int(last.width*LOW_PASS_FILTER + current.width*(1 - LOW_PASS_FILTER));
	int new_height = int(last.height*LOW_PASS_FILTER + current.height*(1 - LOW_PASS_FILTER));
	if(new_width<10)new_width=10;
	if(new_height<10)new_height=10;
	int center_x = current.x + current.width/2;
	int center_y = current.y + current.height/2;
	return Rect(center_x - new_width/2, center_y - new_height/2, new_width, new_height);
}

Rect get_safe_context(Mat &frame, Rect &bbox, float padding_ratio){
	int x,y;
	int diff = bbox.width - bbox.height;
	int longer_edge = diff>0?bbox.width:bbox.height;
	int padding = longer_edge*padding_ratio;
	int padding_left = padding, padding_right = padding, padding_top = padding, padding_bottom = padding;
	if(diff>0){
		x = bbox.x;
		y = bbox.y - diff/2;
	}else{
		y = bbox.y;
		x = bbox.x - diff/2;
	}
	if(x - padding_left < 0){
		padding_right += (padding_left - x);
		padding_left = x;
	}
	if(y - padding_top < 0){
		padding_bottom += (padding_top - y);
		padding_top = y;
	}
	if(x + longer_edge + padding_right > frame.cols){
		padding_left += padding_right - (frame.cols - (x + longer_edge));
		padding_right = frame.cols - (x + longer_edge);
	}
	if(y + longer_edge + padding_bottom > frame.rows){
		padding_top += padding_bottom - (frame.rows - (y + longer_edge));
		padding_bottom = frame.rows - (y + longer_edge);
	}
	//the area to crop from the frame
	return Rect(x - padding_left,
		y - padding_top,
		longer_edge + padding_left + padding_right,
		longer_edge + padding_top + padding_bottom);
}

Rect track(Mat frame, Mat *mask1, Mat &hist, Rect trackWindow){
	int vMin = 150;
    int vMax = 256;
	int sMin = 0;
	Mat hsv, mask, backProject;
	cvtColor(frame, hsv, CV_BGR2HSV);
    inRange(hsv, Scalar(0, sMin, MIN(vMin, vMax)), Scalar(180, 256, MAX(vMin, vMax)), mask);

	float rangeH[] = {0, 180};
	float rangeS[] = {0, 255};
	const float* range[] = {rangeH, rangeS};
	const int channels[] = {0,1};
	calcBackProject(&hsv, 1, channels, hist, backProject, range);

	backProject &= mask;
	(*mask1)(get_safe_context(frame, trackWindow, 0.2)) = 1;
	//backProject &= *mask1;

	imshow("back", backProject);

	Rect last = Rect(trackWindow);
	TermCriteria term(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 );
	RotatedRect box = CamShift(backProject, trackWindow, term);
	Point camCenter = Point(box.center.x, box.center.y);
	return running_avg(last, box.boundingRect());
}

int main(int argc, char **argv){
	
	Rect trackWindow;
	string video_path = argv[1];
	string ground_truth_path = argv[2];

	VideoCapture video(video_path);

	ifstream ground_truth;
	ground_truth.open(ground_truth_path.c_str());

	ofstream prediction;
	prediction.open("prediction.txt", ios::out | ios::trunc);

	int startFrame, currentFrame, x,y,w,h;
	ground_truth>>startFrame>>x>>y>>w>>h;
	trackWindow = Rect(x,y,w,h);

	Mat frame, temp;
	video>>frame;
	if(frame.empty())return -1;
	BgSubtractor bg(frame.size(), BG_TRAIN_ITER, 4);

	//struct timeval t1,t2;
	//double timeuse;
	namedWindow("display", WINDOW_AUTOSIZE);
	int frameCount = 0;
	bool success;
	Mat hist;
	while(true){
		video>>frame;
		if(frame.empty())break;
		cvtColor(frame, frame, CV_BGR2RGB);
		frameCount++;
		if(frameCount<=BG_TRAIN_ITER){
			cvtColor(frame, temp, CV_BGR2YCrCb);
			bg.process(temp);
			continue;
		}else if(frameCount<startFrame){
			continue;
		}else if(frameCount==startFrame){
			hist = extractTargetModel(frame, trackWindow);
		}else{
			cvtColor(frame, temp, CV_BGR2YCrCb);
			bg.process(temp);
			Mat *mask = new Mat(bg.mask, 0);
			Rect result = track(frame, mask, hist, trackWindow);
			trackWindow = result;
		}
		rectangle(frame, trackWindow, Scalar(255,0,0), 3, CV_AA);
		imshow("display", frame);
		int key = waitKey(20);
		if(key==27)break;

		if(ground_truth.eof())break;
		ground_truth>>currentFrame>>x>>y>>w>>h;
		prediction<<frameCount<<' '<<trackWindow.x<<' '<<trackWindow.y<<' '<<trackWindow.width<<' '<<trackWindow.height<<endl;
	}
	ground_truth.close();
	prediction.close();
	return 0;
}