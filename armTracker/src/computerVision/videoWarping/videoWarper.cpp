#include "videoWarper.h"

//------------------------------------------------
videoWarper::videoWarper(){
	bColor			= false;
	autoSaveXML		= false;
	dstW			= 320;
	dstH			= 240;

	whichSelectedQuad = 0;
}


void videoWarper::setIncrementSave(string incrmentalFileBaseName){
    incrementSaveName = incrmentalFileBaseName;
}

//------------------------------------------------
void videoWarper::setup(string uniqueXMLName, int dstWidth, int dstHeight, bool isColor, bool autoSave){
	bColor		= isColor;
	autoSaveXML = autoSave;
	xmlName		= uniqueXMLName;

    settingsDirectory = xmlName;
    int posLastSlash = settingsDirectory.rfind("/");
    if( posLastSlash > 0) settingsDirectory.erase(settingsDirectory.begin()+ posLastSlash+1, settingsDirectory.end()  );
    else settingsDirectory = "";

	dstW = dstWidth;
	dstH = dstHeight;

	if( bColor ){
		colorOut.allocate(dstWidth, dstHeight);
		colorIn.allocate(dstWidth, dstHeight);
	}else {
		grayOut.allocate(dstWidth, dstHeight);
		grayIn.allocate(dstWidth, dstHeight);
	}

	quadSrc[0].set(0, 0);
	quadSrc[1].set(dstWidth, 0);
	quadSrc[2].set(dstWidth, dstHeight);
	quadSrc[3].set(0, dstHeight);

	quadDst[0].set(0, 0);
	quadDst[1].set(dstWidth, 0);
	quadDst[2].set(dstWidth, dstHeight);
	quadDst[3].set(0, dstHeight);

	setQuadPoints(quadSrc);
	readFromFile(xmlName);

}

//------------------------------------------------
void videoWarper::saveSettings(){

    if( incrementSaveName!= "" && settingsDirectory != "" ){
        string xmlSaveName = incrementSaveName;
        xmlSaveName += ofToString(ofGetYear()) +"-"+ ofToString(ofGetMonth()) +"-"+ ofToString(ofGetDay()) +"-"+ ofToString(ofGetHours()) +"-"+ ofToString(ofGetMinutes())+"-"+ ofToString(ofGetSeconds());
        xmlSaveName += ".xml";
        xmlSaveName = settingsDirectory + xmlSaveName;
        saveToFile(xmlSaveName);
    }

    saveToFile(xmlName);
}

//------------------------------------------------
void videoWarper::updateWarp(){
	ofPoint * warpedPts = getScaledQuadPoints(dstW, dstH);
	for(int i = 0; i < 4; i++){
		quadSrc[i] = warpedPts[i];
	}
	if(bColor){
		colorOut.warpIntoMe(colorIn, quadSrc, quadDst);
	}else {
		grayOut.warpIntoMe(grayIn, quadSrc, quadDst);
	}
}

//------------------------------------------------
void videoWarper::updatePixels(ofxCvColorImage & cvImageIn){
	if( bColor ){
		colorIn = cvImageIn;
		updateWarp();
	}
}

//------------------------------------------------
void videoWarper::updatePixels(ofxCvGrayscaleImage & cvImageIn){
	if( !bColor ){
		grayIn = cvImageIn;
		updateWarp();
	}
}

//------------------------------------------------
void videoWarper::updatePixels(unsigned char * pixels){
	if( !bColor ){
		grayIn.setFromPixels(pixels, grayIn.width, grayIn.height);
	}else{
		colorIn.setFromPixels(pixels, colorIn.width, colorIn.height);
	}
	updateWarp();
}

//------------------------------------------------
void videoWarper::setWarpGui(float x, float y, float w, float h){
    drawPositions.x			= x;
    drawPositions.y			= y;
    drawPositions.width		= w;
    drawPositions.height	= h;
}


//------------------------------------------------
void videoWarper::drawInput(){
	ofSetColor(255, 255, 255);

    if( bColor)colorIn.draw(drawPositions.x, drawPositions.y, drawPositions.width, drawPositions.height);
    else grayIn.draw(drawPositions.x, drawPositions.y, drawPositions.width, drawPositions.height);

    guiQuad::draw(drawPositions.x, drawPositions.y, drawPositions.width, drawPositions.height, 255, 255, 0, 1);

}

//------------------------------------------------
void videoWarper::drawOutput(int x, int y, int w, int h){
	ofSetColor(255, 255, 255);
	if( bColor ){
		colorOut.draw(x, y, w, h);
	}
	else{
		grayOut.draw(x, y, w, h);
	}
}

//------------------------------------------------
void videoWarper::mousePressed(float x, float y){
	cout << " x is y is " << x << " " << y << endl; 
	cout << " drawpos is " << drawPositions.x << " , " << drawPositions.y << endl; 
	
    if( selectPoint(x, y, drawPositions.x, drawPositions.y, drawPositions.width, drawPositions.height, 20) ){
        whichSelectedQuad = 1;
    }
}

//------------------------------------------------
void videoWarper::mouseDragged(float x, float y){
    if(whichSelectedQuad ){
        updatePoint(x, y, drawPositions.x, drawPositions.y, drawPositions.width, drawPositions.height);
        updateWarp();
    }
}

//------------------------------------------------
void videoWarper::mouseRelease(){
	if( autoSaveXML ){
		saveToFile(xmlName);
	}
	whichSelectedQuad = 0;
}
