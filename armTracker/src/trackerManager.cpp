#include "trackerManager.h"

void trackerManager::setup(float w, float h){

	points.clear();
	usefulBlobs.clear();
	
	screenW = w;
	screenH = h;
					
	timeSincePts = 0.0;
	t1 = -1.0;
	curId = 0;
	statePct = 0.0;
	
	lightSize = 200.0f;
	
	field.setup(96, 42, screenW, screenH);
	
	state = TRACKER_NONE_LONG;
}	

//leftComp = 100
//rightComp = -100
//topComp = screenH * 0.24
//bottomComp = screenH + 200

void trackerManager::update( ofxCvContourFinder & finder, float minDist, float maxDist, float normalScale, float repelAmnt, float shiftFieldY, float leftComp, float rightComp, float topComp, float bottomComp ){
		
	for(int k = 0; k < points.size(); k++){
		points[k].bMatched = false;
	}
	
	float idMatchDist = 120.0f;	
	
	vector <trackPoint> tmpPts;
	
	for(int j = 0; j < finder.blobs.size(); j++){
	
		//cout << "for J " << j << " :" <<endl; 
		bool bFound = false;
		        
		ofPoint newPt = finder.blobs[j].centroid;
		
		float smallestDist = 1000000.0; 
		int whichToMatch = -1;
		for(int k = 0; k < points.size(); k++){
			if( points[k].bMatched ) continue;

			float dist = (points[k].cvBlob.centroid - newPt).length();
			if( dist < smallestDist && dist < idMatchDist ){
				smallestDist = dist;
				whichToMatch = k;
				bFound = true;
			}
		}
		
		if( bFound ){
			int k = whichToMatch;
			
			points[k].speed *= 0.7;
			points[k].speed += (points[k].cvBlob.centroid - newPt) * 0.3;			
			points[k].dist = (points[k].cvBlob.centroid - newPt).length();
		
			points[k].keepAlive();							
			points[k].cloneFromBlob(finder.blobs[j]);
						
			tmpPts.push_back(points[k]);
			
			//cout << " GOOD MATCH " << endl <<endl;			
		}else{
			trackPoint tmp;
			tmp.birth(curId);				
			
			tmp.cloneFromBlob(finder.blobs[j]);
			
			tmp.dist  = maxDist * 2.0;
			tmp.speed.set(0, 0, 0);
			tmpPts.push_back(tmp);
			
			//cout << " NO MATCH - ADDING FROM TMP " << curId << endl << endl;

			curId++;				
		}
		
	}	
	
//	cout << "finder.blobs.size() is " << finder.blobs.size() << " points.size() is " << points.size() << endl; 
	
	points.clear();
	points = tmpPts;	
		
	vector <usefulBlob> nexBlobs;
					
	for(int k = 0; k < tmpPts.size(); k++){
		
		bool bFound = false;
		for(int i = 0; i < usefulBlobs.size(); i++){
			if( usefulBlobs[i].id == points[k].id ){				
				nexBlobs.push_back( usefulBlobs[i] );
				bFound = true;
				break; 
			}
		}
		if( !bFound ){			
			nexBlobs.push_back( usefulBlob() );
		}
		
		nexBlobs.back().setScale( screenW/finder.getWidth(), screenH/finder.getHeight());
		nexBlobs.back().update(points[k]);
		nexBlobs.back().smooth(0.6);		
	}		
	
	usefulBlobs.clear();
	usefulBlobs = nexBlobs;
	
	field.fadeField(0.93);
	
	for(int k = 0; k <finder.blobs.size(); k++){
		
		ofPolyline pts = finder.blobs[k].pts;
		if( pts.size() > 10 ){
			
			for(int k = 1; k < pts.size(); k++){
				pts[k] = pts[k-1] * 0.78 + pts[k] * 0.22;
			}
			
			ofPoint pre, nex, normal;
			
			for(int j = 0; j < pts.size(); j++){
				if( j == 0 ){
					pre = pts[j];
					nex = pts[j+1];								
				}else if( j == pts.size()-1 ){
					pre = pts[j-1];
					nex = pts[j];				
				}else{
					pre = pts[j-1];
					nex = pts[j+1];
				}
			
				normal = nex - pre;
				normal.set(-normal.y, normal.x);
				
				ofPoint newPt;
				newPt.x = ofMap( pts[j].x , 0, finder.getWidth(),  leftComp, screenW + rightComp);
				newPt.y = ofMap( pts[j].y , 0, finder.getHeight(), topComp, screenH + bottomComp );
				
				newPt += normal * normalScale;

				float xpct = ofMap( newPt.x, 0, screenW, 0.01, 0.997, true );
				float ypct = ofMap( newPt.y + shiftFieldY, 0, screenH, 0.01, 0.997, true );
				
				field.setIntoField( ofPoint(xpct, ypct), normal * 1.0f * repelAmnt );
			
			}
			
		}
		
	}
	
	field.blurField(0.4);
	
	//TODO check this
	if( points.size() == 0 && curId > 10000000 ){
		curId = 0;
	}					
	
	if( points.size() == 0 ){
		statePct *= 0.7;
		statePct += 0.3 * 1.0;
		
	}else{				
		float totalDist = 0.0;
		for(int k = 0; k < points.size(); k++){
			totalDist += points[k].dist;
		}
		totalDist /= points.size();
		
		float val = ofMap(totalDist, minDist, maxDist, 1, 3.99, true);
		statePct *= 0.7;
		statePct += val * 0.3;
		
		//cout << "val is " << val << " statePct is " << statePct << endl;
	}
	
	state = (trackerState)((int)statePct);
	
	if( state > TRACKER_NONE_SHORT ){
		timeSincePts = 0.0;
	}else{			
		if( t1 == -1.0 ){
			t1 = ofGetElapsedTimef();
		}
		timeSincePts += ofGetElapsedTimef() - t1;
		if( timeSincePts > 2.0 ){
			state = (trackerState)TRACKER_NONE_LONG;
		}
	}
				
	t1 = ofGetElapsedTimef();
	
}

ofPoint trackerManager::getForceForPoint( ofPoint & pos ){
	return field.getVelFromPos(ofClamp(pos.x / screenW, 0, 1), ofClamp(pos.y / screenW, 0, 1));
}


bool trackerManager::checkHit( ofPoint & a, float radSize ){
	radSize *= lightSize;
	
	for(int k = 0; k < usefulBlobs.size(); k++){
		float rad		= radSize;
		float distSq	= rad*rad;
	
		if( (usefulBlobs[k].centroid - a).lengthSquared() <= distSq ){
			return true;
		}
	}
	
	return false;			
}


void trackerManager::debugDraw(){
	ofPushStyle();
	ofSetColor(255);
	ofNoFill();
	
	for(int j = 0; j < usefulBlobs.size(); j++){
//		ofDrawBitmapString("id is " + ofToString(points[j].id), points[j].x, points[j].y);
//		ofDrawBitmapString("dist is " + ofToString(points[j].dist), points[j].x, points[j].y+20);
//		ofDrawBitmapString("speed is " + ofToString(points[j].speed), points[j].x, points[j].y+40);
//		ofCircle(points[j].centroid.x, points[j].centroid.y, 100.0f * points[j].b );

		usefulBlobs[j].draw();
	}
	
	ofSetColor(255, 0, 255);
	field.draw(0, 0, screenW, screenH, 10);
	
	ofPopStyle();
}

		