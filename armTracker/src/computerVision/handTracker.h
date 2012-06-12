
#pragma once
#include "ofxOpenCv.h"
#include "cvUtils.h"
#include "getSkeleton.h"
#include "ofControlPanel.h"
#include "lineUtils.h"
#include "armModel.h"

extern ofControlPanel * guiPtr;

using namespace cvUtils;

class handTracker : public ofBaseDraws{
		
		public:
		handTracker(){
			thumbAngle = 0.0;
			thumbAngleRaw = 0.0;
			frontFacePct = 0.0;
		}
		
		void update( ofxCvGrayscaleImage & input, ofPoint curHandPoint, float handSize, int backHandCuttof ){
			if( handSize <= 8.0 )return;
			
			handSize = (int)handSize;
			
			if( handThresh.getWidth() == 0.0  ){
				if( input.getWidth() > 0 ){
					handThresh.allocate(input.getWidth()/2, input.getHeight()/2);
					skel.allocate(input.getWidth()/2, input.getHeight()/2);
				}else{
					return;
				}
			}
			
			if( small.getWidth() == 0 || small.getWidth() != handSize ){
				small.clear(); 
				small.allocate(handSize, handSize);
			}
			
			
			ofRectangle rect;
			rect.setFromCenter((int)curHandPoint.x, (int)curHandPoint.y, handSize, handSize);
			input.setROI(rect);
			
			small.scaleIntoMe(input, CV_INTER_LINEAR);
			input.resetROI();
						
			float handVal = getMaxPixelColor(input, curHandPoint, handSize/4);
			float avgVal = getAveragePixelColor(input, curHandPoint, handSize/4);
			
			float median = avgVal; 
			
			small.threshold(median+backHandCuttof, false);
			small.blur(guiPtr->getValueI("skelHandBlur"));

			handThresh.scaleIntoMe(small, CV_INTER_LINEAR);
			handThresh.threshold(170, false);
			
			contourF2.findContours(handThresh, 0, 10000000, 3, false, false);
			if( contourF2.blobs.size() ){
				handThresh.drawBlobIntoMe(contourF2.blobs[0], 255);
			}
			
			if( bFrontFacing ){
				contourF2.findContours(skel, 30, getWidth() * getHeight() * 0.4, 10, false, true);
			}
			
			skel = handThresh;
			
			getSkeleton::doFromImage(skel, skel, guiPtr->getValueI("skelThresh"), guiPtr->getValueI("skelThresh2"));
			
			line.clear();
			simp.clear();
			angles.clear();
			bThumbFound = false;
			
			if( contourF.findContours(skel, 10, 20000000, 3, false, false) ){
				ofxCvBlob b = contourF.blobs[0];
				
				line = contourF.blobs[0].pts;
				line.setClosed(true);
				line = line.getSmoothed(5, 0.5);
				
				simp = line;
				simp.simplify( ofClamp((float)handThresh.getWidth()/20.0, 5, 200) );
				
				float avgY = 0.0;
				float maxY = 0.0;
				int which = 0; 
				for(int k = 0; k < simp.size(); k++){
					avgY += simp[k].y;
					
					if( simp[k].x  > ( b.boundingRect.x * 1.05 ) && simp[k].x > skel.getWidth() * 0.2 && simp[k].x < skel.getWidth() * 0.85 && simp[k].y > maxY){
						thumbPt = simp[k];
						maxY = simp[k].y;
						which = k;
					}
				}
				
				//cout << " SIZE IS " << simp.size() << endl; 
				//cout << " ratio is " << b.boundingRect.height/b.boundingRect.width << endl; 
				if( simp.size() ){
					avgY /= simp.size();
				}
				
				bool bSkip = false;
				if( maxY <= avgY * guiPtr->getValueF("thumbMaxY")  ){
					bSkip = true;
				}
								
				if( simp.size() && !bSkip ){
					
					angles.assign( simp.size(), 0.0 );
					
					float thumbTmpRaw;
					for(int k = 0; k < simp.size(); k++){
					
						int pre = k-1;
						int cur = k;
						int nex = k+1;
						
						if( pre < 0 ) pre += simp.size();
						if( nex >= simp.size() ) nex -= simp.size(); 
						
						ofPoint d1 = simp[cur]-simp[pre];
						ofPoint d2 = simp[nex]-simp[cur]; 
						
						float a1 = 360.0f + atan2(d1.y, d1.x) * RAD_TO_DEG; 
						float a2 = 360.0f + atan2(d2.y, d2.x) * RAD_TO_DEG;
						
						angles[k] = ofAngleDifferenceDegrees(a1, a2);
						
						if(  k == which ){
							if( angles[k] < -1.0 * guiPtr->getValueF("thumbAngleThresh") ){
								bThumbFound = true;
							}
						}
						
						int nextPoint = which+1;
						int prePoint  = which-1;

						if( nextPoint >= simp.size() ) nextPoint -= simp.size();
						if( prePoint < 0 )prePoint += simp.size();
						
						if( k == prePoint ){
							thumbTmpRaw =  angles[k];
						}
					}
					
					if( bThumbFound && thumbTmpRaw > 0 && thumbTmpRaw < 170 ){
						thumbAngleRaw = thumbTmpRaw;
					}else{
						bThumbFound = false;
					}
				}
			
				if( b.boundingRect.height > b.boundingRect.width ){
					bThumbFound = false;
				}
			}
			
			if( bThumbFound ){
				thumbFoundThresh = 1.0;
			}else{
				thumbFoundThresh *= 0.6;
			}
			
			thumbAngle = ofLerpDegrees(thumbAngle, thumbAngleRaw, 0.6); 
		}

