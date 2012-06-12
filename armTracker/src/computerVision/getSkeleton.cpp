#include "getSkeleton.h"

IplImage *distsrc;
IplImage *out;
IplImage *S00;
IplImage *S45;
IplImage *S90;
IplImage *S135;

static bool bInit = false;

CvMat kern00, kern45, kern90, kern135;
float Smax=0;

float L0[]={
  -1,-1,-1,-1,-1,
   0, 0, 0, 0, 0,
   2, 2, 2, 2, 2,
   0, 0, 0, 0, 0,
  -1,-1,-1,-1,-1
};
float L45[]={
   0,-1,-1, 0, 2,
  -1,-1, 0, 2, 0,
  -1, 0, 2, 0,-1,
   0, 2, 0,-1,-1,
   2, 0,-1,-1, 0
};
float L90[]={
  -1, 0, 2, 0,-1,
  -1, 0, 2, 0,-1,
  -1, 0, 2, 0,-1,
  -1, 0, 2, 0,-1,
  -1, 0, 2, 0,-1
};
float L135[]={
   2, 0,-1,-1, 0,
   0, 2, 0,-1,-1,
  -1, 0, 2, 0,-1,
  -1,-1, 0, 2, 0,
   0,-1,-1, 0, 2
};
	

//void getSkeleton::beginFromScreen(ofImage * img, int w, int h){
//	iPtr = img;
//	iPtr->allocate( w, h, OF_IMAGE_GRAYSCALE );
//}
//
//
//void getSkeleton::doFromImage(ofImage & inputImg, ofImage & outputImg){
//
////	iPtr->grabScreen(0, 0, width, height);
//
//	int width  = inputImg.getWidth();
//	int height = inputImg.getHeight();
//
//	ofxCvGrayscaleImage input;
//	input.allocate(width, height);
//	input.setFromPixels(inputImg.getPixels(), width, height);
//	input.flagImageChanged();
//
//	getSkeleton::doFromImage(input, outputImg);
//}


void getSkeleton::doFromImage(ofxCvGrayscaleImage & input, ofxCvGrayscaleImage & output, int threshVal, int thresholdAmnt2){

//	iPtr->grabScreen(0, 0, width, height);

	int width = input.getWidth();
	int height = input.getHeight();

	
	if( bInit == false ){
		distsrc=cvCreateImage(cvGetSize(input.getCvImage()),IPL_DEPTH_32F,1);
		S00=cvCreateImage(cvGetSize(input.getCvImage()),IPL_DEPTH_32F,1);
		S45=cvCreateImage(cvGetSize(input.getCvImage()),IPL_DEPTH_32F,1);
		S90=cvCreateImage(cvGetSize(input.getCvImage()),IPL_DEPTH_32F,1);
		S135=cvCreateImage(cvGetSize(input.getCvImage()),IPL_DEPTH_32F,1);
		out=cvCreateImage(cvGetSize(input.getCvImage()),IPL_DEPTH_32F,1);
		cvInitMatHeader(&kern00,5,5,CV_32FC1,L0);
		cvInitMatHeader(&kern45,5,5,CV_32FC1,L45);
		cvInitMatHeader(&kern90,5,5,CV_32FC1,L90);
		cvInitMatHeader(&kern135,5,5,CV_32FC1,L135);
		bInit = true;
	}
	
	
	cvDistTransform(input.getCvImage(),distsrc,CV_DIST_L2,5);
    //フィルタ処理
    cvFilter2D(distsrc,S00,&kern00);
    cvFilter2D(distsrc,S45,&kern45);
    cvFilter2D(distsrc,S90,&kern90);
    cvFilter2D(distsrc,S135,&kern135);
    //
    //Smax = MAX(S00,S45,S90,S135)
    //     / Smax, Smax >= 0
    // g = |
    //     \ 0 , others
    //
    for (int y=0; y < out->height; y++){
      for (int x=0; x< out->width; x++){
        Smax = MAX(MAX(((float*)(S00->imageData + y* S00->widthStep))[x], ((float*)(S45->imageData + y* S45->widthStep))[x]),
          MAX(((float*)(S90->imageData + y* S90->widthStep))[x], ((float*)(S135->imageData + y* S135->widthStep))[x]));
        ((float*)(out->imageData + y* out->widthStep))[x] = Smax > 0 ? Smax: 0.0;
      }
    }
	
    cvThreshold(out,out,threshVal,thresholdAmnt2,CV_THRESH_BINARY);
	
	
	
	float * pix = (float *)out->imageData;
	unsigned char * pixC = input.getPixels();
	
	for(int k = 0; k < width*height; k++){
		if( pix[k] > 254.0 / 255.0 ){
			pixC[k] = 255;
		}else{
			pixC[k] = 0;
		}
	}
	
	if( output.getWidth() == 0){
		output.allocate(width, height);
	}
	output = pixC;
	//output.dilate();
	
//	tmpImg.erode();
//	tmpImg.dilate();
	
//	tmpImg.dilate_3x3();
//	tmpImg.blurGaussian(17);
//	tmpImg.threshold(20, false);
//	tmpImg.dilate_3x3();
//	tmpImg.blurGaussian(17);
//	

}
//
//void getSkeleton::endFromScreen(){
//
//	ofImage tmp;
//	tmp.grabScreen(0, 0, iPtr->getWidth(), iPtr->getHeight());
//	tmp.setImageType(OF_IMAGE_GRAYSCALE);
//		
//	getSkeleton::doFromImage(tmp, *iPtr);
//}

