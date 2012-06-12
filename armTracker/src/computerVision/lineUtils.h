#pragma once 
#include "ofMain.h"

namespace lineUtils{		

		static int findClosestPoint( ofPolyline & l, ofPoint p ){
			
			float dist = 99999999.0; 
			int whichPt = -1;
			
			for(int k = 0; k < l.size(); k++){
				float ldist = (l[k] - p).length();
				
				if( ldist < dist ){
					dist = ldist; 
					whichPt = k; 
				}
			}
		
			return whichPt;
		}


		static int findFurthestPointFromPoint( ofPolyline & l, ofPoint p ){
			
			float dist = 0.0; 
			int whichPt = -1;
			
			for(int k = 0; k < l.size(); k++){
				float ldist = (l[k] - p).length();
				
				if( ldist > dist ){
					dist = ldist; 
					whichPt = k; 
				}
			}
		
			return whichPt;
		}

		static vector <int> getPeaks( ofPolyline & l, float angleThresh, int numToLook, bool bInvert, bool bGroup, int numToSkip = 0 ){
			vector <int> results; 			
			if( l.size() >= 3 ){
			
				for(int k = 0; k < l.size(); k++){ 
				
					ofPoint normal; 
					bool bPeak = true;
					
					for(int j = 1; j < numToLook; j++){
						int L = k - j; 
						int R = k + j; 
						
						int Lp = L+1;
						int Rp = R-1;
						
						if( L < 0 ) L += l.size(); 
						if( R >= l.size() ) R -= l.size(); 
						if( Lp < 0 ) Lp += l.size(); 
						if( Rp >= l.size() ) Rp -= l.size(); 
						if( Rp < 0 ) Rp += l.size(); 
						if( Lp >= l.size() ) Lp -= l.size(); 
						
						if( j == 1 ){
							ofPoint delta = l[R] - l[L]; 
							normal.set(-delta.y, delta.x);
							if( bInvert ){
								normal *= -1.0;
							}
						}
						
						ofPoint vecL = l[L] - l[Lp];
						ofPoint vecR = l[R] - l[Rp];

						float angle1 = normal.angle(vecL);						
						float angle2 = normal.angle(vecR);
						
						//cout << " normal is " << normal << " vecL " << vecL << " angle1 = " << angle1 << "    vecR is " << vecR << " angle 2 = " << angle2 << endl; 
						
						if( fabs(angle1) < 90+angleThresh || fabs(angle2) < 90+angleThresh ){
							bPeak = false;
							break;
						}
						
					}
						
					if(  bPeak )results.push_back(k);
				}
			}
			
			if( results.size() && bGroup ){
			
				bool bCollecting = false;
				
				int findGoodStart = 0;
				for(int k = 0; k < l.size(); k++){ 
					bool bFound = false; 
					for(int j = 0; j < results.size(); j++){
						if( results[j] == k ){
							bFound = true; 
							break; 
						}
					}
					if( bFound == false){
						findGoodStart = k;
						break; 
					}
				}
				
				vector <int> newResults; 
				vector <int> curCollection;
				
				int notFoundCount = 0;
				
				for(int k = 0; k < l.size(); k++){ 
					int d = k + findGoodStart; 
					if( d >= l.size() ) d -= l.size(); 
					
					bool bFound = false; 
					for(int j = 0; j < results.size(); j++){
						if( results[j] == d ){
							bFound = true; 
							//TODO: erase from here 
							break; 
						}
					}
					
					if( bFound && bCollecting == false ){
						bCollecting = true;
						curCollection.clear();
						notFoundCount = 0;
					}
					
					if( bCollecting ){
						if( bFound ){
							curCollection.push_back(d); 
						}
						if( !bFound ){
							notFoundCount++;
						}
						if( ( !bFound && notFoundCount > numToSkip ) || k == l.size()-1 ){
							if( curCollection.size() == 1 ){
								newResults.push_back(curCollection[0]); 
							}else if( curCollection.size() > 1 ){
								int res = curCollection[curCollection.size()/2]; 
								newResults.push_back(res); 
							}
							bCollecting = false;
						}
					}
					
				}
				
				results = newResults;
				
			}
			
			return results; 
		}
		
		static vector <int> getPointsMatchingAngleChange( ofPolyline & l, float angleThresh ){
			vector <int> results; 
			if( l.size() >= 3 ){
			
				ofPoint pre, cur, nex, nexnex1, nexnex2;
				ofPoint vec, preVec;
				ofPoint nextVec;
				
				float preAngle = 0.0;
				float curAngle = 0.0;
				float nexAngle = 0.0;
				
				for(int k = 0; k < l.size(); k++){ 
					
					int c = k;
					int p = k-1;
					int n = k+1;
					int nn = k+3;
					int nnn = k+5;
					
					if( p < 0 ) p += l.size(); 
					if( n >= l.size() ) n -= l.size(); 
					if( nn >= l.size() ) nn -= l.size(); 
					if( nnn >= l.size() ) nnn -= l.size(); 
					
					pre		= l[p];
					cur		= l[k];
					nex		= l[n];
					nexnex1  = l[nn];
					nexnex2  = l[nnn];
					
					if( k == 0 ){
						ofPoint prepre = cur - l[p-1];
						preAngle = RAD_TO_DEG * atan2(prepre.y, prepre.x); 
						curAngle = preAngle; 
						nexAngle = preAngle;
					}
					
					preVec  = cur - pre;
					vec		= nex - pre;
					nextVec = nexnex2 - nexnex1; 					
					
					curAngle *= 0.4;
					nexAngle *= 0.4;
					preAngle *= 0.4;

					preAngle += 0.6 * (360 + RAD_TO_DEG * atan2(preVec.y, preVec.x)); 					
					curAngle += 0.6 * (360 + RAD_TO_DEG * atan2(vec.y, vec.x)); 
					nexAngle += 0.6 * (360 + RAD_TO_DEG * atan2(nextVec.y, nextVec.x)); 
				
					float diff		= ofAngleDifferenceDegrees(curAngle, preAngle);
					float nextDiff	= ofAngleDifferenceDegrees(nexAngle, curAngle);

					if( diff > angleThresh && nextDiff < angleThresh*0.8 ){
						//cout << " curAngle is " << curAngle << " nexAngle " << nexAngle << " diff is " << diff << " nextDiff " << nextDiff << endl; 
						results.push_back(k); 
					}
					
					preAngle = curAngle; 
				}
			}
			
			return results;
		}
		

