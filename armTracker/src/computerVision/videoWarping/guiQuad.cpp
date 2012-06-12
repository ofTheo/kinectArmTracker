#include "guiQuad.h"

//----------------------------------------------------
guiQuad::guiQuad(){
	selected = -1;
	quadName = "QUAD_";
}

//----------------------------------------------------
void guiQuad::setup(string _quadName){
	quadName = _quadName;
}

//----------------------------------------------------
void guiQuad::releaseAllPoints(){
	selected = -1;
}

//----------------------------------------------------
//these should be in the range x(0-maxX) y(0-maxH)
//with 	width  = maxW
//		height = maxH
void guiQuad::setQuadPoints( ofPoint * inPts ){

	for(int i = 0; i < 4; i++){
		srcZeroToOne[i].x	= inPts[i].x;
		srcZeroToOne[i].y	= inPts[i].y;

		if(srcZeroToOne[i].x > 1) srcZeroToOne[i].x = 1;
		if(srcZeroToOne[i].y > 1) srcZeroToOne[i].y = 1;
	}
}

//----------------------------------------------------
void guiQuad::readFromFile(string filePath){

	xml.loadFile(filePath);
	loadSettings();
}

//----------------------------------------------------
void guiQuad::loadSettings(){

	string str;


	for(int i = 0; i < 4; i++){

		//These two lines give us:
		// "QUAD_SRC_0"
		// "QUAD_SRC_1" etc...

		str = quadName;
		str += ofToString(i);

		srcZeroToOne[i].x = xml.getValue(str+"_X", srcZeroToOne[i].x);
		srcZeroToOne[i].y = xml.getValue(str+"_Y", srcZeroToOne[i].y);

		if(srcZeroToOne[i].x > 1) srcZeroToOne[i].x = 1;
		if(srcZeroToOne[i].y > 1) srcZeroToOne[i].y = 1;

	}

}

//----------------------------------------------------
bool guiQuad::selectPoint(float x, float y, float offsetX, float offsetY, float width, float height, float hitArea){

	//make sure selected is -1 unless we really find a point
	selected = -1;

	if(width == 0 || height == 0 || x < offsetX || x > offsetX + width || y < offsetY ||  y > offsetY + height){
		//then we are out of our possible quad area
		//so we ignore :)
		return false;
	}

	//lets get it in range x(0 - width) y(0 - height)
	float px = x - offsetX;
	float py = y - offsetY;

	//now get in 0-1 range
	px /= width;
	py /= height;

	hitArea /= width;

	//we want to store the smallest distance found
	//because in the case when two points are in the
	//hit area we want to pick the closet
	float storeDist = 9999999.0;

	for(int i = 0; i < 4; i++){
		float dx = fabs(px -  srcZeroToOne[i].x);
		float dy = fabs(py -  srcZeroToOne[i].y);

		float dist = sqrt(dx*dx + dy*dy);

		if(dist > hitArea)continue;

		if(dist < storeDist){
			selected = i;
			storeDist = dist;
		}
	}

	if(selected != -1){
		srcZeroToOne[selected].x 	= px;
		srcZeroToOne[selected].y 	= py;
		srcScaled[selected].x		= px;
		srcScaled[selected].y		= py;

		return true;
	}

	return false;
}

//----------------------------------------------------
bool guiQuad::updatePoint(float x, float y, float offsetX, float offsetY, float width, float height){

	//nothing to update
	if(selected == -1) return false;

	if(width == 0 || height == 0){
		//dangerous so we ignore :)
		return false;
	}

	if( x < offsetX ) 			x = offsetX;
	if( x > offsetX + width ) 	x = offsetX + width;
	if( y < offsetY ) 			y = offsetY;
	if( y > offsetY + height) 	y = offsetY + height;

	//lets get it in range x(0 - width) y(0 - height)
	float px = x - offsetX;
	float py = y - offsetY;

	//now get in 0-1 range
	px /= width;
	py /= height;

	srcZeroToOne[selected].x 	= px;
	srcZeroToOne[selected].y 	= py;
	srcScaled[selected].x		= px;
	srcScaled[selected].y		= py;

	return true;
}

//----------------------------------------------------
//returns pts to width by height range
ofPoint * guiQuad::getScaledQuadPoints(float width, float height){

	for(int i = 0; i < 4; i++){
		srcScaled[i].x = srcZeroToOne[i].x * width;
		srcScaled[i].y = srcZeroToOne[i].y * height;
	}

	return srcScaled;
}


//----------------------------------------------------
//returns pts in 0-1 scale
ofPoint * guiQuad::getQuadPoints(){
	return srcZeroToOne;
}


//----------------------------------------------------
void guiQuad::saveToFile(string filePath, string newQuadName){
	string str;

	for(int i = 0; i < 4; i++){

		str = newQuadName;
		str += ofToString(i);

		xml.setValue(str+"_X", srcZeroToOne[i].x);
		xml.setValue(str+"_Y", srcZeroToOne[i].y);
	}

	xml.saveFile(filePath);
}

//----------------------------------------------------
void guiQuad::saveToFile(string filePath){
	saveToFile(filePath, quadName);
	printf("guiQuad - saving xml for quad\n");

}

//----------------------------------------------------
void guiQuad::draw(float x, float y, float width, float height, int red, int green, int blue, int thickness){
	ofPushStyle();

	getScaledQuadPoints(width, height);
	glPushMatrix();
		glTranslatef(x, y, 0);

		ofNoFill();

		ofSetColor(red, green, blue);
		glLineWidth(thickness);
		glBegin(GL_LINE_LOOP);
		for(int i = 0; i < 4; i++){
			glVertex2f(srcScaled[i].x, srcScaled[i].y);
		}
		glEnd();

		glLineWidth(1);
		ofSetRectMode(OF_RECTMODE_CENTER);
		for(int i = 0; i < 4; i++){

			if(i == 0)ofSetColor(255, 0, 0);
			if(i == 1)ofSetColor(0, 255, 0);
			if(i == 2)ofSetColor(0, 0, 255);
			if(i == 3)ofSetColor(0, 255, 255);

			ofRect(srcScaled[i].x, srcScaled[i].y, 8, 8);
		}
		ofSetRectMode(OF_RECTMODE_CORNER);
		ofFill();
	glPopMatrix();

	ofPopStyle();
}

//----------------------------------------------------
void guiQuad::draw(float x, float y, float width, float height){

	//default to a think yellow line
	draw(x, y, width, height, 255, 255, 0, 1);
}

