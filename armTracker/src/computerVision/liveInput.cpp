#include "liveInput.h"

//------------------------------------------------------------------------------------------------
liveInput::liveInput(){
	nearThreshold = 200;
	farThreshold  = 20;
	threshold	  = 17;
	minMaskY	  = 0;
	maxMaskY	  = 60000;
	minMaskX	  = 0;
	maxMaskX	  = 60000;
	doWarp		  = false;
	bgMode		  = BG_MODE_ABS;
	bSmooth		  = false;
	timeToWaitTillNext = 0.0;
}

//call after you setup the mode
//------------------------------------------------------------------------------------------------
void liveInput::setupInputWarp(string xmlName, bool bColor, bool autoSave){
	warper.setup(xmlName, w, h, bColor, autoSave);
	doWarp = true;
}

//------------------------------------------------------------------------------------------------
void liveInput::setupKinect(int device, liveInputMode inputMode){
	
	kinect.init(true, false, false);
	kinect.setVerbose(true);
	kinect.open(device);
	
	w = 640;
	h = 480;
	
//	farThreshold	= initialFar;
//	nearThreshold = initialNear;
	
	this->inputMode = inputMode;
	inputType = LIVE_TYPE_KINECT;
	
	setupMode();
}


//------------------------------------------------------------------------------------------------
void liveInput::setupMode(){
	color.allocate(w, h);
	grayscale.allocate(w, h);
	inter.allocate(w,h);
	preview.allocate(w, h);
	
	if( inputMode == LIVE_MODE_DIFF || inputMode == LIVE_MODE_BG_SUB || inputMode == LIVE_MODE_KINECT || inputMode == LIVE_MODE_KINECT_DEPTH ){
		grayscalePre.allocate(w, h);
		grayscaleCur.allocate(w, h);
		
		if( inputMode == LIVE_MODE_KINECT_DEPTH ){
			extra1.allocate(w/8, h/8);	
			//extra2.allocate(w, h);	
		}
		
		if( inputType == LIVE_TYPE_KINECT && inputMode == LIVE_MODE_BG_SUB ){
			bgFloat.allocate(w, h);
			bgFloat.setNativeScale(0, 255.0f);
			liveFloat.allocate(w, h);
			liveFloat.setNativeScale(0, 255.0f);
		}
		
	}

}


//------------------------------------------------------------------------------------------------
void liveInput::setupRegularCamera( int width, int height, liveInputMode inputMode, int deviceId){
	
	inputType = LIVE_TYPE_CAM;
	this->inputMode = inputMode;
	
	camera.setDeviceID(deviceId);
	camera.initGrabber(width, height);
		
	w = camera.getWidth();
	h = camera.getHeight();
	
	setupMode();	
}

//------------------------------------------------------------------------------------------------
void liveInput::setupVideo(string videoPath, liveInputMode inputMode){
	
	inputType = LIVE_TYPE_VIDEO;
	this->inputMode = inputMode;

	videoPlayer.loadMovie(videoPath);
	videoPlayer.play();
	
	w = videoPlayer.getWidth();
	h = videoPlayer.getHeight();

	setupMode();		
}

