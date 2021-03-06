#include "Main.h"
#include "CropUtils.h"

void cropImage(IplImage *frame, IplImage **dest, CvRect *bboxes, CvRect rect, int *numNeg, int numPos, int negScale, int maxScale, int outputSize){

	CvSize frameSize = cvGetSize(frame);
	
	for(int i=0,j=0; j<numPos; j++){
		int longEdge = rect.width>rect.height?rect.width:rect.height;
		int size = rand()%((maxScale-1)*longEdge) + longEdge + 1;
		double ratio = outputSize*1.0/size;
		int padding_left = rand()%(size-longEdge);
		int padding_top = rand()%(size-longEdge);
		int x = rect.x - padding_left;
		int y = rect.y - padding_top;
		if(x<0||y<0||x+size>frameSize.width||y+size>frameSize.height)continue;
		cvSetImageROI(frame, cvRect(x, y, size, size));
		cvResize(frame, dest[i]);
		CvRect bbox = cvRect(int(padding_left*ratio), int(padding_top*ratio), int(rect.width*ratio), int(rect.height*ratio));//bbox of the object in the patch
		bboxes[i] = bbox;
		cvResetImageROI(frame);
		++i;
	}

	*numNeg=0;
	for(int i=0;i<3;++i){
		for(int j=0;j<3;++j){
			if(i==1&&j==1)continue;
			int rel_x = int((-negScale+(1.0+negScale)/2*i)*rect.width);
			int rel_y = int((-negScale+(1.0+negScale)/2*j)*rect.height);
			int x = rect.x + rel_x;
			int y = rect.y + rel_y;
			if(x<0||y<0||x+negScale*rect.width>frameSize.width||y+negScale*rect.height>frameSize.height){
				continue;
			}
			cvSetImageROI(frame, cvRect(x, y, negScale*rect.width, negScale*rect.height));
			cvResize(frame, dest[numPos+*numNeg]);
			cvResetImageROI(frame);
			*numNeg = *numNeg+1;
		}
	}

}

void cropNegFromBg(IplImage *frame, IplImage **dest, CvRect *bboxes, int *numNeg, int non_bg_idx){
	int j=0;
	for(int i=0;i<*numNeg;++i){
		if(i==non_bg_idx)continue;
		if(bboxes[i].width*SCALE<10||bboxes[i].height*SCALE<10)continue;
		cvSetImageROI(frame, cvRect(bboxes[i].x*SCALE, bboxes[i].y*SCALE, bboxes[i].width*SCALE, bboxes[i].height*SCALE));
		cvResize(frame, dest[j]);
		cvResetImageROI(frame);
		j++;
	}
	*numNeg = j;
}

void stitchImages(IplImage **crops, IplImage *display, CvRect *bboxes, int numNeg, int numPos){
	int numCrops = numNeg + numPos;
	int grid = sqrt(NUM_POS_PER_FRAME+NUM_NEG_PER_FRAME)+1;
	int k = 0;
	for(int i=0;i<grid;++i){
		for(int j=0;j<grid;++j){
			if(k==numCrops)return;
			cvSetImageROI(display, cvRect(i*PATCH_WIDTH, j*PATCH_HEIGHT, PATCH_WIDTH, PATCH_HEIGHT));
			cvCopy(crops[k], display);
			if(k<numPos)
				cvRectangle(display, cvPoint(bboxes[k].x, bboxes[k].y), cvPoint(bboxes[k].x+bboxes[k].width, bboxes[k].y+bboxes[k].height), CVX_RED, 1, CV_AA);
			cvResetImageROI(display);
			++k;
		}
	}
}

void saveNegFromBg(IplImage **crops, ofstream &filenames, ofstream &groundTruth, ofstream &fileBBox, string prefix, char *frameCountStr, int numCrops){
	char idxStr[5];
	string filename;
	double ratio = PROB_MAP_WIDHT*1.0/PATCH_WIDTH;

	IplImage *probMap = cvCreateImage(cvSize(PROB_MAP_WIDHT, PROB_MAP_HEIGHT), IPL_DEPTH_8U, 1);

	for(int i=0;i<numCrops;++i){
		sprintf(idxStr, "%02d", i);
		filename = prefix+frameCountStr+idxStr+"BG";
		cvSaveImage((filename+".jpg").c_str(), crops[i]);
		filenames<<filename<<".jpg 0"<<endl;
		if(OUTPUT_PROB_MAP){
			cvZero(probMap);
			cvSaveImage((filename+".prob.jpg").c_str(), probMap);
			groundTruth<<filename<<".prob.jpg 0"<<endl;
		}
		fileBBox<<"0 0 0 0"<<endl;
	}
}

void saveImages(IplImage **crops, CvRect *bboxes, ofstream &filenames, ofstream &groundTruth, ofstream &fileBBox, string prefix, char *frameCountStr, int numNeg, int numPos){
	char idxStr[5];
	string filename;
	int numCrops = numNeg + numPos;
	double ratio = PROB_MAP_WIDHT*1.0/PATCH_WIDTH;

	IplImage *probMap = cvCreateImage(cvSize(PROB_MAP_WIDHT, PROB_MAP_HEIGHT), IPL_DEPTH_8U, 1);

	for(int i=0;i<numCrops;++i){
		sprintf(idxStr, "%02d", i);
		filename = prefix+frameCountStr+idxStr;
		cvSaveImage((filename+".jpg").c_str(), crops[i]);
		if(i>=numPos){
			filenames<<filename<<".jpg 0"<<endl;
			if(OUTPUT_PROB_MAP){
				cvZero(probMap);
				cvSaveImage((filename+".prob.jpg").c_str(), probMap);
				groundTruth<<filename<<".prob.jpg 0"<<endl;
			}
			fileBBox<<"0 0 0 0"<<endl;
		}else{
			filenames<<filename<<".jpg 1"<<endl;
			if(OUTPUT_PROB_MAP){
				cvZero(probMap);
				cvRectangle(probMap, cvPoint(int(bboxes[i].x*ratio) - PROB_MAP_PADDING, int(bboxes[i].y*ratio) - PROB_MAP_PADDING),
					cvPoint(int((bboxes[i].x+bboxes[i].width)*ratio) + PROB_MAP_PADDING, int((bboxes[i].y+bboxes[i].height)*ratio) + PROB_MAP_PADDING), CVX_WHITE, -1);	
				cvSaveImage((filename+".prob.jpg").c_str(), probMap);
				groundTruth<<filename<<".prob.jpg 1"<<endl;
			}
			fileBBox<<bboxes[i].x<<" "<<bboxes[i].y<<" "<<bboxes[i].x+bboxes[i].width<<" "<<bboxes[i].y+bboxes[i].height<<endl;
		}
	}
}