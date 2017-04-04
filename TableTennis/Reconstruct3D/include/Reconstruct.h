#ifndef HEADER_RECONSTRUCT
#define HEADER_RECONSTRUCT

#include <opencv2/opencv.hpp>

class Reconstruct
{
public:
	Reconstruct(std::string camera_matrix_dir);
	~Reconstruct();
	CvPoint3D32f uv2xyz(CvPoint uvLeft, CvPoint uvRight);
private:
	CvMat *intrinsicMatrix;
	CvMat *distortionCoeffs;
	CvMat *rotationVectors, *rotationMatrixLeft, *rotationMatrixRight;
	CvMat *translationVectors, *translationLeft, *translationRight;
	CvMat *matZScale;
};

#endif