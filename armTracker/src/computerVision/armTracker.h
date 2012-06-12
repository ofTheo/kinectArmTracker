#include "getSkeleton.h"
#include "lineUtils.h"
#include "armModel.h"
#include "cvUtils.h"
#include "ofControlPanel.h"

extern ofControlPanel * guiPtr;

using namespace cvUtils;
using namespace lineUtils;

static bool InsidePolygon( vector <ofPoint> & polygon, ofPoint & p){
  int counter = 0;
  int i;
  double xinters;
  ofPoint p1,p2;
  int N = polygon.size(); 
  
  if( N <= 2 )return false;

  p1 = polygon[0];
  for (i=1;i<=N;i++) {
    p2 = polygon[i % N];
    if (p.y > MIN(p1.y,p2.y)) {
      if (p.y <= MAX(p1.y,p2.y)) {
        if (p.x <= MAX(p1.x,p2.x)) {
          if (p1.y != p2.y) {
            xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
            if (p1.x == p2.x || p.x <= xinters)
              counter++;
          }
        }
      }
    }
    p1 = p2;
  }

  if (counter % 2 == 0)
    return false;
  else
    return true;
}

static bool sortContoursLeft(ofxCvBlob A, ofxCvBlob B){
	float leftMostAV = 100000;
	
	for(int k = 0; k < A.pts.size(); k++){
		if( A.pts[k].x < leftMostAV ){
			leftMostAV = A.pts[k].x; 
		}
	}

	float leftMostBV = 100000;
	
	for(int k = 0; k < B.pts.size(); k++){
		if( B.pts[k].x < leftMostBV ){
			leftMostBV = B.pts[k].x; 
		}
	}
	
	return leftMostAV < leftMostBV;
}

class indexPoint : public ofPoint{
	public:
	
	indexPoint( ofPoint p, int indexI){
		x = p.x;
		y = p.y;
		z = p.z;
		
		index = indexI;
	}
	int index;
};

class armTracker : public ofBaseDraws {
	public:
		
		armTracker(){
			bArmFound  = false;
			curLineLengthSmoothed = 200.0;
			curLineLength = 0.0; 
			averageYPosition = 0.0;
			depthHand = 0.0;
			depthShoulder = 0.0;
			timeArmLastFirstSeen = 0.0;
			timeLastSeen = 0.0;
			timeToWaitTillSeen = 1.5;
			timeLastPersonSeen = 0.0;
		}
		
		void setup(float w, float h){
			
		}
	
		float getCurrentHandAngle(float relToZero){
			return ofAngleDifferenceDegrees(curHandAngle + 360, relToZero);
		}
		
		bool armFound(){
			if( bArmFound && ofGetElapsedTimef() - timeArmLastFirstSeen > timeToWaitTillSeen){
				return true;
			}
			return false;
		}
		
		vector <indexPoint> removeAbruptPointsNear( vector <indexPoint> & abPts, vector <ofPoint> & nearPts, float thresh ){
			
			vector <indexPoint> newPts;
			vector <int> removeList; 
			
			for(int k = 0;  k < nearPts.size(); k++){
				for(int i = 0; i < abPts.size(); i++){
					if( ( ofPoint( abPts[i].x, abPts[i].y ) - nearPts[k] ).length() < thresh ){						 
						
//						if( k == 0 ){
//							cout << "removing from shoulder" << endl; 
//						}
//
//						if( k == 1 ){
//							cout << "removing from elbow" << endl; 
//						}
						
						removeList.push_back(i);
					}
				}
			}
			
			for(int i = 0; i < abPts.size(); i++){				
				bool keep = true; 
				for(int k = 0; k < removeList.size(); k++){
					if( i == removeList[k] ){
						keep = false;
						break;
					}
				}
				if( keep ){
					newPts.push_back ( abPts[i] );
				}
			}
			
			return newPts; 
		} 
		
