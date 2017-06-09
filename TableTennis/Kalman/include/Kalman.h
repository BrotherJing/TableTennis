#ifndef KALMAN_INCLUDE
#define KALMAN_INCLUDE

#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

#include <string>

using namespace std;
using namespace cv;

class Kalman
{
public:
    Kalman();
    ~Kalman();

    /**
     * @brief track all tracking process is executed in this function
     */
    void track(float x, float y, float z);

    /**
     * @brief initKalman initialize the Kalman Filter
     */
    void initKalman(float x, float y, float z);

    std::vector<Point3f> predictList;
    std::vector<Point3f> measureList;
    std::vector<Point3f> realList;
    
    /**
     * @brief getCurrentState return the current state of KF
     * @return
     */
    Mat getCurrentState() const;

	Mat getPredictState() const;

    /**
     * @brief drawTrackResult draw the tracking result on image
     */
    void drawTrackResult();

private :

    Mat camCenter;
    Mat KFPredictCenter;
    Mat KFCorrectCenter;

    KalmanFilter KF;
    Mat_<float> measurement;

    float dT, kD, kM;
    static const float RADIUS;
    static const float MASS;
    static const float GRAVITY;
    static const float RHO_A;
    static const float C_D;
    static const float C_M;
    static const float PI;
};

#endif