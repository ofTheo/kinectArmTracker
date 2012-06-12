#pragma once
#include "ofxOpenCv.h"
#include "trueVectorField.h"
#include "lineUtils.h"
#include "ofControlPanel.h"

extern ofControlPanel * guiPtr;

using namespace lineUtils;


class trackPoint{
	public:
	
	trackPoint(){
		b = 0.0;
		id = -1;
		lightAmnt = 0.0;
	}
	
	void cloneFromBlob(ofxCvBlob & b){
		cvBlob.pts = b.pts; 
		cvBlob.centroid = b.centroid;
		cvBlob.area = b.area;
		cvBlob.length = b.length;
		cvBlob.boundingRect = b.boundingRect;
		cvBlob.hole = b.hole;
		cvBlob.nPts = b.nPts;
	}
	
	void birth(unsigned int newId){
		id = newId;
		timeFirstSeen = ofGetElapsedTimef();
		timeSeen = 0.0;
		b = 1.0;
		bMatched = true;
	}
	
	
	void keepAlive(){
		timeSeen = ofGetElapsedTimef() - timeFirstSeen;
		bMatched = true;
		b = 1.0;		
	}
	
	ofxCvBlob cvBlob;
	
	ofPoint speed;
	float lightAmnt;
	float dist;
	bool bMatched;
	unsigned int id;
	float timeFirstSeen;
	float timeSeen;
	float b;
};

class usefulBlob{
	public:
		usefulBlob(){
			speedX = 0.0;
			speedXN = 0.0;
			perimRatio = 0.0;
		}
		
		void setScale(float scaleX, float scaleY){
			scale.set(scaleX, scaleY, 1.0);
		}
		
		void update(trackPoint & p){
			ofxCvBlob & b = p.cvBlob;
			
			id  = p.id;
			pts = b.pts;
			bounds = b.boundingRect;
			centroid = b.centroid;
			headEst  = b.centroid;
			headEst.y *= 0.3;
			headEst.y += b.boundingRect.y * 0.7;
			
			speed = p.speed * scale;
			timeSeen = p.timeSeen;
			dist = p.dist * scale.x;
			
			centroid *= scale;
			headEst  *= scale;
			
			bounds.x *= scale.x;
			bounds.y *= scale.y;
			bounds.width *= scale.x;
			bounds.height *= scale.y;
			
			for(int k = 0; k < pts.size(); k++){
				pts[k] *= scale;
			}
			
			speedX	= ofClamp(fabs(speed.x), guiPtr->getValueF("blobSpeedMin"), guiPtr->getValueF("blobSpeedMax") );  
			speedXN = ofMap(fabs(speed.x), guiPtr->getValueF("blobSpeedMin"), guiPtr->getValueF("blobSpeedMax"), 0, 1, true );  
			
			
			ofPoint vec = ofPoint( bounds.x + bounds.width * 0.5, bounds.y + bounds.height * 0.5 ) - centroid;
			
//			float angle = atan2(-vec.y, vec.x) * RAD_TO_DEG; 
//			
//			if( vec.length() > guiPtr->getValueF("armsUpLen") && ofAngleDifferenceDegrees(angle, 90) < guiPtr->getValueF("armsUpAngleWidth") 
//				&& ( bounds.width / bounds.height ) < guiPtr->getValueF("armsUpSkinnyVal") ){
//			
//				bArmsUp = true;
//			}else{
//				bArmsUp = false;			
//			}
			
			//cout << "blob speed " << speed.length() << endl; 
		}
		
		
		void smooth(float amnt){
			if( pts.size() <= 1 )return;
			
			for(int k = 1; k < pts.size(); k++){
				pts[k] = pts[k-1] * amnt + pts[k] * (1.0-amnt);
			}		
			
			lowPolyPerson.clear();
			lowPolyPerson.addVertexes(pts);
			lowPolyPerson.simplify( 4 );
			
			horizLines.clear();
			
			
			ofRectangle centroidBounds;
			centroidBounds.setFromCenter(centroid, bounds.width, bounds.height);
			
			if( lowPolyPerson.size() >= 2 ){
				float heighestPt = 1000000;
				int which = -1;
			
				for(int k = 0; k < lowPolyPerson.size(); k++){
					
					int pre = k-1;
					if( pre < 0 ){
						pre += lowPolyPerson.size();
					}
														
					ofPoint v = lowPolyPerson[k] - lowPolyPerson[pre];
					if( fabs( ofAngleDifferenceDegrees( atan2(v.y, v.x) * RAD_TO_DEG, 180.0 ) ) < 30.0 ){
						
						ofPoint midPt = ( lowPolyPerson[pre] + lowPolyPerson[k] ) / 2.0;

						float xPct    = ofMap( midPt.x, centroidBounds.x, centroidBounds.x + centroidBounds.width, 0.0, 1.0, true );
						float yPct    = ofMap( midPt.y, centroidBounds.y, centroidBounds.y + centroidBounds.height, 0.0, 1.0, true );
													
						if( xPct > 0.4 && xPct < 0.6 ){
							if( midPt.y < heighestPt ){
								heighestPt = midPt.y;
								
								headEst = midPt; 
							}
						}
																
						ofPolyline polyline;
						polyline.addVertex(lowPolyPerson[pre]);
						polyline.addVertex(lowPolyPerson[k]);
			
						horizLines.push_back(polyline);
					
					}
				}
				
			}
			
			calTopRect();
			findInterestingPoint();
			
			if( perimRatio > guiPtr->getValueF("perimRatio") &&  ( bounds.width / bounds.height ) < guiPtr->getValueF("armsUpSkinnyVal")  ){
				bArmsUp = true;
			}else{
				bArmsUp = false;
			}
		}
		
