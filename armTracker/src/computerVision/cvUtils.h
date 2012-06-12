#pragma once 
#include "ofxOpenCv.h"

namespace cvUtils{		

	static float getAveragePixelColor( ofxCvGrayscaleImage & img, ofPoint pos, float rad ){
		
		if( img.getWidth() == 0 )return 0.0;
		
		int pixCount = 0;
		unsigned char * pix = img.getPixels();
		
		int minX = pos.x - rad; 
		int minY = pos.y - rad; 
		
		minY = MIN(MAX(0, minY), img.getHeight()-1);
		int maxY = MIN(minY+rad*2, img.getHeight()-1);

		minX = MIN(MAX(0, minX), img.getWidth()-1);
		int maxX = MIN(minX+rad*2, img.getWidth()-1);

		int w = img.getWidth(); 			
		int h = img.getHeight();
		
		int total = w*h;
		
		float avgVal = 0.0;
		for(int y = minY; y < maxY; y++){
			for(int x = minX; x < maxX; x++){
				int i = x + y*w;
				if( i >= 0 && i < total ){
					if( pix[i] > 0 ){
						avgVal += (float)pix[i];
						pixCount++;
					}
				}
			}
		}
		
		if( pixCount > 0 ){
			avgVal /= (float)pixCount;
		}
		
		return avgVal;
		
	}

	static float getMaxPixelColor( ofxCvGrayscaleImage & img, ofPoint pos, float rad ){
		
		if( img.getWidth() == 0 )return 0.0;
		
		int pixCount = 0;
		unsigned char * pix = img.getPixels();
		
		int minX = pos.x - rad; 
		int minY = pos.y - rad; 
		
		minY = MIN(MAX(0, minY), img.getHeight()-1);
		int maxY = MIN(minY+rad*2, img.getHeight()-1);

		minX = MIN(MAX(0, minX), img.getWidth()-1);
		int maxX = MIN(minX+rad*2, img.getWidth()-1);

		int w = img.getWidth(); 			
		int h = img.getHeight();
		
		int total = w*h;
		
		float maxC = 0.0;
		for(int y = minY; y < maxY; y++){
			for(int x = minX; x < maxX; x++){
				int i = x + y*w;
				if( i >= 0 && i < total ){
					if( pix[i] > 0 && pix[i] > maxC ){
						maxC = pix[i];
					}
				}
			}
		}
		
		return maxC;
		
	}

};