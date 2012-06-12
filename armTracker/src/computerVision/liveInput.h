#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "videoWarper.h"


typedef enum{
	LIVE_TYPE_CAM, 
	LIVE_TYPE_VIDEO, 
	LIVE_TYPE_KINECT, 
} liveInputType;

typedef enum{
	LIVE_MODE_DIFF, 
	LIVE_MODE_BG_SUB,
	LIVE_MODE_KINECT,
	LIVE_MODE_SMART_THRESH,	
	LIVE_MODE_KINECT_DEPTH
} liveInputMode;

typedef enum{
	BG_MODE_ABS,
	BG_MODE_LIGHTER,
	BG_MODE_DARKER
}backgroundSubMode;

class liveInput{

	public:
		liveInput();
		void setupKinect(int device, liveInputMode inputMode);
		void setupDC1394( int width, int height, liveInputMode inputMode );
		void setupRegularCamera( int width, int height, liveInputMode inputMode, int deviceID = 0 );
		void setupVideo(string videoPath, liveInputMode inputMode );
		void setupFake(int width, int height, int numPeople, liveInputMode inputMode );
		
		bool update(bool bFlipHoriz, bool bFlipVert, bool bRecord = false);		
		
		void setupInputWarp(string xmlName, bool bColor, bool autoSave = false);
		
		void grabBackground(){
			if( grayscalePre.getWidth() > 0 && inputType == LIVE_TYPE_KINECT ){
				bgFloat = preview;
			}else{
				if( grayscalePre.getWidth() > 0 ){
					grayscalePre = grayscaleCur;
				}
			}
		}
		
		ofxCvGrayscaleImage & getMainImage(bool bFlipH = false){
			if( !bFlipH )return preview;
			else{
				if( previewFlip.getWidth() == 0.0 ){
					previewFlip.allocate( preview.getWidth(), preview.getHeight() );
				}
				previewFlip = preview;
				previewFlip.mirror(false, true);
				return previewFlip;
			}
		}
		
		ofxCvGrayscaleImage & getSecondImage(bool bFlipH = false){
			if( !bFlipH )return extra1;
			else{
				if( extra2Flip.getWidth() == 0.0 ){
					extra2Flip.allocate( extra1.getWidth(), extra1.getHeight() );
				}
				extra2Flip = extra1;
				extra2Flip.mirror(false, true);	
				return extra2Flip;			
			}
		}
		
		void updateKinectPosition( float pct, float _timeToWaitTillNext ){
			if( ofGetElapsedTimef() > timeToWaitTillNext ){
								
				kinect.setCameraTiltAngle(ofMap(pct, 0, 1, -30, 30, true));
				
				timeToWaitTillNext = ofGetElapsedTimef() + _timeToWaitTillNext;
			}
		}

		void shiftKinect( float amnt, float _timeToWaitTillNext ){
			if( ofGetElapsedTimef() > timeToWaitTillNext ){
				
				kinect.setCameraTiltAngle( getKinectAngleNorm() + ofMap(amnt, 0, 1, -30, 30, true) );
				
				timeToWaitTillNext = ofGetElapsedTimef() + _timeToWaitTillNext;
			}
		}
		
		float getKinectAngleNorm(){
			return ofMap(kinect.getTargetCameraTiltAngle(), -30, 30, 0, 1, true); 
		}
		
		int w, h;

		ofxCvGrayscaleImage grayscale;
		ofxCvGrayscaleImage preview;
		ofxCvGrayscaleImage inter;
		ofxCvFloatImage bgFloat;
		ofxCvFloatImage liveFloat;
		
		float timeToWaitTillNext;
		
		ofxCvColorImage		color;
		liveInputType		inputType;
		liveInputMode		inputMode;
		
		float nearThreshold;
		float farThreshold;
		float threshold;
				
		int minMaskY, maxMaskY;
		int minMaskX, maxMaskX;
		
		backgroundSubMode bgMode;
		ofxCvGrayscaleImage extra1;		
		ofxCvGrayscaleImage extra2;		
		
		ofxCvGrayscaleImage previewFlip;
		ofxCvGrayscaleImage extra2Flip;		
		
		string sessionStr;
		ofImage recImg;
		
		vector <ofPoint> fakes;

		ofVideoGrabber camera;

	//protected:
		ofxKinect kinect;

		ofVideoPlayer videoPlayer;
			
		void setupMode();
		
		bool doWarp;
		videoWarper warper;
		
		ofxCvGrayscaleImage grayscalePre;		
		ofxCvGrayscaleImage grayscaleCur;	
		
		bool bSmooth;
		
		float * floatPixels;


};