		void calTopRect(){
			if( bounds.width == 0.0  || bounds.height == 0.0 ) return; 
		
			topRect = bounds;
			topRect.height = centroid.y - topRect.y; 
			
			float maxX = topRect.x; 
			float minX = topRect.x + topRect.width;
			
			float len = 0.0;
			
			for(int i = 0; i < lowPolyPerson.size(); i++){
				if( lowPolyPerson[i].y < topRect.y + topRect.height ){
					
					int pre = i - 1;
					if( pre < 0 ) pre += lowPolyPerson.size(); 
					
					len += ( lowPolyPerson[i] - lowPolyPerson[pre] ).length(); 
					
					if( lowPolyPerson[i].x < minX ){
						minX = lowPolyPerson[i].x;
					}
					if( lowPolyPerson[i].x > maxX ){
						maxX = lowPolyPerson[i].x;
					}				
				}
			}
					
			if( topRect.x < minX ){
				topRect.width -= fabs( topRect.x  - minX);
				topRect.x = minX; 
			}
			if( topRect.x + topRect.width > maxX ){
				topRect.width -= fabs( (topRect.x + topRect.width)  - maxX );
			}
			
			perimRatio *= 0.75;
			perimRatio += 0.25 * ( len / ( topRect.width*2.0 + topRect.height*2.0 ) );			
			
		}		
		
		void findInterestingPoint(){
			
			
			
			ofPolyline lStretched; 
			ofPolyline l; 

			float minX = 1000000;
			float maxX = 0.0; 
			int minI = -1;
			int maxI = -1;
			
			float heightAdd = guiPtr->getValueF("blobFurthestYFactor");
			
			for(int k = 0; k < lowPolyPerson.size(); k++){
								
				//TODO: make this cutofff a slider
				if( lowPolyPerson[k].y < centroid.y * guiPtr->getValueF("blobHandYCuttoff") ){
					
					float vd = fabs( lowPolyPerson[k].y < centroid.y );
					
					float dMax = lowPolyPerson[k].x + vd * heightAdd;
					float dMin = lowPolyPerson[k].x - vd * heightAdd;
				
					if( dMax > maxX ){
							maxX = dMax;
							maxI = k;
					}
					if( dMin < minX ){
							minX = dMin;
							minI = k;
					}					
				}
			}
			
			//TODO: hookup the ratio to slider
			float scaleVal =  guiPtr->getValueF("blobFurthestThresh");
			if( bFurthestSet ) scaleVal *= 1.25;
			
			
			if( minI != -1 && fabs( minX - centroid.x ) > bounds.height * scaleVal ){
				l.addVertex( lowPolyPerson[minI] ); 
			}
			if( maxI != -1 && fabs( maxX - centroid.x ) > bounds.height * scaleVal ){
				l.addVertex( lowPolyPerson[maxI] ); 
			}
			
			furthestPts.clear();
			
			if( l.size() ){
			
				furthestPtPre = furthestPt;
				
				if( bFurthestSet == false ){
				
//					cout << " finding a new pt for the first time " << endl; 
					
					int furthest = 0;
					if( l.size() == 2 && fabs( maxX - centroid.x ) > fabs( maxI - centroid.x ) ){
						furthest = 1;
					}
					
					furthestPt = l[furthest];
					bFurthestSet = true;
						
					furthestPts.push_back( furthestPt );
					if( l.size() >= 2 ){
						furthestPts.push_back( l[ (furthest + 1) % 2 ] );
					}
					
					furthestPtPre = furthestPt;
					
				}else{
					
					if( l.size() >= 2 ){
						
						if( ( l[0] - furthestPt ).length() < ( l[1] - furthestPt ).length() ){
							furthestPt = l[0];
						}else{
							furthestPt = l[1];						
						}
						
					}else{				
						furthestPt = l[0];
					}
					
					bFurthestSet = true;
					
					furthestPts.push_back( furthestPt );
					if( l.size() >= 2 ){
						furthestPts.push_back( l[ (minI + 1) % 2 ] );
					}					
										
				}
			
			}else{
				bFurthestSet = false;
				furthestPts.clear();
			}
			
//			cout << " bFurthestSet = " << bFurthestSet << " pre is " << furthestPtPre << " cur is " << furthestPt << " vel is " << furthestVel << endl;
			
			furthestVel *= 0.6;
			furthestVel += (furthestPt-furthestPtPre) * 0.4; 
			
		}
		
