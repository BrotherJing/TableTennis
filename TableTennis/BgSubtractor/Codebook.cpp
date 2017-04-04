#include "Codebook.h"
#include <cstdio>

void codebook_tick_img(IplImage *frame, codeBook **codebooks){
	CvSize sz = cvGetSize(frame);
	for(int y=0;y<sz.height;++y){
		for(int x=0;x<sz.width;++x){
			codebook_tick(codebooks[y][x]);
		}
	}
}

void update_codebook_img(IplImage *frame, codeBook **codebooks, unsigned *cbBounds){
	CvSize sz = cvGetSize(frame);
	for(int y=0;y<sz.height;++y){
		uchar *p = (uchar *)(frame->imageData + y*frame->widthStep);
		for(int x=0;x<sz.width;++x){
			update_codebook(p+x*3, codebooks[y][x], cbBounds, 3);
		}
	}
}

void clear_stale_entries_img(IplImage *frame, codeBook **codebooks){
	int numCleared = 0;
	CvSize sz = cvGetSize(frame);
	for(int y=0;y<sz.height;++y){
		for(int x=0;x<sz.width;++x){
			numCleared += clear_stale_entries(codebooks[y][x]);
		}
	}
	//std::cout<<"entry cleared: "<<numCleared<<std::endl;
}

void codebook_tick(codeBook &c){
	c.t++;
}

void background_diff_img(IplImage *frame, IplImage *mask, codeBook **codebooks, int *minMod, int *maxMod){
	CvSize sz = cvGetSize(frame);
	cvZero(mask);
	for(int y=0;y<sz.height;++y){
		uchar *p = (uchar *)(frame->imageData + y*frame->widthStep);
		uchar *p_mask = (uchar *)(mask->imageData + y*mask->widthStep);
		for(int x=0;x<sz.width;++x){
			*(p_mask+x) = background_diff(p+x*3, codebooks[y][x], 3, minMod, maxMod);
		}
	}
}

int update_codebook(uchar *p, codeBook& c, unsigned* cbBounds, int numChannels){
	unsigned int high[3], low[3];
	for(int n=0;n<numChannels;++n){
		high[n] = *(p+n)+*(cbBounds+n);//high for channel n
		if(high[n]>255)high[n]=255;
		low[n] = *(p+n)-*(cbBounds+n);
		if(low[n]<0)low[n]=0;
	}
	int matchChannel;
	int i;
	//see if it fits a codebook
	for(i=0;i<c.numEntries;++i){
		matchChannel = 0;
		for(int n=0;n<numChannels;++n){
			if(c.cb[i]->learnLow[n]<=*(p+n) &&
				c.cb[i]->learnHigh[n]>=*(p+n)){
				matchChannel++;
			}
		}
		if(matchChannel==numChannels){
			c.cb[i]->t_last_update = c.t;
			for(int n=0;n<numChannels;++n){
				if(c.cb[i]->max[n]<*(p+n)){
					c.cb[i]->max[n]=*(p+n);
				}else if(c.cb[i]->min[n]>*(p+n)){
					c.cb[i]->min[n]=*(p+n);
				}
			}
			break;
		}
	}
	//stale = max negative runs
	for(int s=0;s<c.numEntries;++s){
		int negRun = c.t - c.cb[s]->t_last_update;
		if(c.cb[s]->stale < negRun)c.cb[s]->stale = negRun;
	}
	//if no fit found
	if(i==c.numEntries){
		code_element **foo = new code_element*[c.numEntries+1];
		for(int i1=0;i1<c.numEntries;++i1){
			foo[i1] = c.cb[i1];
		}
		foo[c.numEntries] = new code_element;
		if(c.numEntries)delete [] c.cb;
		c.cb = foo;
		for(int n=0;n<numChannels;++n){
			c.cb[c.numEntries]->learnHigh[n] = high[n];
			c.cb[c.numEntries]->learnLow[n] = low[n];
			c.cb[c.numEntries]->max[n] = *(p+n);
			c.cb[c.numEntries]->min[n] = *(p+n);
		}
		c.cb[c.numEntries]->t_last_update = c.t;
		c.cb[c.numEntries]->stale = 0;
		c.numEntries += 1;
	}
	//adjust learning boundary
	for(int n=0;n<numChannels;++n){
		if(c.cb[i]->learnHigh[n]<high[n])c.cb[i]->learnHigh[n]+=1;
		if(c.cb[i]->learnLow[n]>low[n])c.cb[i]->learnLow[n]-=1;
	}
	return i;
}

