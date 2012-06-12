/*
 *  footDetector.cpp
 *  openFrameworks
 *
 *  Created by theo on 9/24/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "footDetector.h"

void footDetector::setup(int w, int h){
	width	= w;
	height	= h;
	
	gray1.allocate(width, height);
	gray2.allocate(width, height);
	gray3.allocate(width, height);
	gray4.allocate(width, height);
		
	float1.allocate(w, h);
	float2.allocate(w, h);

//	float1.setNativeScale(0.0, 255.0);
//	float2.setNativeScale(0.0, 255.0);	
}

void footDetector::update(ofxCvGrayscaleImage & grayIn){
	gray2 = gray1;
	gray1.scaleIntoMe(grayIn, CV_INTER_NN);
	
	gray3.absDiff(gray2, gray1);;
	//gray3 -= gray1;
	
	float2 = gray3;
	
	float1 *= 0.75;
	float1 += float2;
	//float1 *= 255.0;
	
	gray4 = float1;
}

void footDetector::draw(){

}