		void draw(){
		
			ofPushStyle();
				ofSetColor(200, 100, 0);
				ofNoFill();
//				ofBeginShape(); 
//					ofVertexes(pts);
//				ofEndShape(false);

				ofSetColor(100, 50, 0);
				lowPolyPerson.draw();
								
				ofBeginShape(); 
					ofVertexes(pts);
				ofEndShape(false);
				
				ofPushMatrix();
					ofTranslate(0, -30, 0);
					for(int k = 0; k < horizLines.size(); k++){
						horizLines[k].draw();
					}
				ofPopMatrix();
				
				ofCircle(centroid, 10.0);
				ofCircle(headEst, 6.0);
				
				ofLine( bounds.x + bounds.width * 0.5,  bounds.y + bounds.height * 0.5, 
						centroid.x,  centroid.y ); 
				
				ofCircle(furthestPt, 20.0);
				
				for(int i = 0; i < furthestPts.size(); i++){
					ofCircle(furthestPts[i], 9.0);
				}
				
				ofRect(topRect);
				
				if( bArmsUp ){
					ofTriangle(headEst + ofPoint(-20, 0), headEst + ofPoint(20, 0), headEst + ofPoint(0, -35) );
				}
				
				ofDrawBitmapString("id is " + ofToString(id), headEst + ofPoint(0, -60));
				ofDrawBitmapString("speed x is " + ofToString(speed.x, 2), headEst + ofPoint(0, -40));
				ofDrawBitmapString("speed 0/1 is: " + ofToString((int)speedXN), headEst + ofPoint(0, -20));
				ofDrawBitmapString("perimRatio is: " + ofToString(perimRatio, 2), headEst + ofPoint(0, -80));
				ofDrawBitmapString("vel is: " + ofToString(furthestVel, 2), headEst + ofPoint(0, -100));
				
			ofPopStyle();
		}
		
		float speedX;
		float speedXN;
		
		bool bArmsUp;
		
		ofPoint scale;
		ofPoint centroid;
		ofPoint headEst;
		ofRectangle bounds;
		vector <ofPoint> pts; 
		vector <ofPolyline> horizLines;
		
		float perimRatio;
		
		ofPoint furthestPtPre;
		ofPoint furthestPt; 
	
		vector <ofPoint> furthestPts;
	
		bool bFurthestSet;
		ofPoint furthestVel;
		
		ofPolyline lowPolyPerson;
		ofRectangle topRect; 

		int id; 
	
		ofPoint speed;
		float dist;
		float timeSeen;
};


typedef enum{
	TRACKER_NONE_LONG = 0,
	TRACKER_NONE_SHORT,
	TRACKER_LOW,
	TRACKER_MED,
	TRACKER_HIGH
}trackerState;

class trackerManager{
	public:
	
		void setup(float w, float h);
		void update( ofxCvContourFinder & finder, float minDist, float maxDist, float normalScale, float repelAmnt, float shiftFieldY, float leftComp, float rightComp, float topComp, float bottomComp );
		bool checkHit( ofPoint & a, float radSize );		
		ofPoint getForceForPoint( ofPoint & pos );
		
		void debugDraw();

		int curNumPts;
		float t1;
				
		float lightSize;
		float averageYPosition; 
		
		trackerState state;
		float statePct;
		trueVectorField field;
		
		float timeSincePts;
		int curId;

		float screenW, screenH;
		vector <trackPoint> points;
		vector <usefulBlob> usefulBlobs;
};