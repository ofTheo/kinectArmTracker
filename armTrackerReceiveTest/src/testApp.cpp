#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetWindowTitle("arm tracker - receive test");

	// listen on the given port
	cout << "listening for osc messages on port " << PORT << "\n";
	receiver.setup(PORT);
	bGotData = false;
	
	ofBackground(30, 30, 30);

	shoulderAngle = 180;
	elbowAngle = 180;
	wristAngle = 180;
	thumbAngle = 180;
}

//--------------------------------------------------------------
void testApp::update(){

	//check for waiting messages
	while(receiver.hasWaitingMessages()){
		//get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);

		// we add 180 to the arm to draw it in the same way as the tracker.
		// remove the 180 if you want to draw it the other way
		if(m.getAddress() == "armTracker"){
			shoulderAngle	= m.getArgAsFloat(0) + 180.0;
			elbowAngle		= m.getArgAsFloat(1) + 180.0;			
			wristAngle		= m.getArgAsFloat(2) + 180.0;
			thumbAngle		= -m.getArgAsFloat(3) + wristAngle;
			bGotData = true;
		}

	}
}


//--------------------------------------------------------------
void testApp::draw(){

	string buf;
	if(!bGotData){
		buf = "waiting for osc messages on port" + ofToString(PORT);
		ofSetColor(255);
		ofDrawBitmapString(buf, 10, 20);
		
	}else{
		buf = "data coming in on port " + ofToString(PORT);
		buf += "\nshoulderAngle = " + ofToString(shoulderAngle, 0);
		buf += "\nelbowAngle = " + ofToString(elbowAngle, 0);
		buf += "\nwristAngle = " + ofToString(wristAngle, 0);
		buf += "\nthumbAngle = " + ofToString(thumbAngle, 0);
	
		float rad = 120;
		
		ofSetColor(255);
		ofDrawBitmapString(buf, 10, 20);

		ofPoint p(ofGetWidth()-200, ofGetHeight()/2);
		ofPoint np = p + ofPoint(cos(shoulderAngle*DEG_TO_RAD) * rad, sin(shoulderAngle*DEG_TO_RAD) * rad);	
		
		ofCircle(p, 4);
		ofLine(p, np);
		
		rad *= 0.8;
		
		p = np;
		np = p + ofPoint(cos(elbowAngle*DEG_TO_RAD) * rad, sin(elbowAngle*DEG_TO_RAD) * rad);	

		ofLine(p, np);

		rad *= 0.3;

		p = np;
		np = p + ofPoint(cos(wristAngle*DEG_TO_RAD) * rad, sin(wristAngle*DEG_TO_RAD) * rad);	

		ofLine(p, np);

		rad *= 0.6;
		np = p + ofPoint(cos(thumbAngle*DEG_TO_RAD) * rad, sin(thumbAngle*DEG_TO_RAD) * rad);	

		ofLine(p, np);
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

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

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