		bool indexMatched(int v, vector <int> & arr ){
			for(int k = 0; k < arr.size(); k++){
				if( arr[k] == v ){
					return true;
				}
			}
			return false;
		}

		vector <ofPoint> getThreePoints(ofPolyline & pts, vector <int> res, vector <int> resRev, int start = 0){
			
			vector <ofPoint> ptsOut;
			vector <int> centerInt; 
			ofPoint p1, p2, p3;
			
			int stage = 0;
			for(int c = 0; c < pts.size(); c++){
				int j = c + start; 
				if( j >= pts.size() ) j -= pts.size(); 
			
				if( stage == 0 || stage == 1 ){
					if( indexMatched(j, res) ){
						p1 = pts[j];
						stage = 1;
					}
				}
				if( stage == 1 || stage == 2 ){
					if( indexMatched(j, resRev) ){
						centerInt.push_back( j );
						stage = 2;
						//cout << " got to stage 2 " << endl; 
					}
				}	
				if( stage == 2 ){
					if( indexMatched(j, res) ){
						p3 = pts[j];
						stage = 3; 
						//cout << " got to stage 3 " << endl; 
						break; 
					}
				}
			}
			
			if( stage == 3 && centerInt.size() ){
				p2 = pts[centerInt[centerInt.size()/2]];
				ptsOut.push_back(p1);
				ptsOut.push_back(p2);
				ptsOut.push_back(p3);
			}
			
			return ptsOut;
			
		}
		
		bool findFrontFacingThumb(float threshVal){
			bFrontFacingThumb = false;
			
			small3.dilate_3x3();
			small3.dilate_3x3();						
			small3.threshold(threshVal, false);
			small3.blur(guiPtr->getValueI("skelHandBlur"));
			small3.threshold(guiPtr->getValueI("handPostBlurThresh"), false);
			skel.scaleIntoMe(small3, CV_INTER_LINEAR);

			if( contourF2.findContours(skel, 30, getWidth() * getHeight() * 0.4, 10, false, true) ){
				
				frontFacingAngle = ofLerpDegrees(frontFacingAngle,  
												ofMap( contourF2.blobs[0].boundingRect.height / contourF2.blobs[0].boundingRect.width, 1.0, 1.8, 0.0, 70, true), 
												0.24); 
				
				if( contourF2.blobs.size() >= 2 ){
					
					b1 = contourF2.blobs[0].centroid;
					b2 = contourF2.blobs[1].centroid;
					
					if( fabs( b1.x - b2.x ) < contourF2.blobs[0].boundingRect.width * 0.8 ){
						if( b2.y > b1.y ){
							bFrontFacingThumb = true;
						}
					}
				}
			}
			
			return bFrontFacingThumb;
		}
		
