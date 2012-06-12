
#pragma once
#include "liveInput.h"
#include "ofControlPanel.h"

class kinectManager{


	public:
	
		void setup(ofControlPanel & panel){
			guiPtr = &panel; 
			
			numDevices = 0;
			
			ofxKinect tmpKinect;
			tmpKinect.init(true, false, false);			
			numDevices = ofxKinect::numTotalDevices();
			
			cout << " this many devices " << numDevices << endl; 
	
			if( numDevices >= 1 ){
				ofxKinectCalibration::setClippingInCentimeters(80, 180);
				inputR.setupKinect(0, LIVE_MODE_KINECT_DEPTH);
			}else{
				inputR.setupVideo("kinectTest.mov", LIVE_MODE_KINECT_DEPTH);
			}
			
		}

		void update(bool bRecordFrames1 = false, bool bRecordFrames2 = false){
			
			ofxKinectCalibration::setClippingInCentimeters( guiPtr->getValueI("clipMin"), guiPtr->getValueI("clipMax") ); 
			
			bool bFlipH = guiPtr->getValueB("flipVideoH"); 
			if( numDevices == 1  ){
				bFlipH = !bFlipH;
			}
			
			if( inputR.update(bFlipH, guiPtr->getValueB("flipVideoV"), bRecordFrames1 ) ){
				bFrameRNew = true;
			}else{
				bFrameRNew = false;			
			}

			if( numDevices == 0  ){
				bFlipH = !bFlipH;
			}				
	
		}
		
		bool isFrameNewR(){
			return bFrameRNew; 
		}
		
		int numDevices;
		bool bFrameRNew;
		liveInput inputR;
		
		ofControlPanel * guiPtr;
		

};