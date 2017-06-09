#include "Kalman.h"
#include <cmath>

const float Kalman::RADIUS = 0.02;
const float Kalman::MASS = 0.00275;
const float Kalman::GRAVITY = 9.82;
const float Kalman::RHO_A = 1.29;
const float Kalman::C_D = 0.405;
const float Kalman::C_M = 0.62;
const float Kalman::PI = 3.141592653;

Kalman::Kalman():
	dT(0.005),
	kD(0.5*MASS*C_D*RHO_A*PI*RADIUS*RADIUS*dT),
	kM(0.5*MASS*C_M*RHO_A*RADIUS*PI*RADIUS*RADIUS*dT){

}

Kalman::~Kalman(){

}

void Kalman::initKalman(float x, float y, float z){
	const int stateNum = 6;
    const int measureNum = 3;
    const int controlNum = 1;

    Mat statePost = (Mat_<float>(stateNum, 1) << x,y,z,
                     0,0,0);
    Mat transitionMatrix = (Mat_<float>(stateNum, stateNum) << 1, 0, 0, dT, 0, 0,
                            0, 1, 0, 0, dT, 0,
                            0, 0, 1, 0, 0, dT,
                            0, 0, 0, 1, 0, 0,
                            0, 0, 0, 0, 1, 0,
                            0, 0, 0, 0, 0, 1);
    Mat controlMatrix = (Mat_<float>(stateNum, 1) << 0, 0, 0, 0, 0, 1);

    KF.init(stateNum, measureNum, controlNum);

    KF.transitionMatrix = transitionMatrix;
    KF.statePost = statePost;
    KF.controlMatrix = controlMatrix;
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-5));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-3));
    setIdentity(KF.errorCovPost, Scalar::all(1e1));

    measurement = Mat::zeros(measureNum, 1, CV_32F);
	camCenter = (Mat_<float>(3, 1) << x,y,z);
}

Mat Kalman::getCurrentState() const
{
    return KF.statePost;
    //return Point(statePre.at<float>(0), statePre.at<float>(1));
}

Mat Kalman::getPredictState() const
{
    Mat statePre = KF.statePre;
    return statePre;
    //return Point(statePre.at<float>(0), statePre.at<float>(1));
}

void Kalman::track(float x, float y, float z){
	float vx = KF.statePost.at<float>(3);
	float vy = KF.statePost.at<float>(4);
	float vz = KF.statePost.at<float>(5);
	float v = sqrt(vx*vx+vy*vy+vz*vz);
	Mat transitionMatrix = (Mat_<float>(6, 6) << 1, 0, 0, dT, 0, 0,
                            0, 1, 0, 0, dT, 0,
                            0, 0, 1, 0, 0, dT,
                            0, 0, 0, 1-kD*v, 0, 0,
                            0, 0, 0, 0, 1-kD*v, 0,
                            0, 0, 0, 0, 0, 1-kD*v);
    KF.transitionMatrix = transitionMatrix;
	KF.predict((Mat_<float>(1,1)<<-1*GRAVITY*dT));
	KFPredictCenter = getPredictState();
	randn(measurement, Scalar::all(0), Scalar::all(1));
	measurement.at<float>(0) += x;
	measurement.at<float>(1) += y;
	measurement.at<float>(2) += z;
	//camCenter = Point(measurement.at<float>(0), measurement.at<float>(1));
	//measurement.at<float>(0) = i*i;
	//measurement.at<float>(1) = i*i;
	KF.correct(measurement);
	//correctCenter = getCurrentState();
	//correctCenter = Point(measurement.at<float>(0), measurement.at<float>(1));
	//last.at<float>(0) = correctCenter.x;
	//last.at<float>(1) = correctCenter.y;
	drawTrackResult();
	camCenter.at<float>(0) = x;
	camCenter.at<float>(1) = y;
	camCenter.at<float>(2) = z;
}

void Kalman::drawTrackResult()
{
    /*Mat image;

    currentFrame.copyTo(image);

    circle(image, camCenter, 2, Scalar(255,0,0), 2, CV_AA); // draw camshift result
    circle(image, KFPredictCenter, 2, Scalar(0,255,0), 2, CV_AA); // draw kalman predict result
    circle(image, KFCorrectCenter, 2, Scalar(0,0,255), 2, CV_AA); // draw kalman correct result
    rectangle(image, trackWindow, Scalar(0,0,255), 3, CV_AA); // draw track window

    imshow(winName, image);*/
    predictList.push_back(Point3f(KFPredictCenter.at<float>(0), KFPredictCenter.at<float>(1), KFPredictCenter.at<float>(2)));
    measureList.push_back(Point3f(measurement.at<float>(0), measurement.at<float>(1), measurement.at<float>(2)));
    realList.push_back(Point3f(camCenter.at<float>(0), camCenter.at<float>(1), camCenter.at<float>(2)));
}