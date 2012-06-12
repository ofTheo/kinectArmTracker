/*
 *  footDetector.h
 *  openFrameworks
 *
 *  Created by theo on 9/24/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
 
#include "ofxOpenCv.h"
#include "ofMain.h"

class footDetector{

	public:
		
		void setup(int w, int h);
		void update(ofxCvGrayscaleImage & img);
		void draw();
		
		ofxCvGrayscaleImage gray1;
		ofxCvGrayscaleImage gray2;
		ofxCvGrayscaleImage gray3;
		ofxCvGrayscaleImage gray4;


		ofxCvFloatImage float1;
		ofxCvFloatImage float2;
		

		int width, height;
};