#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofControlPanel.h"
#include "liveInput.h"
#include "netUtils.h"
#include "trackerManager.h"
#include "ofxAssimpModelLoader.h"
#include "handTracker.h"
#include "kinectManager.h"
#include "armTracker.h"
#include "ofxOsc.h"

class testApp : public ofBaseApp{
	
	public:
		
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		
		void dragEvent(ofDragInfo dragInfo);
	
		void eventsIn(guiCallbackData & data);
		
		void updateCapture( liveInput & input, armTracker & arm, handTracker & hand, bool bFlipH);		

		float elapsedTime;
		int appFrameCount;
		float appFrameRate;
		
		ofxOscSender oscTX;
		
		ofxCvContourFinder contourFinder;
		trackerManager tracker;
	
		armTracker armTrackL;
		handTracker handL;	

		armTracker armTrackR;
		handTracker handR;	
		string ip;
		int port;
		
		kinectManager km;
				
		float speedComp;
										
		int capW, capH;
				
		ofControlPanel gui;		
};
	