//
//
////from
//// http://www.eml.ele.cst.nihon-u.ac.jp/~momma/wiki/wiki.cgi/OpenCV/Kasvandの反復型線検出オペレータ.html
//
//void contourAnalyzer::allocate(IplImage * src){
//
//}
//
//void contourAnalyzer::operate(IplImage * srcImg, ofxCvFloatImage & outImg){
//    
//	
//	cvDistTransform(srcImg,distsrc,CV_DIST_L2,5);
//    //フィルタ処理
//    cvFilter2D(distsrc,S00,&kern00);
//    cvFilter2D(distsrc,S45,&kern45);
//    cvFilter2D(distsrc,S90,&kern90);
//    cvFilter2D(distsrc,S135,&kern135);
//    //
//    //Smax = MAX(S00,S45,S90,S135)
//    //     / Smax, Smax >= 0
//    // g = |
//    //     \ 0 , others
//    //
//    for (int y=0; y < out->height; y++){
//      for (int x=0; x< out->width; x++){
//        Smax = MAX(MAX(((float*)(S00->imageData + y* S00->widthStep))[x], ((float*)(S45->imageData + y* S45->widthStep))[x]),
//          MAX(((float*)(S90->imageData + y* S90->widthStep))[x], ((float*)(S135->imageData + y* S135->widthStep))[x]));
//        ((float*)(out->imageData + y* out->widthStep))[x] = Smax > 0 ? Smax: 0.0;
//      }
//    }
//    cvThreshold(out,out,7,1,CV_THRESH_BINARY);
//	
//	outImg.setFromPixels((float *)out->imageData, 640, 480);
//	
	//cvCopy(out, outImg);
	//cvCvtScaleAbs(out,outImg,255,0);
//}
//
//
//
//				unsigned char * pix = grey.getPixels();
//				memset(pix, 0, 640*480);
//
//				if( contourFinder.blobs[0].pts.size() > 20 ){
//					CvPoint * ptsArr = new CvPoint[contourFinder.blobs[0].pts.size()];
//					int nPts = contourFinder.blobs[0].pts.size();
//					
//					for(int k = 0; k < nPts; k++){
//						ptsArr[k].x=contourFinder.blobs[0].pts[k].x;
//						ptsArr[k].y=contourFinder.blobs[0].pts[k].y;
//					}
//					
//					cvFillPoly(grey.getCvImage(), &ptsArr, &nPts, 1, cvScalarAll(255.0));
//					grey.flagImageChanged();
//					
//					delete[] ptsArr;
//				}else{
//					return;
//				}
//				
//				operate(grey.getCvImage(), grey3);
//				grey3.flagImageChanged();
//				grey = grey3;
//				grey.threshold(254, false);
//
//
//	
//				if( contourFinder.findContours(grey, 50, 100000000, 10, false, false) ){
//					for(int k = 0; k < contourFinder.blobs[0].pts.size(); k++){
//						if( k > 0 ){
//							contourFinder.blobs[0].pts.at(k) = contourFinder.blobs[0].pts.at(k) * 0.15 + contourFinder.blobs[0].pts.at(k-1) * 0.85;
//						}
//					}
//				}else{
//					bPerson = false;				
//					return;
//				}
//				
//				pts = contourFinder.blobs[0].pts;	
//				bounds = contourFinder.blobs[0].boundingRect;
//				
//				if( bounds.width < img.getWidth() / 4 ){
//					bPerson = false;
//					return;
//				}
//				
//				
				
				