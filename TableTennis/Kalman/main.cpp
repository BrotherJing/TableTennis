/*
usage:
./kalman <coords.txt>
*/

#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <fstream>
#include <iostream>

#include "Kalman.h"

using namespace std;
using namespace cv;

KalmanFilter KF;
Mat measurement, last;
Point predictCenter, correctCenter, camCenter;

void initKalman(float x, float y)
{
    const int stateNum = 4;
    const int measureNum = 2;

    Mat statePost = (Mat_<float>(stateNum, 1) << x,y,
                     0,0);
    Mat transitionMatrix = (Mat_<float>(stateNum, stateNum) << 1, 0, 1, 0,
                            0, 1, 0, 1,
                            0, 0, 1, 0,
                            0, 0, 0, 1);

    KF.init(stateNum, measureNum);

    KF.transitionMatrix = transitionMatrix;
    KF.statePost = statePost;
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-5));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-3));
    setIdentity(KF.errorCovPost, Scalar::all(1e1));

    measurement = Mat::zeros(measureNum, 1, CV_32F);
    last = Mat::zeros(measureNum, 1, CV_32F);
    last.at<float>(0) = x;
    last.at<float>(1) = y;
    camCenter = Point(x, y);
}

Point getCurrentState()
{
    Mat statePost = KF.statePost;
    return Point(statePost.at<float>(0), statePost.at<float>(1));
}

Point getPredictState()
{
    Mat statePre = KF.statePre;
    return Point(statePre.at<float>(0), statePre.at<float>(1));
}

void drawTrackResult(Mat image)
{
	image = Scalar::all(0);

    circle(image, camCenter, 4, Scalar(255,0,0), 2, CV_AA); // draw camshift result
    circle(image, predictCenter, 2, Scalar(0,255,0), 2, CV_AA); // draw kalman predict result
    circle(image, correctCenter, 2, Scalar(0,0,255), 2, CV_AA); // draw kalman correct result
    
    imshow("display", image);
}

int main(int argc, char **argv){
	float x, y, z;
    Mat image(1000, 1000, CV_8UC3);
	if(argc>1){
		bool init = false;
		Kalman kalman;
		ifstream fin;
		fin.open(argv[1]);
		while(!fin.eof()){
			fin>>x>>y>>z;
			if(!init){
				kalman.initKalman(x,y,z);
				init = true;
			}else{
				kalman.track(x,y,z);
			}
		}
		fin.close();
		ofstream predict, measure, real;
		predict.open("predict.txt", ios::out|ios::trunc);
		measure.open("measure.txt", ios::out|ios::trunc);
		real.open("real.txt", ios::out|ios::trunc);
		for(int i=0;i<kalman.predictList.size();++i){
			predict<<kalman.predictList[i].x<<' '<<kalman.predictList[i].y<<' '<<kalman.predictList[i].z<<endl;
			measure<<kalman.measureList[i].x<<' '<<kalman.measureList[i].y<<' '<<kalman.measureList[i].z<<endl;
			real<<kalman.realList[i].x<<' '<<kalman.realList[i].y<<' '<<kalman.realList[i].z<<endl;
		}
		predict.close();
		measure.close();
		real.close();
	}else{
		initKalman(100,100);
		for(int i=100;i<900;i+=1){
			KF.predict();
			predictCenter = getPredictState();
			randn(measurement, Scalar::all(0), Scalar::all(10));
			measurement.at<float>(1) += i;
			measurement.at<float>(0) += i;
			//camCenter = Point(measurement.at<float>(0), measurement.at<float>(1));
			//measurement.at<float>(0) = i*i;
			//measurement.at<float>(1) = i*i;
			KF.correct(measurement);
			//correctCenter = getCurrentState();
			correctCenter = Point(measurement.at<float>(0), measurement.at<float>(1));
			//last.at<float>(0) = correctCenter.x;
			//last.at<float>(1) = correctCenter.y;
			drawTrackResult(image);
			camCenter = Point(i, i);
			int key = waitKey(0);
			if(key==27)break;
		}
	}
}