		float getFrontFacingAngle(){
			return frontFacingAngle;
		}
		
		void updateAlt2( ofxCvGrayscaleImage & input, armModel & arm, ofPoint curHandPoint, float handSize, int backHandCuttof, float angleThresh, float angleThreshInner, int numPtsSearch, float handLenThresh ){
			if( handSize <= 8.0 )return;
			if( input.getWidth() <= 0 ) return; 
			if( handSize >= input.getWidth() * 0.6 ) return; 
			if( curHandPoint.x <= 0 || curHandPoint.x >= input.getWidth() ) return; 
			if( curHandPoint.y <= 0 || curHandPoint.y >= input.getHeight() ) return; 
			
			handSize = (int)handSize;
			
			//cout << "DEBUG b1 " << endl;
			if( handThresh.getWidth() == 0.0  ){
				if( input.getWidth() > 0 ){
					handThresh.allocate(input.getWidth()/2, input.getHeight()/2);
					skel.allocate(input.getWidth()/2, input.getHeight()/2);
					handThresh.set(0);
					skel.set(0); 
				}else{
					return;
				}
			}
			//cout << "DEBUG b2 - handSize" << handSize << " small width is " << small.getWidth() <<  endl;

			if( small.getWidth() == 0 || small.getWidth() != handSize ){
				small.clear(); 
				small.allocate(handSize, handSize);
				small.set(0);
				small2.clear(); 
				small2.allocate(handSize, handSize);
				small2.set(0);
				small3.clear(); 
				small3.allocate(handSize, handSize);
				small3.set(0);
			}

			//cout << "DEBUG b3 " << endl;

			ofRectangle rect;
			rect.setFromCenter((int)curHandPoint.x, (int)curHandPoint.y, handSize, handSize);
			
			//cout << "rect roi is: " << rect.x << " " << rect.y << " " << rect.width << "  " << rect.height << endl; 
//			cout << "small is " << small.getWidth() << " " << small.getHeight() << endl; 
			
			input.setROI(rect);
						
			//cout << "DEBUG b4 " << endl;

			small.scaleIntoMe(input, CV_INTER_LINEAR);
			small2 = small;
			small3 = small;
			input.resetROI();
						
			float handVal = getMaxPixelColor(input, curHandPoint, handSize/4);
			float avgVal = getAveragePixelColor(input, curHandPoint, handSize/4);
						
			float median = avgVal; 
			
			small.threshold(median+backHandCuttof, false);
			small.blur(guiPtr->getValueI("skelHandBlur"));

			//cout << "DEBUG b5 " << endl;

			handThresh.scaleIntoMe(small, CV_INTER_LINEAR);
			handThresh.threshold(guiPtr->getValueI("handPostBlurThresh"), false);
			
			contourF2.findContours(handThresh, 0, 10000000, 3, false, false);
			if( contourF2.blobs.size() ){
				handThresh.drawBlobIntoMe(contourF2.blobs[0], 255);
			}
			
			//cout << "DEBUG b6 " << endl;
			
			
			line.clear();
			simp.clear();
			angles.clear();
			bThumbFound = false;
			bFrontFacingThumb = false;
			thumbPts.clear();
			
			
			bool bSkipThumb = false;
			
			handLenThresh *= handThresh.getWidth();
			
			frontFacePct *= 0.92;
			
			if( contourF.findContours(handThresh, 10, (handThresh.getWidth() * 0.8) * (handThresh.getHeight() * 0.7), 3, false, false) ){
				
				ofRectangle blobBounds = contourF.blobs[0].boundingRect;
				
				bool bArmStraight = ( fabs( ofAngleDifferenceDegrees( arm.getElbowAngle(), 0 ) ) < 30 );
//				
//				if( !bArmStraight && blobBounds.width / blobBounds.height <= 1.35 ){
//					frontFacePct = ofLerp( frontFacePct , 1.0, 0.4 ); 					
//				}			
//				
				if( bArmStraight && (blobBounds.height / blobBounds.width) - 0.9  > 0 ){
					thumbFoundThresh *= 0.7;
					thumbAngleRaw = 0.0;
					thumbAngle = ofLerp(thumbAngle, thumbAngleRaw, 0.2);
					bSkipThumb = true;
				} 
				
				line = contourF.blobs[0].pts;
				line.setClosed(true);
				line = line.getSmoothed(guiPtr->getValueI("handContourSmooth"), 0.5);

				results		= lineUtils::getPeaks( line, angleThresh, numPtsSearch, false, true, guiPtr->getValueI("peakSkipAllowedForHand") );
				resultsRev	= lineUtils::getPeaks( line, angleThreshInner, numPtsSearch, true, true, guiPtr->getValueI("peakSkipAllowedForHand") ); 
				
				vector <int> newResults;
				for(int k = 0; k < results.size(); k++){
					if( line[results[k]].x < 0.96 * handThresh.getWidth() ){
						newResults.push_back( results[k] ); 
					}
				}
				
				if( newResults.size() < results.size() ){
					//cout << "removed: " << results.size() - newResults.size() << endl;
					results = newResults;
				}
				
				if( results.size() >= 2 ){
					if( resultsRev.size() >= 1 ){	
					
						vector <ofPoint> pts;
						float curLargest = 0.0;
						
						bool bFound = false;
						
						//lets loop around the shape starting at three different indexes
						//we will pick the shape that has the largest avg distance
						for(int k = 0; k < 1; k++){
							
							int start = ofMap(k, 0, 3, 0, line.size(), true);
							
							vector <ofPoint> ptsTmp = getThreePoints(line, results, resultsRev, start); 
							
							if( ptsTmp.size() == 3 ){
								float avgLen = ((ptsTmp[0]-ptsTmp[1]).length() + (ptsTmp[1]-ptsTmp[2]).length())/2;
								
								if( avgLen > curLargest && avgLen > handLenThresh ){
									pts			= ptsTmp;
									
									thumbPts = pts;
							
									ofPoint v1 = pts[0] - pts[1];
									ofPoint v2 = pts[2] - pts[1];

									angle1 = RAD_TO_DEG * atan2(v1.y, v1.x);							
									angle2 = RAD_TO_DEG * atan2(v2.y, v2.x);
									
									thumbAngleRaw = -ofAngleDifferenceDegrees( angle1, angle2 );
									
									bool badAngle1 = ( angle1 > -120 && angle1 < 0 );
									
									if( thumbAngleRaw > 0 && thumbAngleRaw < 160 && !badAngle1){
										curLargest	= avgLen;
										bFound		= true;
										thumbPts	= pts;
									}
									
								}
							}
						}
						
						if(bFound){
							bThumbFound = true;
						}						
					}
				}
				
			}
			
			if( bSkipThumb ){
				bThumbFound = false;
			}
						
			if( bThumbFound ){
				thumbAngle = ofLerp(thumbAngle, thumbAngleRaw, 0.2);
				thumbFoundThresh = ofLerp(thumbFoundThresh, 1.0, 0.1);
			}else{
				thumbAngleRaw = 0.0;
				thumbAngle = ofLerp(thumbAngle, thumbAngleRaw, 0.2);
				thumbFoundThresh = 0.0;
			}		
			
				
			if( frontFacePct >= 0.5 && bThumbFound == false ){
				bFrontFacing = true;
			}else{
				bFrontFacing = false;
			}
			
					
			if( bFrontFacing ){
				findFrontFacingThumb((handVal*0.37 + avgVal*0.63));
			}		
			
		}

		
		float getThumbFound(){
			return ( thumbFoundThresh > 0.4 );
		}
		