		void update(ofxCvGrayscaleImage & input, ofxCvGrayscaleImage & blurInput, int numSegments = 5, int smoothAmnt = 11, 
				int smoothThresh = 30, float minLineLength = 100.0, float maxPercentInPoly = 0.6){
				
			abruptPts.clear();
			
			if( input.getWidth() == 0 ) return;
			
			if( gray.getWidth() == 0 ){
				//cout << "DEBUG 0 " << endl; 
				gray.setUseTexture(true);
				gray.allocate(input.getWidth(), input.getHeight());
				thresh.setUseTexture(true);
				thresh.allocate(input.getWidth(), input.getHeight());
				skel.allocate(input.getWidth(), input.getHeight());
				gray2.setUseTexture(true);
				gray2.allocate(input.getWidth()/3, input.getHeight()/3);
				skelSmall.setUseTexture(true);
				skelSmall.allocate(input.getWidth()/2, input.getHeight()/2);
				armM.setup(80, 110, 40);
				//cout << "DEBUG 1 " << endl; 
				
			}
		
				
			
			//percentInPoly = 0.0;
			
			if( guiPtr->getValueB("bFilterWhitePixels") ){
				unsigned char * pix = input.getPixels(); 
				int num = input.getWidth() * input.getHeight();
				for(int i = 0; i < num; i++){
					if( pix[i] == 255 ){
						pix[i] = 0;
					}
				}
				input.setFromPixels(pix, input.getWidth(), input.getHeight());
				input.flagImageChanged();
			}
			
			//cout << "DEBUG 2 " << endl;

			results.clear();
			gray = input;
			
			gray2.scaleIntoMe(gray, CV_INTER_LINEAR);
			gray2.threshold(2, false);
			gray2.blur(smoothAmnt);
			gray2.erode();
			
			//cout << "DEBUG 3 " << endl;
		
			
			thresh.scaleIntoMe(gray2, CV_INTER_LINEAR);
			thresh.threshold(smoothThresh, false);
			
			finder.findContours(thresh, powf(input.getHeight() * 0.2, 2) * guiPtr->getValueF("minBlobSizePct"), powf(input.getHeight() * 0.7, 2) * guiPtr->getValueF("maxBlobSizePct"), 3, false, true);

			//cout << "DEBUG 4 " << endl;

			bArmFoundPrevFrame = bArmFound; 
			bArmFound = false;
			
			if( bArmFoundPrevFrame == false ){
				timeToWaitTillSeen = ofMap(ofGetElapsedTimef()-timeLastSeen, 0.5, 5.0, 0.0, 1.1, true);
			}
					
			if( finder.blobs.size() ){
				
				averageYPosition *= 0.96;
				averageYPosition += 0.04 * ( (finder.blobs[0].boundingRect.y * 0.85 + finder.blobs[0].centroid.y * 0.15) / thresh.getHeight() );
			
				gray2.threshold(smoothThresh, false);
				
				skelSmall.scaleIntoMe(thresh, CV_INTER_LINEAR);
				getSkeleton::doFromImage(skelSmall, skelSmall);
				
				//cout << "DEBUG 5 " << endl;
	
				shoulderLeftX = finder.blobs[0].boundingRect.x + finder.blobs[0].boundingRect.width * 0.47;
				boundsLeft = finder.blobs[0].boundingRect.x;
				
				chestPt = finder.blobs[0].centroid;
				
				shoulderLeftX *= 0.75;
				shoulderLeftX += 0.25 * finder.blobs[0].centroid.x;
			}else{
				reason = " REASON: No person in range "; 
				return;
			}
			
			skelFinder.findContours(skelSmall, 120/4, 10000000, 100, false, false);
			
			if( skelFinder.blobs.size() ){
			
				vector <ofxCvBlob> blobsTmp = skelFinder.blobs;
				
				for(int k = 0; k < blobsTmp.size(); k++){
					ofxCvBlob & b = blobsTmp[k];
					b.boundingRect.x *= 2.0;
					b.boundingRect.y *= 2.0;
					b.boundingRect.width *= 2.0;
					b.boundingRect.height *= 2.0;
					b.centroid *= 2.0;
					b.area *= 4.0; 
					b.length *= 4.0;
					
					 for(int j = 0; j < b.pts.size(); j++){
						b.pts[j] *= 2.0;
					 }
				}
				
				//cout << "DEBUG 6 " << endl;
				
				if( blobsTmp.size() > 6 ){
					blobsTmp.erase(blobsTmp.begin(), blobsTmp.begin()+blobsTmp.size()/3);
				}
				sort(blobsTmp.begin(), blobsTmp.end(), sortContoursLeft);
				
				int curB = 0;
				ofxCvBlob bestBlob = blobsTmp[curB];
				
				bool bFound = false;
				for( int k = 0; k < blobsTmp.size(); k++ ){
					curB = k;
					
					if( !InsidePolygon(finder.blobs[0].pts, blobsTmp[curB].pts[ blobsTmp[curB].pts.size()/2 ] ) ){
						continue;
					}
					
					if( blobsTmp[curB].pts.size() > gray.getWidth() / 8.0f ){
						bestBlob = blobsTmp[curB];
						bFound = true;
						break;
					}
				}
				
				if( !bFound ){
					reason = "REASON: all lines were too short"; 
					return;
				}

				armLine	= bestBlob.pts;
				armLineLowRes.clear();

				float skelHeight = skelSmall.getHeight() * 2.0;
				float shoulderLeftX_Comp = shoulderLeftX;
				
				float startY = guiPtr->getValueF("LegRemoveYStartPct");
				float endY = guiPtr->getValueF("LegRemoveYEndPct");
				float reduceAmnt = guiPtr->getValueF("LegRemoveYReducePct");


				for(int k = 0; k < armLine.size(); k++){
					shoulderLeftX_Comp = shoulderLeftX * ofMap( armLine[k].y, skelHeight * startY, skelHeight * endY, 1.0, reduceAmnt, true ); 
					
					if( armLine[k].x <= shoulderLeftX_Comp  ){
						armLineLowRes.addVertex( armLine[k] );
					}
				}
				
//				armLine	= bestBlob.pts;
//				
//				armLineLowRes.clear();
//				for(int k = 0; k < armLine.size(); k++){
//					if( armLine[k].x <= shoulderLeftX ){
//						armLineLowRes.addVertex( armLine[k] );
//					}
//				}
//				
				armLineLowRes.setClosed(true);
				armLineLowRes = armLineLowRes.getSmoothed(9, 0.5);
				
				results = lineUtils::getPeaks(armLineLowRes, 15.0, 20, false, true, guiPtr->getValueI("peakSkipAllowedForArm"));
				if( results.size() ){
					for(int k = 0; k < results.size(); k++){
						abruptPts.push_back( indexPoint( armLineLowRes[ results[k] ], results[k] ) );
					}
				}
				//cout << " results size is " << results.size() << endl; 
				
				//armLineLowRes = armLineLowRes.getResampledBySpacing( 3 );//blobs[0].boundingRect.width / 15.0 );	
				//armLineLowRes = armLineLowRes.getSmoothed(2, 0.5);
				
				blurInput.threshold(guiPtr->getValueI("blur_bounds_thresh"));
				finderBody.findContours(blurInput, powf(blurInput.getHeight() * 0.2, 2), blurInput.getWidth() * blurInput.getHeight() * 0.5, 10, false, true); 
				
				if( finderBody.blobs.size() ){
					float xscale = ( input.getWidth() / blurInput.getWidth() );
					float yscale = ( input.getHeight() / blurInput.getHeight() );
				
					ofRectangle checkRect = finderBody.blobs[0].boundingRect;
					checkRect.x *= xscale;
					checkRect.y *= yscale;
					checkRect.width *= xscale;
					checkRect.height *= yscale;
					
					checkRect.y -= checkRect.height/3; 
					checkRect.height += checkRect.height/3;
				
					percentInPoly = getPercentInPoly(armLineLowRes, checkRect);
					if( percentInPoly >= maxPercentInPoly ){
						reason = " REASON: not in poly "; 
						return;
					}
				}
				
				float centerX = 0.0; 
				float tipX = 1000; 
			
				float preAngle = 0.0;
				float avgAngle = 0.0;
				
				for(int k = 0; k < armLineLowRes.size(); k++){
					if( armLineLowRes[k].x < tipX ){
						tipX = armLineLowRes[k].x;
					}
					centerX += armLineLowRes[k].x;
				}
				
				if( armLineLowRes.size() ){
					centerX /= (float)armLineLowRes.size();
				}
					
				//--- LETS FIND THE CLOSEST POINT TO THE SHOULDER AND FURTHEST POINT 
				
				//if( centerX < bestBlob.centroid.x ){
					
					int closest = findClosestPoint(armLineLowRes, chestPt);
					if( closest >= 0 ){
						closestPoint = armLineLowRes[closest];
					}
					
					int furthest = findFurthestPoint(armLineLowRes, chestPt);
					if( furthest >= 0 ){
						furthestPoint = armLineLowRes[furthest];
						
						int wristPtIndex = furthest-6;
						if( wristPtIndex < 0 ) wristPtIndex += armLineLowRes.size();
						
					}	
													
					if( closest >= 0 && furthest >= 0 ){
						oneHalf.clear();
						
						for(int k = 0; k < armLineLowRes.size(); k++){
							int cur = k + closest; 
							cur %= armLineLowRes.size(); 
							oneHalf.addVertex(armLineLowRes[cur]);

							if( cur == furthest )break;						
						}
						
						float curLen = getLineLength(oneHalf);
						if( !std::isnan(curLen) ){
							curLineLength = curLen;
						}
						curLineLengthSmoothed *= 0.92;
						curLineLengthSmoothed += curLineLength * 0.08;
						
//						int wristPtIndex = getPointAtPercent(oneHalf, 0.95); 
//						
//						cout << " wristPoint is " << wristPtIndex << endl; 
//						if( wristPtIndex > 0 ){
//							wristPt = oneHalf[wristPtIndex];
//						}
						
						if( fabs( curLineLengthSmoothed - curLineLength ) > curLineLengthSmoothed / 8.0 ){
							reason = " REASON: arm length changed too quickly ";
							//bArmFound = false;
							//return;
						}
						
						if( curLineLengthSmoothed < minLineLength ){
							reason = " REASON: arm length changed too quickly ";
							bArmFound = false;
							return;						
						}
					}
					
					//LETS FIND AVG ANGLE					
					if( armLineLowRes.size() >= 2 ){
						int start = 0;
						if( closest >= 0 ){
							start = closest; 
						}
						
						int numAdded = 0;
						ofPoint avgDelta; 
												
						for(int k = 0; k < armLineLowRes.size(); k++){
							
							int cur = start + k; 
							cur %= armLineLowRes.size(); 
							
							int pre = cur-1;
							if( pre < 0 ){
								pre += armLineLowRes.size(); 
							}
							
							if( k < armLineLowRes.size()/2 ){								
								avgDelta += ( armLineLowRes[cur] - armLineLowRes[pre] ).normalized(); 
								numAdded ++;
							}
						}
						
						avgDelta /= (float)numAdded;
						avgAngle = RAD_TO_DEG * atan2(avgDelta.y, avgDelta.x); 
					}
										
					float angleDiff1 = ofAngleDifferenceDegrees(avgAngle, 180.0);					
					curAngle = avgAngle;

					// HERE WE CORRECT OUR UNDERSTANDING OF FURTHEST 
					
					bSwapped = 0;
					
					lengths.clear();
					
					if( armLineSimp.size() >= 2 ){
																		
						ofPolyline tmp;
						for(int i = 0; i < armLineSimpler.size()/2 + 2; i++){
							tmp.addVertex(armLineSimpler[i]);
						}												
						elbowPt = armLineSimpler[ getBiggestAngleJoint( tmp ) ]; 
						
						ofPoint lowestPt(0.0, 0.0); 
						for(int k = 0; k < oneHalf.size(); k++){
							if( k > oneHalf.size()/3 && k < ((float)oneHalf.size() * 2.0/3.0) ){
								if( oneHalf[k].y > lowestPt.y ){
									lowestPt = oneHalf[k]; 
								}
							}
						}
						
						if( lowestPt.y != 0.0 ){
							elbowPt *= 0.3;
							elbowPt += lowestPt * 0.7; 
						}
						
					}
					
					//OKAY WE HAVE AN ARM!!! 
					//--------
					//--------
					if( fabs(angleDiff1) < 80 ){
					
						bArmFound = true;
						if( bArmFoundPrevFrame == false ){
							timeArmLastFirstSeen = ofGetElapsedTimef(); 
						}
						timeLastSeen = ofGetElapsedTimef();
						
						vector <ofPoint> checkPts;
						checkPts.push_back( closestPoint );
						checkPts.push_back( elbowPt ); 
						
//						cout << " ABRUPT PTS WAS " << abruptPts.size() << endl;
						abruptPts = removeAbruptPointsNear( abruptPts, checkPts, 60.0 ); 
//						cout << " ABRUPT PTS IS " << abruptPts.size()  << endl;
						
						if( abruptPts.size() ){
							lengths.clear();
							
							float d1 = (furthestPoint - closestPoint).length();
							float d2 = (furthestPoint - elbowPt).length();
							float d  = d1 * 0.4 + d2 * 0.6;
						
							lengths.push_back( d ); 
												
							int which = -1;
							float maxDist = 0.0; 
							
							int secondPt = -1;
							float maxDist2nd = 0.0;
							
							for(int k = 0; k < abruptPts.size(); k++){

								float d1 = (abruptPts[k] - closestPoint).length();
								float d2 = (abruptPts[k] - elbowPt).length();
								float d  = d1 * 0.4 + d2 * 0.6;
																
								lengths.push_back( d ); 
						
								if( d >= maxDist ){
									maxDist2nd = maxDist; 
									secondPt = which;
									
									maxDist = d;
									which = k;
								}
								
								if( d < maxDist && d > maxDist2nd ){
									maxDist2nd = d; 
									secondPt = k; 
								}

							}
							
							//lets change furthest based on our abrupt points
							
							if( maxDist > lengths[0] ){
							
								bSwapped = 1;
								
								ofPoint pTry1 = abruptPts[which];
								int firstIndex = which;
																								
								if( secondPt >= 0 && maxDist2nd > lengths[0] * 0.7 && bArmFoundPrevFrame ){
									
									ofPoint pTry2 = abruptPts[secondPt];
									
									if( (pTry2 - preFurthestPoint).length() < (pTry1 - preFurthestPoint).length() ){
										pTry1 = pTry2;
										firstIndex = secondPt;
										
										bSwapped = 2;
									 }
								}
							
								furthestPoint.set(pTry1.x, pTry1.y, 0.0);
								furthest = abruptPts[firstIndex].index;
								
							}	
							
							ofPoint handCenter = (furthestPoint + wristPt) / 2.0;
							handRect.setFromCenter(handCenter.x, handCenter.y, MIN(curLineLengthSmoothed / 2.0, gray.getWidth()/4.0), MIN(curLineLengthSmoothed / 2.0, gray.getWidth()/4.0) );					
						
							preFurthestPoint = furthestPoint;																		
																																																													
						}
					}else{
						reason =  " REASON: doesn't match angle - angleDiff1 is " + ofToString( angleDiff1, 2 );
						return;
					}	

					armLineSimp = oneHalf;
					armLineSimp.simplify( curLineLength / ( numSegments * 8 ) );
					armLineSimpler = oneHalf.getResampledByCount( 10 );					
					
					if( armLineSimp.size() >= 2 ){
						
						float lenFromTip = (curLineLength/10.0)/2; 
						
						ofPoint delt2 = furthestPoint - closestPoint;
						
						int distBack = lenFromTip; 
						
						int ptToTry1 = (furthest + distBack) % armLineLowRes.size();
						int ptToTry2 = furthest - distBack; if( ptToTry2 < 0 ) ptToTry2 += armLineLowRes.size(); 
						
						t1 = armLineLowRes[ptToTry1];
						t2 = armLineLowRes[ptToTry2];

						ofPoint delt3 = t1;						
						if( (t1 - wristPt).length() > (t2 - wristPt ).length() ){
							delt3 = t2;
						}

						ofPoint checkPt  = delt3 * 1.0; 
						//checkPt *= 0.5;
						
						unsigned int nearestIndex = 0;
						armLineLowRes.getClosestPoint(checkPt, &nearestIndex);
						
						if( nearestIndex != -1 ){
							ofPoint tmpWrist = armLineLowRes[nearestIndex];
							
							attemptWrist = tmpWrist;
							attemptWristN = checkPt;
							
							ofPoint delta = tmpWrist - furthestPoint;
							delta.normalize(); 							
							delta *= curLineLength / 8;
							tmpWrist = furthestPoint + delta;
							
							float amnt = 0.78;
							float wristDiff = (tmpWrist - wristPt).length();
							if( wristDiff > 8 ){
								amnt = ofMap(wristDiff, 8, 20, amnt, 0.1, true);
							}
							
							wristPt *= amnt; 
							wristPt += tmpWrist * (1.0-amnt); 

						}
						
					}
					
					
//				}
//				else{
//					reason =  " REASON: center X is not less than blob centroid ";
//					return;
//				}					
			}else{
				reason =  " REASON: no skeleton found ";
				return; 
			}
			
			
			if( armFound() ){
				float updateAmnt = 0.5; 
				
//				float shoulderDelta = ( closestPoint - armM.shoulder ).length(); 
//				if( shoulderDelta > curLineLength / 2 ){
//					updateAmnt = ofMap(shoulderDelta, curLineLength / 8, curLineLength / 2, updateAmnt, 0.7, true);
//				}
//				
				depthShoulder = getAveragePixelColor(gray, closestPoint, 20);
				depthHand	  = getAveragePixelColor(gray, furthestPoint, 20);
				
				float diff = depthHand - depthShoulder;
				
				armM.headAngle *= 0.9;
				armM.headAngle += ofMap( diff, 40, -20, 40, -20, true) * 0.1;
				
				armM.update(closestPoint, elbowPt, wristPt, furthestPoint, updateAmnt);
			}

		}
		
