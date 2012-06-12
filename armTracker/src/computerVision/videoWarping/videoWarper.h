#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"
#include "guiQuad.h"

#include "guiCustomImpl.h"


class videoWarper : public guiQuad, public guiCustomImpl{

	public:
		videoWarper();
        void setIncrementSave(string incrmentalFileBaseName);

		void setup(string uniqueXMLName, int dstWidth, int dstHeight, bool isColor, bool autoSave = false);
		void updatePixels(ofxCvGrayscaleImage &cvImageIn);
		void updatePixels(ofxCvColorImage &cvImageIn);
		void updatePixels(unsigned char * pixels);

		void setWarpGui(float x, float y, float w, float h);

		void reloadSettings(){
			guiQuad::loadSettings();
		}
        void saveSettings();

		void drawInput();
		void drawOutput(int x, int y, int w, int h);

		void mousePressed(float x, float y);
		void mouseDragged(float x, float y);
		void mouseRelease();

		void update(){}
		void draw(float x, float y, float w, float h){
			setWarpGui(x, y, w, h);
			drawInput();
		}


		ofRectangle drawPositions;

		bool bColor;

		ofPoint quadSrc[4];
		ofPoint quadDst[4];

		float dstW, dstH;

		int whichSelectedQuad;

		string xmlName;
		bool autoSaveXML;

        string settingsDirectory;
        string incrementSaveName;

		ofxCvGrayscaleImage grayOut;
		ofxCvColorImage colorOut;

		ofxCvGrayscaleImage grayIn;
		ofxCvColorImage colorIn;

		protected:
			void updateWarp();

};


