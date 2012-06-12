#pragma once

/*
 *  armModel.h
 *  PuppetParade
 *
 *  Created by theo on 9/24/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
 
#include "kalmanTracker.h"

class armModel{

	public:
		
		armModel(){
			p.clear();

			p.push_back( &shoulder );
			p.push_back( &elbow );
			p.push_back( &wrist );
			p.push_back( &tip );
			
			headAngle = 0.0;

		}
		
		void setup( float _armLen, float _foreArmLen, float _wristLen ){
			cout << " armModel SETUP - START " << endl;
			
			armLen = _armLen;
			foreArmLen = _foreArmLen;
			wristLen = _wristLen;
			thumbLen = wristLen * 0.65;
			
			shoulder.set(foreArmLen + wristLen + armLen, 0); 
			elbow.set(foreArmLen + wristLen, 0); 
			wrist.set(wristLen, 0); 
			tip.set(0, 0); 
			
			thumbAttach.set(wristLen* 0.45, 0); 
			thumb.set(0, 0); 
			
			trackerX.setup(0.00001, 0.00001);
			trackerY.setup(0.00001, 0.00001);
			
			trackerWX.setup(0.00001, 0.00001);
			trackerWY.setup(0.00001, 0.00001);
			
			trackerX.setFirst(tip.x);
			trackerY.setFirst(tip.y);

			trackerWX.setFirst(wrist.x);
			trackerWY.setFirst(wrist.y);
			
			wristSmooth = 0.4;
			shoulderSmooth = 0.4;
			elbowSmooth = 0.4;
			
			curAngleW = 180.0f;
			curAngleS = 180.0f;
			curAngleE = 180.0f;
			curThumbAngle = 180.0f;
			
			cout << " armModel SETUP - DONE " << endl;
	
		}
		
		float getCurrentShoulderAngle(){	
		
			float res =  -180.0 + atan2( elbow.y-shoulder.y, elbow.x -shoulder.x )  * RAD_TO_DEG; 

			if( res > 180 ){
				res -= 360.0; 
			}else if( res < -180){
				res += 360.0; 
			}
					
			return res; 
		}

		float getCurrentElbowAngle(){	
		
			float res =  -180.0 + atan2( wrist.y-elbow.y, wrist.x -elbow.x ) * RAD_TO_DEG;
						
			if( res > 180 ){
				res -= 360.0; 
			}else if( res < -180 ){
				res += 360.0; 
			}	
			
			return res; 
		}

		float getCurrentWristAngle(){
			float res =  -180.0 + atan2( tip.y-wrist.y, tip.x -wrist.x ) * RAD_TO_DEG;
						
			if( res > 180.0 ){
				res -= 360.0; 
			}else if( res < -180.0 ){
				res += 360.0; 
			}	
			
			return res; 
		}		
		
		float getShoulderAngle(){
			float res =  -180.0 + atan2( elbow.y-shoulder.y, elbow.x -shoulder.x )  * RAD_TO_DEG; 

			curAngleS = ofLerpDegrees(curAngleS, res, 1.0-shoulderSmooth);

			if( curAngleS > 180 ){
				curAngleS -= 360.0; 
			}else if( curAngleS < -180){
				curAngleS += 360.0; 
			}
					
			return curAngleS; 
		}

		float getElbowAngle(){	
		
			float res =  -180.0 + atan2( wrist.y-elbow.y, wrist.x -elbow.x ) * RAD_TO_DEG;
			
			curAngleE = ofLerpDegrees(curAngleE, res, 1.0-elbowSmooth);
			
			if( curAngleE > 180 ){
				curAngleE -= 360.0; 
			}else if( curAngleE < -180 ){
				curAngleE += 360.0; 
			}	
			
			return curAngleE; 
		}

		float getWristAngle(){
			float res =  -180.0 + atan2( tip.y-wrist.y, tip.x -wrist.x ) * RAD_TO_DEG;
			
			curAngleW = ofLerpDegrees(curAngleW, res, 1.0-wristSmooth);
			
			if( curAngleW > 180.0 ){
				curAngleW -= 360.0; 
			}else if( curAngleW < -180.0 ){
				curAngleW += 360.0; 
			}	
			
			return curAngleW; 
		}
		
		float setWristSmoothing(float wristSmoothAmnt){
			wristSmooth = wristSmoothAmnt;
		}

		float setElbowSmoothing(float elbowSmoothAmnt){
			elbowSmooth = elbowSmoothAmnt;
		}
		
		float setShoulderSmoothing(float shoulderSmoothAmnt){
			shoulderSmooth = shoulderSmoothAmnt;
		}		
		
		
		float getThumbAngle(){
			return curThumbAngle; 
		}		
		
		void setThumbInfo( bool thumbFound, float thumbAngle ){
			thumbAngle = ofMap(thumbAngle, 0, 140, 0, 90, true); 
			
			if( thumbFound )curThumbAngle = ofLerpDegrees(curThumbAngle, thumbAngle, 0.85);
			else curThumbAngle = ofLerpDegrees(curThumbAngle, 0.0, 0.65);
		}
		
		void setFrontFacing(float tiltAngle = 0.0){
			tiltAngle += 180.0;
			
			headAngle = ofLerpDegrees(headAngle, 90.0, 0.16); 		
			curAngleW = ofLerpDegrees(curAngleW, tiltAngle, 0.2);
			
			float len = (tip-wrist).length(); 
			ofPoint rel( cos( (curAngleW)*DEG_TO_RAD) * len, sin( (curAngleW)*DEG_TO_RAD) * len ); 
			
			tip = wrist + rel; 
//			altTip = tip;
		}
		
		void setElbowAngle(float newAngle){
			wrist.rotate(newAngle, elbow, ofPoint(0, 0, 1));
			altTip.rotate(newAngle, elbow, ofPoint(0, 0, 1));
			tip.rotate(newAngle, elbow, ofPoint(0, 0, 1));
		}

		void setWristAngle(float newAngle){
			altTip.rotate(newAngle, wrist, ofPoint(0, 0, 1));
			tip.rotate(newAngle, wrist, ofPoint(0, 0, 1));
		}
		
		void update(ofPoint p1s, ofPoint p2e, ofPoint p3w, ofPoint p4t, float smoothAmnt = 1.0){
			
			//cout << " p3w was " << p3w << endl; 
			
			p3w.x = 2.0 * trackerWX.setNext(p3w.x);
			p3w.y = 2.0 * trackerWX.setNext(p3w.y);

			//cout << " p3w is " << p3w << endl; 
			
			ofPoint d1 = p2e - p1s;
			ofPoint d2 = p2e - p3w; 
			ofPoint d3 = p4t - p3w;
			ofPoint ogp3w = p3w;
			
			bLenZero = false;
			
			if( d1.length() == 0 ){
				bLenZero = true;
			}
			if( d2.length() == 0 ){
				bLenZero = true;
			}
			if( d3.length() == 0 ){
				bLenZero = true;
			}
									
			d1.normalize();
			d1 *= armLen; 

			d2.normalize();
			d2 *= foreArmLen; 

			ofPoint newp2e_1  = p1s + d1; 
			ofPoint newp2e_2  = p3w + d2; 
			
			p2e = ( newp2e_1 + newp2e_2 ) / 2;
			
			d1 = p2e - p1s; 
			d1.normalize();
			d1 *= armLen; 

			d2 = p3w - p2e; 
			d2.normalize();
			d2 *= foreArmLen; 
			
			p2e = p1s + d1;
			p3w = p2e + d2;
			
			//ofPoint d3 = p4t - p3w; 
			d3.normalize(); 
			d3 *= wristLen;
			
			p4t = p3w + d3; 
			
			float resX = 2.0 * trackerX.setNext(p4t.x);
			float resY = 2.0 * trackerY.setNext(p4t.y);	
			
			ofPoint kalTip(resX, resY);
			
			//cout << "ORIGINAL: " << p4t << "KALMAN: " << kalTip << endl; 
			
			ofPoint d3alt( kalTip - p3w ); 
			d3alt.normalize();
			d3alt *= wristLen;
			
			shoulder *= (1.0-smoothAmnt); 
			shoulder += p1s * smoothAmnt;
			
			elbow *= (1.0-smoothAmnt);
			elbow += p2e * smoothAmnt; 
			
			wrist *= (1.0-smoothAmnt);
			wrist += p3w * smoothAmnt; 

			tip *= (1.0-smoothAmnt);
			tip += p4t * smoothAmnt; 
			
			ofPoint newPt = p3w + d3alt;
			
			altTip *= 0.6;
			altTip += newPt * 0.4;
			
			float drawThumbAngle = -curThumbAngle*DEG_TO_RAD + atan2(d3alt.y, d3alt.x); 
			
			thumbAttach = (altTip + wrist )/2;			
			thumb.set( thumbAttach.x + cos(drawThumbAngle) * thumbLen, thumbAttach.y + sin(drawThumbAngle) * thumbLen, 0 ); 
		
			// LETS limit the wrist and elbow angles to reasonable limits
			
			curElbowDiff = ofAngleDifferenceDegrees(getCurrentElbowAngle(), getCurrentShoulderAngle());
			
			if( curElbowDiff >= 30 ){
				setElbowAngle( -1.0 * (30 - curElbowDiff ) );
			}
			else if( curElbowDiff <= -80 ){
				setElbowAngle( -1.0 * (-80 - curElbowDiff ) );
			}

			curWristDiff = ofAngleDifferenceDegrees(getCurrentWristAngle(), getCurrentElbowAngle());
			
			if( curWristDiff >= 90 ){
				setWristAngle( -1.0 * (90 - curWristDiff ) );
			}
			else if( curWristDiff <= -90 ){
				setWristAngle( -1.0 * (-90 - curWristDiff ) );
			}
			
			
		}
		
		void draw(float x, float y){
			ofPushMatrix();
			ofTranslate(x, y, 0);
			
			ofSetColor( 50, 80, 215 ); 
			ofLine( shoulder, elbow ); 

			ofSetColor( 70, 100, 235 ); 
			ofLine( elbow, wrist ); 

			ofSetColor( 90, 120, 255 ); 
			ofLine( wrist, tip ); 
			
			ofSetColor( 200, 200, 255 ); 			
			ofLine(wrist, altTip);

			ofSetColor( 240, 240, 255 ); 			
			ofLine(thumbAttach, thumb);
			
			ofDrawBitmapString(ofToString(curElbowDiff,0), elbow.x, elbow.y - 20);
			
			if( bLenZero ){
				ofSetColor(255, 50, 40);
				ofCircle(0, 0, 40);
			}

			ofPopMatrix();
		}
		
		
		bool bLenZero;
		ofPoint shoulder;
		ofPoint elbow;
		ofPoint wrist; 
		ofPoint tip; 
		ofPoint altTip;
		ofPoint thumbAttach;
		ofPoint thumb;
		
		float curAngleW;
		float curAngleE;
		float curAngleS;
		
		float curElbowDiff;
		float curWristDiff;
		
		float curThumbAngle; 
		
		float wristSmooth;
		float elbowSmooth;
		float shoulderSmooth;
		
		float headAngle;
		
		kalmanTracker trackerX;
		kalmanTracker trackerY;

		kalmanTracker trackerWX;
		kalmanTracker trackerWY;

		float thumbLen; 
		float armLen; 
		float foreArmLen;
		float wristLen; 
		
		vector <ofPoint *> p;
};