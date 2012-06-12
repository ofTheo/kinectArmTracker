#include "testApp.h"

ofControlPanel * guiPtr;
trackerManager * trackManPtr;

bool bDebug = false;

float screenW = 1300;
float screenH = 800;

bool bFrameNew = false;
bool hide = false;

bool bFirstSetup = false;
bool bNeedsSetup = true;
bool bRecordFrames = false;
bool oscSetup = false;
     
//--------------------------------------------------------------
void testApp::setup(){	

	ofBackground(0,0,0);	
	ofSetCircleResolution(180);
	ofSetWindowTitle("OSC Arm Tracker - by Design I/O");
	
	ofSeedRandom();
	ofSetFrameRate(30);	
	ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_ERROR);
	
	ip		= getBroadcastIp();
	port	= 9555;

	//by default we use the broadcast ip - if you want to overide the ip uncomment the xml tag in oscIP.xml 
	ofxXmlSettings xml;
	if( xml.loadFile("oscIP.xml") ){
		ip = xml.getValue("ip", ip);
		port = xml.getValue("port", port);		
	}


	//some dummy vars we will update to show the variable lister object
	elapsedTime		= ofGetElapsedTimef();
	appFrameCount	= ofGetFrameNum();	
	appFrameRate	= ofGetFrameRate();
		
	guiPtr = &gui;
	km.setup(gui);
	
	tracker.setup(screenW, screenH);

	gui.loadFont("MONACO.TTF", 7);		
	gui.setup("kinect", 0, 0, ofGetWidth(), ofGetHeight(), true, false);
	gui.addChartPlotter("some chart", guiStatVarPointer("app fps", &appFrameRate, GUI_VAR_FLOAT, true, 2), 200, 50, 200, 5, 80);
	gui.loadSettings("controlPanelSettings.xml");
			
	gui.setWhichPanel(0);
		
	gui.setWhichColumn(0);
	gui.addToggle("send osc", "sendOSC", false);

	gui.addToggle("flip horiz", "flipVideoH", false);
	gui.addToggle("flip vert", "flipVideoV", false);
			
	gui.addSlider("clipMin (cm)", "clipMin", 80, 0, 400, true);
	gui.addSlider("clipMax (cm)", "clipMax", 180, 0, 400, true);

	gui.addToggle("filter 255 pixels", "bFilterWhitePixels", false);

	gui.addDrawableRect("preview", &km.inputR.getMainImage(false),  200, 150);
	gui.addDrawableRect("body thresh", &km.inputR.getSecondImage(false),  200, 150);
		
	gui.addSlider("arm smoothAmnt", "smoothAmnt", 11, 3, 20, true);
	gui.addSlider("arm smoothThresh", "smoothThresh", 70, 20, 200, true);
	gui.addSlider("minArmLength", "minArmLength", 100.0, 20, 400.0, false);
	gui.addSlider("maxPercentInPoly", "maxPercentInPoly", 0.6, 0.0, 1.0, false);
	gui.addSlider("peakSkipForArm", "peakSkipAllowedForArm", 4, 0, 10, true);

	gui.addSlider("blur_bounds_thresh", "blur_bounds_thresh", 160, 0, 255, true);
	gui.addSlider("minBlobSizePct", "minBlobSizePct", 1.0, 0.1, 4.0, false);
	gui.addSlider("maxBlobSizePct", "maxBlobSizePct", 1.0, 0.1, 4.0, false);

	gui.setWhichColumn(1);

	gui.addDrawableRect("arm track", &armTrackL.thresh, 200, 150);
	gui.addDrawableRect("hand", &handL.handThresh,  200, 150);
	gui.addDrawableRect("skel", &handL,  200, 150);

	gui.addSlider("skelThresh", "skelThresh", 7, 3, 15, true);
	gui.addSlider("skelThresh2", "skelThresh2", 1, 1, 7, true);
	gui.addSlider("skelHandBlur", "skelHandBlur", 5, 1, 11, true);
	gui.addSlider("handPostBlurThresh", "handPostBlurThresh", 170, 0, 255, true);
	gui.addSlider("handContourSmooth", "handContourSmooth", 7, 1, 200, true);

	gui.setWhichColumn(3);

	gui.addDrawableRect("arm track", &armTrackL, 640, 480);

	gui.setWhichColumn(2);

	gui.addSlider("LegRemoveYStartPct", "LegRemoveYStartPct", 0.7, 0.0, 1.0, false);
	gui.addSlider("LegRemoveYEndPct", "LegRemoveYEndPct", 0.9, 0.0, 1.0, false);
	gui.addSlider("LegRemoveYReducePct", "LegRemoveYReducePct", 0.8, 0.0, 1.0, false);

	gui.addSlider("numSegments", "numSegments", 5, 2, 12, true);

	gui.addSlider("backHandCuttof", "backHandCuttof", -7, -30, 30, true);
	gui.addSlider("thumbMaxY", "thumbMaxY", 1.2, 0.7, 3.0, false);
	gui.addSlider("thumbAngleThresh", "thumbAngleThresh", 100.0, 50, 140.0, false);
	gui.addSlider("thumbAngleThresh2", "thumbAngleThresh2", 3.0, 0.1, 20.0, false);
	gui.addSlider("thumbAngleThresh2Inner", "thumbAngleThresh2Inner", 3.0, 0.1, 20.0, false);
	gui.addSlider("handNumPts", "handNumPts", 15, 3, 30, true);
	gui.addSlider("peakSkipForHand", "peakSkipAllowedForHand", 4, 0, 10, true);

	gui.addSlider("avgThumbLen", "avgThumbLen", 0.2, 0.01, 1.0, false);

	gui.addSlider("elbowSmooth", "elbowSmooth", 0.4, 0.0, 1.0, false);
	gui.addSlider("shoulderSmooth", "shoulderSmooth", 0.4, 0.0, 1.0, false);
	gui.addSlider("wristSmooth", "wristSmooth", 0.4, 0.0, 1.0, false);		

	gui.addSlider("kinectTopPt", "kinectTopPt", 0.15, 0.0, 1.0, false);
	gui.addSlider("kinectBotPt", "kinectBotPt", 0.45, 0.0, 1.0, false);
	gui.addSlider("kinectBotMax", "kinectBotMax", 0.55, 0.0, 1.0, false);		

	//load from xml!
	gui.loadSettings("controlPanelSettings.xml");
	
	if( gui.getValueB("sendOSC") ){
		cout << "setting up osc on: " << ip << " port " << port << endl; 
		oscTX.setup(ip, port);
		oscSetup = true;
	}
			
	trackManPtr = &tracker;
			
}

