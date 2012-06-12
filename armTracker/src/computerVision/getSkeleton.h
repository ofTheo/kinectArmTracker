#pragma once
#include "ofxOpenCv.h"
#include "ofImage.h"

class getSkeleton{

	public:
		
//		static void doFromImage(ofImage & inputImg, ofImage & outputImg);
//		static void doFromImage(ofxCvGrayscaleImage & input, ofImage & outputImg);
		static void doFromImage(ofxCvGrayscaleImage & input, ofxCvGrayscaleImage & output, int thresholdAmnt = 8, int thresholdAmnt2 = 1);
	
//		void beginFromScreen(ofImage * imgPtr, int w, int h);
//		void endFromScreen();

		ofImage * iPtr;
};