		static vector <int> getPointsMatchingAngleChangeRev( ofPolyline & l, float angleThresh ){
			vector <int> results; 
			if( l.size() >= 3 ){
			
				ofPoint pre, cur, nex, nexnex1, nexnex2;
				ofPoint vec, preVec;
				ofPoint nextVec;
				
				float preAngle = 0.0;
				float curAngle = 0.0;
				float nexAngle = 0.0;
				
				for(int k = 0; k < l.size(); k++){ 
					
					int c = k;
					int p = k-1;
					int n = k+1;
					int nn = k+3;
					int nnn = k+5;
					
					if( p < 0 ) p += l.size(); 
					if( n >= l.size() ) n -= l.size(); 
					if( nn >= l.size() ) nn -= l.size(); 
					if( nnn >= l.size() ) nnn -= l.size(); 
					
					pre		= l[p];
					cur		= l[k];
					nex		= l[n];
					nexnex1  = l[nn];
					nexnex2  = l[nnn];
					
					if( k == 0 ){
						ofPoint prepre = cur - l[p-1];
						preAngle = RAD_TO_DEG * atan2(prepre.y, prepre.x); 
						curAngle = preAngle; 
						nexAngle = preAngle;
					}
					
					preVec  = cur - pre;
					vec		= nex - pre;
					nextVec = nexnex2 - nexnex1; 					
					
					curAngle *= 0.4;
					nexAngle *= 0.4;
					preAngle *= 0.4;

					preAngle += 0.6 * (360 + RAD_TO_DEG * atan2(preVec.y, preVec.x)); 					
					curAngle += 0.6 * (360 + RAD_TO_DEG * atan2(vec.y, vec.x)); 
					nexAngle += 0.6 * (360 + RAD_TO_DEG * atan2(nextVec.y, nextVec.x)); 
				
					float diff		= ofAngleDifferenceDegrees(curAngle, preAngle);
					float nextDiff	= ofAngleDifferenceDegrees(nexAngle, curAngle);

					if( diff < angleThresh && nextDiff > angleThresh ){
						//cout << " curAngle is " << curAngle << " nexAngle " << nexAngle << " diff is " << diff << " nextDiff " << nextDiff << endl; 
						results.push_back(k); 
					}
					
				}
			}
			
			return results;
		}
		


		static float getPercentInPoly( ofPolyline & l, ofRectangle rect ){
			if( l.size() == 0 ){
				return 0.0;
			}
			
			int totalInPoly = 0; 
			
			for(int k = 0; k < l.size(); k++){
				if( rect.inside(l[k]) ){
					totalInPoly ++;
				}
			}
			
			return (float)totalInPoly / (float)l.size();
			
		}
		
		static int getBiggestAngleJoint( ofPolyline & line ){
			float biggestAngle = 0.0f;
			int whichSmallest	= 0;
			for(int j = 1; j < MIN(5, line.size()); j++){
				if( j < line.size()-1 ){
					ofPoint d2 = line[j+1] -  line[j];
					ofPoint d1 = line[j]   -  line[j-1];
					
					float ang = d2.angle(d1);
					if( ang > biggestAngle ){
						biggestAngle = ang;
						whichSmallest = j;
					}
				}
			}
			
			return whichSmallest;
		} 
		
		static float getLineLength( ofPolyline & l ){
			if( l.size() <= 1 ){
				return 0.0;
			}
			
			float dist = 0.0; 			
			for(int k = 1; k < l.size(); k++){
				float ldist = (l[k] - l[k-1]).length();
				dist += ldist;
			}
			
			return dist; 
		}
		
		static int getPointAtPercent( ofPolyline & l, float pct ){
			if( l.size() <= 1 ){
				return -1;
			}
		
			float totalDist = getLineLength(l) * pct;
			
			float dist = 0.0; 			
			for(int k = 1; k < l.size(); k++){
				float ldist = (l[k] - l[k-1]).length();
				dist += ldist;
				if( dist > totalDist ){
					return k;
				}
			}
			
			return l.size()-1; 
		}		
		
		static int findFurthestPoint(ofPolyline & l, ofPoint relativeFrom){
			int startPt = findClosestPoint(l, relativeFrom);
			if( startPt < 0 )return -1;
			
			return (startPt + l.size()/2) % l.size(); 
			
//			float totalLineLength = getLineLength(l);
//			float dist = 0.0; 			
//			for(int i = 0; i < l.size(); i++){
//			
//				int cur = startPt + i; 
//				int pre = cur - 1; 
//				if( cur < 0 ) cur += l.size(); 
//			
//				float ldist = (l[cur] - l[pre]).length();
//				dist += ldist;
//				if( dist >= totalLineLength){
//					return cur; 
//				}
//			}

		}


};