		float timeNoArmSeen(){
			return ofGetElapsedTimef() - timeLastSeen;
		}	
		
		float timeNoPersonSeen(){
			return ofGetElapsedTimef() - timeLastPersonSeen;
		}
		
		void resetTimeNoPersonSeen(){
			timeLastPersonSeen = ofGetElapsedTimef();
		}
		
		ofPoint getHandCenter(){
			return handRect.getCenter();
		}	
		
		float getHandSearchWidth(){
			return handRect.width;
		}
		
		void draw(float x, float y){
			draw(x,y, gray.getWidth(), gray.getHeight());
		}
		
		void draw(float x, float y, float w, float h){
		
			if( gray.getWidth() == 0 )return;
		
			ofPushStyle();
			ofSetColor(255);
			
			ofPushMatrix();
			ofTranslate(x, y, 0);
			ofScale(w/gray.getWidth(), h/getHeight(), 1.0); 
			
			gray.draw(0, 0, w, h);

//			ofEnableAlphaBlending();
//			ofSetColor(255, 255, 255, 100);
//			thresh.draw(0, 0, w, h);
						
			ofEnableAlphaBlending();
			ofSetColor(255, 255, 255, 100);
			skelSmall.draw(0, 0, w, h);
				

			if( finder.blobs.size() ){
			
				timeLastPersonSeen = ofGetElapsedTimef();
				
				ofSetColor(90, 90, 90, 255);
				ofLine(0, averageYPosition * thresh.getHeight(), thresh.getWidth(), averageYPosition* thresh.getHeight()); 
			
				ofSetColor(230, 120, 50);
				ofCircle(chestPt.x, chestPt.y, 8.0); 
				
				ofSetColor(10, 90, 200);
				ofLine( shoulderLeftX, 0, shoulderLeftX, gray.getHeight() ); 
				ofLine( boundsLeft, 0, boundsLeft, gray.getHeight() ); 
				
				if( bArmFound ){
				
					if( lengths.size() ){
					
						ofSetColor(255);
						for(int k = 0; k < lengths.size(); k++){
							//cout << " length is " << lengths[k] << endl; 
							ofLine( 10 + k * 20, finder.getHeight(), 10 + k * 20, finder.getHeight()-lengths[k]);
						}
					}
					
					ofFill();
					
					if( bSwapped ){
						ofSetColor(255.0, 100, 30);
					}else{
						ofSetColor(0.0, 255.0, 0.0);					
					}
					ofCircle(furthestPoint.x, furthestPoint.y, 10.0);
					ofCircle(furthestPoint.x, furthestPoint.y, 3.0);				
				
					ofSetColor(0, 100, 230);
					if( results.size() ){
						for(int k = 0; k < results.size(); k++){
							ofNoFill();
							ofCircle(armLineLowRes[results[k]], 7.0);
						}
					}
									
					ofSetColor(255, 255, 0);
					ofSetLineWidth(3);
					armLineLowRes.draw();	

					ofPushMatrix();
					ofTranslate(0, 20, 0);
				
					ofSetLineWidth(2);
					armLineLowRes.draw();	
					

					ofPopMatrix();
				
//					ofPushMatrix();
//						ofTranslate(0, -10, 0);
//						ofSetColor(120, 120, 120);
//						oneHalf.draw();
//					ofPopMatrix();
					
//					if( armLineSimp.size() >= 2 ){
//						ofPushMatrix();
//							ofTranslate(0, 70, 0);
//							ofSetColor(255, 0, 0); 
//							for(int j = 1; j < armLineSimp.size(); j++){
//								ofSetColor( (100 + j * 70) % 255, 0, (j * 70) % 255); 
//								ofLine(armLineSimp[j-1], armLineSimp[j]);
//								if( j < armLineSimp.size()-1 ){
//									ofPoint d2 = armLineSimp[j+1] -  armLineSimp[j];
//									ofPoint d1 = armLineSimp[j] -  armLineSimp[j-1];
//									
//									ofDrawBitmapString( ofToString(d2.angle(d1), 2), armLineSimp[j] + ofPoint(0, 13) );
//								}
//							}
//							
//							ofTranslate(0, 10, 0);
//							armLineSimpler.draw();
//						ofPopMatrix();
//					}
					
					ofFill();
					ofSetColor(0, 255, 0);
					for(int k = 0; k < abruptPts.size(); k++){
						ofCircle(abruptPts[k].x, abruptPts[k].y, 6.0);
					}
								
					ofNoFill();
		
					ofSetColor(190, 100, 30);
					ofRect(handRect);								
					
					ofSetColor(255);
					ofPushMatrix();
						ofTranslate(0, -50, 0);
						ofLine(closestPoint, elbowPt);
						ofCircle(elbowPt, 4.0);
						
						ofDrawBitmapString(ofToString( armM.getElbowAngle(), 0 ), elbowPt.x, elbowPt.y - 10);
						
						ofLine(elbowPt, wristPt);
						
						ofCircle(wristPt, 4.0);
						ofLine(elbowPt, wristPt);	

						ofCircle(furthestPoint, 4.0);
						ofLine(furthestPoint, wristPt);	
					ofPopMatrix();
					
					ofSetColor(255, 0, 255);
					ofCircle(attemptWristN, 5.0);
					ofCircle(attemptWrist, 12.0);
					ofCircle(attemptWrist, 2.0);


					ofSetColor(0, 255, 0);

					ofCircle(t1, 5.0);
					ofCircle(t2, 5.0);
					
					ofSetColor(0, 100, 255);

					ofCircle(t1, 3.0);
					ofCircle(t2, 3.0);				
					
					
					armM.draw(0, -100);
																																						
				}
				
				ofNoFill();
				if( bSwapped == 0){
					ofSetColor(255.0, 100, 30);
				}else if( bSwapped == 1 ){
					ofSetColor(0.0, 100.0, 255.0);					
				}else{
					ofSetColor(255.0, 100, 255.0);					
				}
				
				ofCircle(furthestPoint.x, furthestPoint.y, 30.0);
				ofCircle(furthestPoint.x, furthestPoint.y, 3.0);

				ofSetColor(100.0, 100.0, 0.0);
				ofCircle(closestPoint.x, closestPoint.y, 10.0);
				ofCircle(closestPoint.x, closestPoint.y, 3.0);
				
				ofPoint p2;
				p2.x = cos(curAngle * DEG_TO_RAD) * 100.0;
				p2.y = sin(curAngle * DEG_TO_RAD) * 100.0;
				
				ofLine( closestPoint, closestPoint +  p2 ); 
				
				ofPushMatrix();
					ofTranslate(0, 40, 0);
					for(int k = 0; k < lines.size(); k++){
						ofSetColor(230 - k * 4, 100 - k * 4, 0);

						lines[k].draw();
					}
				ofPopMatrix();
				
				ofPushStyle();
					finderBody.draw(0,0, gray.getWidth(), gray.getHeight());
					//skelFinder.draw(0,0);
				ofPopStyle();
				
				if(!bArmFound){
					//cout << reason << endl; 
					ofDrawBitmapString(reason, 10, 20);
				}else{
					ofDrawBitmapString("depth shoulder = " + ofToString(depthShoulder) + "\nDepth hand = " + ofToString(depthHand), 10, 20);
					ofDrawBitmapString("Length = " + ofToString(curLineLength, 2) + "\nSmoothed Len: " + ofToString(curLineLengthSmoothed, 2), 10, 58);				
				}
				
				ofDrawBitmapString("percentInPoly = " + ofToString(percentInPoly * 100.0, 0.0), 10, 100);
				
//				ofPushMatrix();
//					ofTranslate(x, y + 10, 0);
//					ofSetColor(50, 0, 200, 30);
//					ofBeginShape();
//					ofNoFill();
//					for(int d = 0; d < armLine.size()/2; d++){
//						if( d == 0 ){
//							ofCircle(armLine[d], 3);
//						}
//						
//						ofVertex(armLine[d]);
//						
//						if( d == (armLine.size()/2) - 1 ){
//							ofCircle(armLine[d], 7);
//						}
//					}
//					ofEndShape(false);
//					
//				ofPopMatrix();
							
			}

			ofPopMatrix();
			ofEnableAlphaBlending();
			ofPopStyle();
		}