//------------------------------------------------------------------------------------------------
bool liveInput::update(bool bFlipHoriz, bool bFlipVert, bool bRecord){
	bool bFrameNew = false;
	
	if( inputType == LIVE_TYPE_CAM ){
		camera.update();
		if( camera.isFrameNew() ){
			color = camera.getPixels();		
			color.mirror(bFlipVert, bFlipHoriz);

			preview = color;
//			grayscaleCur = preview;				
			bFrameNew = true;			
		}
	}else if( inputType == LIVE_TYPE_KINECT ){
		kinect.update();
		if( kinect.isFrameNew() ){
			unsigned char * pix = kinect.getDepthPixels();
			
			if( bSmooth ){
				inter = pix;
				inter.mirror(bFlipVert, bFlipHoriz);
				
				unsigned char * src = inter.getPixels();
				unsigned char * dst = preview.getPixels();
				
				int num = w * h;
				float v;
				for(int k = 0; k < num; k++){
					v = dst[k];
					v *= 0.3;
					v += src[k]*0.7;
					if( v > 255 ) v = 255;
					
					dst[k] = (int)v;
				} 
				
				preview = dst;
				
			}else{
				preview = pix;
				preview.mirror(bFlipVert, bFlipHoriz);							
			}
			
			bFrameNew = true;			
		}
	}
	else if( inputType == LIVE_TYPE_VIDEO ){
	
		if( ofGetKeyPressed('p') ){
			videoPlayer.setSpeed(0.001);
		}else if( ofGetKeyPressed('g') ){
			videoPlayer.setSpeed(1.0);
		}else if( ofGetKeyPressed('l') ){
			videoPlayer.setSpeed(0.1);
		}
	
		videoPlayer.update();
		bFrameNew = videoPlayer.isFrameNew();
		
		if( videoPlayer.getSpeed() < 0.1 ){
			bFrameNew = true;
		}
		
		if( bFrameNew ){
			color = videoPlayer.getPixels();
			color.mirror(bFlipVert, bFlipHoriz);	
			
			preview = color;			
//			grayscaleCur = preview;				
			bFrameNew = true;			
		}
		
		
	}
	
	if( bFrameNew ){
		if( doWarp ){
			warper.updatePixels(preview);
			grayscaleCur = warper.grayOut;
		}else{
			grayscaleCur = preview;
		}
	}
	
	if( bFrameNew ){
	
		if( bRecord ){
			
			if( preview.getWidth() > 0 ){
				if( recImg.getWidth() == 0 ){
					recImg.allocate(preview.getWidth(), preview.getHeight(), OF_IMAGE_GRAYSCALE);
				}
				
				if( sessionStr == "" ){
					sessionStr = "recordings/rec-"+ofGetTimestampString() + "/";
					
					if( !ofDirectory::doesDirectoryExist(sessionStr) ){
						ofDirectory::createDirectory( sessionStr, true, true);
					}
				}				
				recImg.setFromPixels(preview.getPixels(), preview.getWidth(), preview.getHeight(), OF_IMAGE_GRAYSCALE);
				recImg.saveImage(sessionStr + "f-"+ofToString(1000000+ofGetFrameNum()) + ".jpg", OF_IMAGE_QUALITY_BEST);
			}
		}else{
			sessionStr = "";
		}
	
		if( inputMode == LIVE_MODE_DIFF ){			

			if( inputType != LIVE_TYPE_KINECT ){
				grayscalePre	= grayscaleCur;
				grayscale.absDiff(grayscalePre, grayscaleCur);
				grayscale.threshold(threshold, false);

			}else{
					unsigned char * pix = grayscaleCur.getPixels();
					int numPixels = grayscaleCur.getWidth() * grayscaleCur.getHeight();

					int x = 0;
					int y = 0;
					for(int i = 0; i < numPixels; i++){
						if( x >= w ){
							x = 0;
							y++;
						}
						if( pix[i] < nearThreshold && pix[i] > farThreshold && y > minMaskY && y < maxMaskY ){
							pix[i] = 255;
						}else{
							pix[i] = 0;
						}
						x++;
					}
					
					grayscaleCur = pix;
					grayscale.absDiff(grayscalePre, grayscaleCur);
					grayscalePre = pix;
					
			}
		}
		else if( inputMode == LIVE_MODE_KINECT_DEPTH ){			

			if( inputType == LIVE_TYPE_KINECT || inputType == LIVE_TYPE_VIDEO ){
					
					grayscaleCur.threshold(3, false);
					
					grayscaleCur.setROI(ofRectangle(0, 0, 630, 480));

					extra1.scaleIntoMe(grayscaleCur, CV_INTER_LINEAR);	
					grayscaleCur.resetROI();
					extra1.blur(17);

										
			}
			
		}else if( inputMode == LIVE_MODE_BG_SUB ){			

			if( inputType != LIVE_TYPE_KINECT ){
								
				if( bgMode == BG_MODE_ABS ){
					grayscale.absDiff(grayscalePre, grayscaleCur);
				}else if( bgMode == BG_MODE_LIGHTER ){
					grayscale = grayscaleCur;				
					grayscale -= grayscalePre;
				}else if( bgMode == BG_MODE_DARKER ){
					grayscale = grayscalePre;
					grayscale -= grayscaleCur;				
				}
				grayscale.threshold(threshold, false);

			}else{
										
					float * fp = bgFloat.getPixelsAsFloats();
					float * lv = liveFloat.getPixelsAsFloats();
																
					unsigned char * pix = grayscaleCur.getPixels();
					int numPixels = grayscaleCur.getWidth() * grayscaleCur.getHeight();

					int x = 0;
					int y = 0;
					
					float curPix;
					unsigned char curBg;
					
					for(int i = 0; i < numPixels; i++){
						if( x >= w ){
							x = 0;
							y++;
						}
						
						curPix = (float)pix[i];
														
						if( fp[i] > curPix && curPix > 0 ){
							fp[i] *= 0.99;
							fp[i] += curPix * 0.01;
						}else if( curPix > 0 ){
							fp[i] *= 0.9996;
							fp[i] += curPix * 0.0004;						
						}						
						
						if( fp[i] < 0.01 ){
							fp[i] = 0.0;
						}
												
						curBg = fp[i];
						
						if( curPix < nearThreshold && curPix > farThreshold && y > minMaskY && y < maxMaskY ){
							if( curPix - curBg > threshold ){
								pix[i] = ofMap(curPix - curBg, threshold, threshold * 2, 140, 255, true) ;
							}							
							else pix[i] = 0;
						}else{
							pix[i] = 0;
						}
						x++;
					}
					
					bgFloat.flagImageChanged();
					
					grayscaleCur = pix;
					grayscaleCur.blur(7);
					grayscale = grayscaleCur;
					grayscale.threshold(128, false);
					grayscale.erode();
					grayscale.erode();
			}
		}
	}
	
	return 	bFrameNew;

}

		


