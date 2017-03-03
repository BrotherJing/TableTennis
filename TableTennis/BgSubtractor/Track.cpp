#include "Main.h"

Tracker::Tracker(CvRect c){
	bbox = c;
	context = cvRect(c.x-c.width*2, c.y-c.height*2, (int)(c.width*5), (int)(c.height*5));
	center = cvPoint(c.x+c.width/2, c.y+c.height/2);
	last = center;
}

void Tracker::set(CvRect c){
	bbox = c;
	context = cvRect(c.x-c.width, c.y-c.height, (int)(c.width*3), (int)(c.height*3));
	center = cvPoint(c.x+c.width/2, c.y+c.height/2);
	last = center;
}

bool trackBall(Tracker *tracker, CvRect *bbs, int cnt){

	float dist_min = numeric_limits<float>::max();
	int best_choice=-1;
	int contain_tracker = -1;
	for(int i=0;i<cnt;++i){
		if(bbs[i].x+bbs[i].width/2>tracker->context.x&&bbs[i].x+bbs[i].width/2<tracker->context.x+tracker->context.width&&
			bbs[i].y+bbs[i].height/2>tracker->context.y&&bbs[i].y+bbs[i].height/2<tracker->context.y+tracker->context.height){
			float dist=DIS(tracker->center, cvPoint(bbs[i].x+bbs[i].width/2, bbs[i].y+bbs[i].height/2));
			if(dist<dist_min){
				dist_min = dist;
				best_choice=i;
			}
		}
	}
	if(best_choice!=-1){// candidate found
		int x=bbs[best_choice].x, y=bbs[best_choice].y;
		int w=(int)(tracker->bbox.width*LOWPASS_FILTER_RATE+bbs[best_choice].width*(1.0 - LOWPASS_FILTER_RATE));
		int h=(int)(tracker->bbox.height*LOWPASS_FILTER_RATE+bbs[best_choice].height*(1.0 - LOWPASS_FILTER_RATE));
		tracker->context = cvRect(x-w/2, y-h/2, (int)(w*2), (int)(h*2));
		tracker->bbox = cvRect(x, y, w, h);
		tracker->last = tracker->center;
		tracker->center = cvPoint(bbs[best_choice].x+bbs[best_choice].width/2, bbs[best_choice].y+bbs[best_choice].height/2);
		return true;
	}
	return false;
}