int clear_stale_entries(codeBook &c){
	int staleThresh = c.t>>1;
	int *keep = new int[c.numEntries];
	int keepCnt = 0;
	for(int i=0;i<c.numEntries;++i){
		if(c.cb[i]->stale>staleThresh){
			keep[i] = 0;
		}else{
			keep[i] = 1;
			keepCnt += 1;
		}
	}
	c.t = 0;
	code_element **foo = new code_element*[keepCnt];
	int k=0;
	for(int ii=0;ii<c.numEntries;++ii){
		if(keep[ii]){
			foo[k] = c.cb[ii];
			foo[k]->t_last_update = 0;
			k++;
		}
	}
	delete [] keep;
	delete [] c.cb;
	c.cb = foo;
	int numCleared = c.numEntries - keepCnt;
	c.numEntries = keepCnt;
	return numCleared;
}

uchar background_diff(uchar *p, codeBook &c, int numChannels, int *minMod, int *maxMod){
	int matchChannel;
	int i;
	for(i=0; i<c.numEntries; ++i){
		matchChannel = 0;
		for(int n=0;n<numChannels;++n){
			if(c.cb[i]->min[n]-minMod[n]<=*(p+n) &&
				c.cb[i]->max[n]+maxMod[n]>=*(p+n)){
				matchChannel++;
			}else{
				break;
			}
		}
		if(matchChannel == numChannels){
			break;//found an entry
		}
	}
	if(i>=c.numEntries)return 255;//foreground
	return 0;//background
}

BgSubtractor::BgSubtractor(CvSize size, int train_iter, int numComponents):
	size(size),
	train_bg_model_iter(train_iter),
	frameCount(0),
	numComponents(numComponents){
	codebooks = new codeBook*[size.height];
	for(int i=0;i<size.height;++i){
		codebooks[i] = new codeBook[size.width];
		for(int j=0;j<size.width;++j){
			codebooks[i][j].numEntries=0;
			codebooks[i][j].t=0;
		}
	}
	mask = cvCreateImage(size, IPL_DEPTH_8U, 1);
	bboxes = new CvRect[numComponents];
}

unsigned BgSubtractor::BOUNDS_DEFAULT[3] = {10, 10, 10};
int BgSubtractor::MIN_MOD_DEFAULT[3] = {20, 20, 20};
int BgSubtractor::MAX_MOD_DEFAULT[3] = {20, 20, 20};

/*
frame: image in YCrCb mode
draw: optional. draw bounding box on this image
return: false during training, true during bg subtracting.
*/
bool BgSubtractor::process(IplImage *frame, IplImage *draw){
	frameCount++;
	codebook_tick_img(frame, codebooks);
	if(frameCount<train_bg_model_iter){
		update_codebook_img(frame, codebooks, BOUNDS_DEFAULT);
	}else if(frameCount==train_bg_model_iter){
		update_codebook_img(frame, codebooks, BOUNDS_DEFAULT);
		clear_stale_entries_img(frame, codebooks);
	}else{
		background_diff_img(frame, mask, codebooks, MIN_MOD_DEFAULT, MAX_MOD_DEFAULT);
		numComponents = 4;
		find_connected_component(mask, &numComponents, bboxes);
		if(draw!=NULL)
			draw_connected_components(draw, numComponents, bboxes);
		return true;
	}
	return false;
}

bool BgSubtractor::process(cv::Mat &frame, cv::Mat &draw){
	IplImage *frameImg = new IplImage(frame);
	IplImage *drawImg = new IplImage(draw);
	bool result = process(frameImg, drawImg);
	delete frameImg;
	delete drawImg;
	return result;
}

bool BgSubtractor::process(cv::Mat &frame){
	IplImage *frameImg = new IplImage(frame);
	bool result = process(frameImg, NULL);
	delete frameImg;
	return result;
}