		float getWidth(){
			return gray.getWidth();
		}
		
		float getHeight(){
			return gray.getHeight();
		}

		bool bArmFound;
		bool bArmFoundPrevFrame;
		
		vector <int> results;
		vector <float> lengths;
		int bSwapped;
	
		ofxCvGrayscaleImage skel;
		ofxCvGrayscaleImage skelSmall;
		
		ofxCvGrayscaleImage gray;
		ofxCvGrayscaleImage gray2;

		ofxCvGrayscaleImage thresh;
		ofxCvContourFinder finder;
		ofxCvContourFinder skelFinder;
		ofxCvContourFinder finderBody;
		ofPolyline armLine, armLineLowRes;
		ofPolyline armLineSimp;
		ofPolyline armLineSimpler;
		
		ofPolyline oneHalf; 
		
		vector <ofPolyline> lines;
		
		
		vector <indexPoint> abruptPts;
		
		ofPoint chestPt;
		
		ofPoint wristPt; 
		ofPoint elbowPt;
		
		float averageYPositionSmoothed;
		float averageYPosition;
		
		ofPoint attemptWristN;
		ofPoint attemptWrist;
		
		ofRectangle handRect;
		
		ofPoint furthestPoint;
		ofPoint closestPoint;
		ofPoint preFurthestPoint;
		
		float timeArmLastFirstSeen; 
		float timeToWaitTillSeen;
		float timeLastSeen;
		float timeLastPersonSeen;
		
		float percentInPoly;
		
		ofPoint t1; 
		ofPoint t2;
		
		string reason;
		
		armModel armM; 
		
		float curAngle;		
		float curHandAngle;
		float shoulderLeftX, boundsLeft;
		
		float depthHand;
		float depthShoulder;
		
		float curLineLength;
		float curLineLengthSmoothed;
		
};