//------------------------------------------------------------------------------------------------------------------
void testApp::updateCapture( liveInput & input, armTracker & arm, handTracker & hand, bool bFlip){

	arm.armM.setElbowSmoothing(gui.getValueF("elbowSmooth"));
	arm.armM.setShoulderSmoothing(gui.getValueF("shoulderSmooth"));
	arm.armM.setWristSmoothing(gui.getValueF("wristSmooth"));

	arm.update(input.getMainImage(bFlip), input.getSecondImage(bFlip), gui.getValueI("numSegments"), 
				gui.getValueI("smoothAmnt"), gui.getValueI("smoothThresh"), gui.getValueI("minArmLength"),
				gui.getValueF("maxPercentInPoly") );

	if( arm.armFound() ){
		hand.updateAlt2(input.getMainImage(bFlip), arm.armM, arm.getHandCenter(), arm.getHandSearchWidth(), gui.getValueI("backHandCuttof"),  gui.getValueI("thumbAngleThresh2"),  gui.getValueI("thumbAngleThresh2Inner"), gui.getValueI("handNumPts"), gui.getValueF("avgThumbLen") );

		if( hand.bFrontFacing ){
			
			arm.armM.setFrontFacing(0.0);
			
			if( hand.bFrontFacingThumb ){
				arm.armM.setThumbInfo( hand.bFrontFacingThumb, 60.0 );
			}else{
				arm.armM.setThumbInfo( hand.getThumbFound(), hand.thumbAngle );				
			}
		}else{
			arm.armM.setThumbInfo( hand.getThumbFound(), hand.thumbAngle );
		}
			
		//do stuff here
		if( gui.getValueB("sendOSC") ){
			ofxOscMessage m;
			m.setAddress("armTracker");
			m.addFloatArg(arm.armM.getCurrentShoulderAngle());
			m.addFloatArg(arm.armM.getCurrentElbowAngle());
			m.addFloatArg(arm.armM.getCurrentWristAngle());
			m.addFloatArg(arm.armM.getThumbAngle());
			oscTX.sendMessage(m);
		}
	}
	
	if( arm.timeNoPersonSeen() >30.0 ){
		input.updateKinectPosition(0.7, 7.0);
		arm.resetTimeNoPersonSeen();
	}
	
	if( arm.timeNoArmSeen() < 5.0 ){ 
	
		float val = input.getKinectAngleNorm();
		
		if( arm.averageYPosition < gui.getValueF("kinectTopPt") ){
			val += 0.1;
			input.updateKinectPosition(val, 3.0);
		}

		if( arm.averageYPosition > gui.getValueF("kinectBotPt") && val > gui.getValueF("kinectBotMax") ){
			val -= 0.1;
			input.updateKinectPosition(val, 3.0);
		}
		
	}
	
}

//--------------------------------------------------------------
void testApp::update(){
	elapsedTime		= ofGetElapsedTimef();
	appFrameCount	= ofGetFrameNum();	
	appFrameRate	= ofGetFrameRate();
	gui.update();
	
	if( gui.getValueB("sendOSC") && !oscSetup ){
		cout << "setting up osc on: " << ip << " port " << port << endl; 
		oscTX.setup(ip, port);
		oscSetup = true;
	}	
	
	km.update(bRecordFrames, false);
		
	//device 0
	if( km.isFrameNewR() ){
		updateCapture( km.inputR, armTrackL, handL, false);
	}

}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(255, 255, 255);
	if( !hide )gui.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
		
	if( key == 'f' ){
		ofToggleFullscreen();
	}
	
	if( key == 'd' ){
		bDebug = !bDebug;
	}
	
	if( key == 'h' ){
		hide = !hide;
	}
	
	if( key == OF_KEY_LEFT ){
		int newPanel = gui.getSelectedPanel()-1;
		if( newPanel >= 0 ){
			gui.setSelectedPanel(newPanel);
		}
	}
	
	if( key == OF_KEY_RIGHT ){
		int newPanel = gui.getSelectedPanel()+1;
		if( newPanel < gui.getNumPanels() ){
			gui.setSelectedPanel(newPanel);
		}
	}	
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

