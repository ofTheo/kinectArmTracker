#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class guiQuad{

	public:
		guiQuad();

		void setup(string _quadName);
		void readFromFile(string filePath);
		void loadSettings();

		void releaseAllPoints();
		//these should be in the range x(0-maxX) y(0-maxH)
		//with 	width  = maxW
		//		height = maxH
		void setQuadPoints( ofPoint * inPts );

		bool selectPoint(float x, float y, float offsetX, float offsetY, float width, float height, float hitArea);
		bool updatePoint(float x, float y, float offsetX, float offsetY, float width, float height);

		//returns pts to width by height range
		ofPoint * getScaledQuadPoints(float width, float height);
		//returns pts in 0-1 scale
		ofPoint * getQuadPoints();
		void saveToFile(string filePath, string newQuadName);
		void saveToFile(string filePath);
		void draw(float x, float y, float width, float height, int red, int green, int blue, int thickness);
		void draw(float x, float y, float width, float height);


	protected:
		ofxXmlSettings	xml;
		ofPoint srcZeroToOne[4];
		ofPoint srcScaled[4];
		string quadName;
		int selected;
};