		float getWidth(){
			return skel.getWidth();
		}
		
		float getHeight(){
			return skel.getHeight();
		}
		
		void draw(float x, float y){
			draw(x, y, getWidth(), getHeight());
		}
		
		void draw(float x, float y, float w, float h){
			ofPushStyle();
				ofPushMatrix();
					ofTranslate(x, y, 0.0);
					ofScale( w/getWidth(), h/getHeight());
					
					ofSetColor(255);
					skel.draw(0, 0);
					
					if( bFrontFacing ){
						ofSetColor(0, 255, 0); 
						ofCircle(getWidth() - 35, 35, 15);
					}
					
					if( bFrontFacingThumb ){
						ofSetColor(70, 190, 255); 
						ofCircle(getWidth() - 35, 35 + 40, 15);
						
						ofLine( b1, b2 );
						ofCircle(b1, 4); 
						ofCircle(b2, 4); 
					}
					
					
					ofTranslate(0, -10, 0);
					ofSetColor(100, 190, 100);
			
					line.draw();
					
					if( results.size() ){
						ofSetColor(250, 100, 0);
						ofCircle(line[0], 10.0);
						ofSetColor(100, 100, 200);
						for(int k = 0; k < results.size(); k++){
							ofCircle(line[results[k]], 3.5);
						}
						ofSetColor(250, 100, 100);
						for(int k = 0; k < resultsRev.size(); k++){
							ofCircle(line[resultsRev[k]], 2.5);							
						}
						
						if( thumbPts.size()>=3 ){
							ofPushStyle();
								ofNoFill();
								ofSetColor(255, 255, 255);
								ofBeginShape();
									ofVertexes(thumbPts);
								ofEndShape(false);
								ofCircle(thumbPts[2], 2.5);
							ofPopStyle();
						}
					}

					ofPushMatrix();
					ofTranslate(0, 40, 0);
					ofSetColor(0, 255, 255); 
					simp.draw();
					ofPopMatrix();
					
					ofTranslate(-4, 15, 0);
					
					ofSetColor(255, 100, 100); 
					for(int k = 0; k < angles.size(); k++){
						ofDrawBitmapString(ofToString(angles[k], 2), simp[k]);
					}
					
					ofNoFill();
					if( bThumbFound ){
						ofDrawBitmapString("ANGLE IS " + ofToString(thumbAngleRaw, 2), 20, 20);
						ofDrawBitmapString("ANGLE 1 " + ofToString(angle1, 2), 20, 40);
						ofDrawBitmapString("ANGLE 2 " + ofToString(angle2, 2), 20, 60);					
					}
					
					
				ofPopMatrix();
			ofPopStyle();
		}
		
		ofPolyline line;
		ofPolyline simp;
		ofxCvContourFinder contourF;
		ofxCvContourFinder contourF2;
		
		ofxCvGrayscaleImage handThresh;
		ofxCvGrayscaleImage small;
		ofxCvGrayscaleImage small2;
		ofxCvGrayscaleImage small3;
		ofxCvGrayscaleImage skel;
		
		vector <float> angles;
		vector <int> results;
		vector <int> resultsRev;
		vector <ofPoint> thumbPts;
		
		float frontFacingAngle;
		
		float angle1, angle2;
		float frontFacePct;
	
		bool bFrontFacing;
		bool bFrontFacingThumb;
		bool bThumbFound; 
		
		ofPoint b1, b2;
		
		float thumbFoundThresh;
		
		float nextAngle;
		float nextAngle2;
		
		float thumbAngleRaw, thumbAngle;
		
		ofPoint thumbPt;

};