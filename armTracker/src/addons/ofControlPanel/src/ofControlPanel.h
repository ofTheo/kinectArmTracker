#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "guiIncludes.h"


class xmlAssociation{
    public:
        xmlAssociation(guiBaseObject * objPtr, string xmlNameIn, int numParamsIn){
            guiObj      = objPtr;
            xmlName     = xmlNameIn;
            numParams   = numParamsIn;
        }

        guiBaseObject * guiObj;
        string xmlName;
        int numParams;
};

extern guiColor gTextColor;
extern guiColor gFgColor;
extern guiColor gBgColor;
extern guiColor gOutlineColor;

class ofControlPanel: public guiBaseObject{

	public:
	
        static float borderWidth;
        static float topSpacing;
        static float tabWidth;
        static float tabHeight;

        ofControlPanel();
        ~ofControlPanel();
		
		//set the control panel name
        void setup(string controlPanelName, float panelX = 0, float panelY = 0, float width = 1024, float height = 768, bool doSaveRestore = true, bool bPanelLocked = false);

		//add as many panels as you need
        guiTypePanel * addPanel(string panelName, int numColumns = 4, bool locked = false);
		
		//set which panel you are adding your object to. 
        void setWhichPanel(int whichPanel);
        void setWhichPanel(string panelName);
        void setWhichColumn(int column);		
		
		//add objects
		guiBaseObject * addGuiBaseObject(guiBaseObject * element, int varsToAddToXml); //set varsToAdd to 0 if you don't need xml saving
		
		//add objects - helpers
        guiTypeToggle * addToggle(string name, string xmlName, bool defaultValue);
        guiTypeMultiToggle * addMultiToggle(string name, string xmlName, int defaultBox, vector <string> boxNames);
        guiTypeSlider * addSlider(string sliderName, string xmlName, float value , float min, float max, bool isInt = false);
        guiType2DSlider * addSlider2D(string sliderName, string xmlName, float valueX, float valueY, float minX, float maxX, float minY, float maxY, bool isInt = false);
        guiTypeDrawable * addDrawableRect(string name, ofBaseDraws * drawablePtr, int drawW, int drawH);
		guiTypeVideo * addVideoRect(string name, ofVideoPlayer * drawablePtr, int drawW, int drawH);
        guiTypeCustom * addCustomRect(string name, guiCustomImpl * customPtr, int drawW, int drawH);
        guiTypeTextDropDown * addTextDropDown(string name, string xmlName, int defaultBox, vector <string> boxNames);
        guiTypeVairableLister * addVariableLister(string name, vector <guiVariablePointer> & varsIn);
		guiTypeChartPlotter * addChartPlotter(string name, guiStatVarPointer varPtr, float width, float height, int maxNum, float minYVal, float maxYVal);
//		guiTypeLogger * addLogger(string name, simpleLogger * logger, int drawW, int drawH);
//		guiTypeFileLister * addFileLister(string name, simpleFileLister * lister, int drawW, int drawH);
//		guiTypeLabel * addLabel( string text );
//		guiTypeTextInput* addTextInput( string name, string text, int width );

		// remove an object
		void removeObject( string xmlName );
		
		//setters and getters
		string getSelectedPanelName();		
		int getSelectedPanel();
		void setSelectedPanel(int whichPanel);
		int getWhichPanel() { return currentPanel; }
		int getNumPanels(){ return panels.size(); }
		
		//other 
		void loadFont( string fontName, int fontsize );
		void setSize( int newWidth, int newHeight );

		//layout
		void enableIgnoreLayoutFlag(){
			bIgnoreLayout = true;
		}
		void disableIgnoreLayoutFlag(){
			bIgnoreLayout = false;
		}
		
		
		//events
		void setupEvents();
		ofEvent <guiCallbackData> & createEventGroup(string eventGroupName, vector <string> xmlNames);
		ofEvent <guiCallbackData> & createEventGroup(string xmlName);

		void enableEvents();
		void disableEvents();
		ofEvent <guiCallbackData> & getEventsForPanel(int panelNo);
		ofEvent <guiCallbackData> & getAllEvents();
		ofEvent <guiCallbackData> & getEventGroup(string eventGroupName);		
		
        void setValueB(string xmlName, bool value,  int whichParam = 0);
        void setValueI(string xmlName, int value,  int whichParam = 0);
        void setValueF(string xmlName, float value,  int whichParam = 0);
        bool getValueB(string xmlName, int whichParam = 0);
        float getValueF(string xmlName, int whichParam = 0);
        int getValueI(string xmlName, int whichParam = 0);
		
		//check if a value has changed ( alternative to using events )
		bool hasValueChanged(string xmlName, int whichParam = 0);
		bool hasValueChangedInPanel(string whichPanel);
		bool newPanelSelected();		
			
		//clears all changed values
		void clearAllChanged();
		
        void setIncrementalSave(string incrmentalFileBaseName);
        void disableIncrementalSave();
        void loadSettings(string xmlFile);
        void reloadSettings();
        void saveSettings(string xmlFile, bool bUpdateXmlFile = true);
        void saveSettings();
        void setXMLFilename(string xmlFile);

        void setDraggable(bool bDrag);
        void setMinimized(bool bMinimize);
        void show();
        void hide();

        void toggleView();
        bool mousePressed(float x, float y, int button);
        bool mouseDragged(float x, float y, int button);
        void mouseReleased();
				
		/// return true if we consume the keypress
		bool keyPressed( int k );
		void keyReleased( int k );

        void updateBoundingBox();
        void update();
        void draw();
		
		void mousePressed(ofMouseEventArgs & args){
			if( fabs( ofGetFrameNum() - lastFrameDrawn ) <= 1 ){
				mousePressed(args.x, args.y, args.button);
			}
		}
        
		void mouseDragged(ofMouseEventArgs & args){
			if( fabs( ofGetFrameNum() - lastFrameDrawn ) <= 1 ){
				mouseDragged(args.x, args.y, args.button);		
			}
		}
		
        void mouseMoved(ofMouseEventArgs & args){
		
		}
        
		void mouseReleased(ofMouseEventArgs & args){
			mouseReleased();
		}
		
		void keyPressed(ofKeyEventArgs & args){
			if( fabs( ofGetFrameNum() - lastFrameDrawn ) <= 1 ){		
				keyPressed(args.key);
			}
		}
		
		void keyReleased(ofKeyEventArgs & args){
			if( fabs( ofGetFrameNum() - lastFrameDrawn ) <= 1 ){		
				keyReleased(args.key);
			}
		}
		
	protected:
        ofTrueTypeFont guiTTFFont;

        vector <xmlAssociation> xmlObjects;
        vector <guiBaseObject *> guiObjects;
        vector <guiTypePanel *> panels;
        vector <ofRectangle> panelTabs;
		
		vector <guiCustomEvent *> customEvents;

		ofxXmlSettings settings;
		string currentXmlFile;
		string settingsDirectory;

		ofRectangle topBar;
		ofRectangle minimizeButton;
		ofRectangle saveButton;
		ofRectangle restoreButton;
	
		void setLayoutFlag( guiBaseObject * obj ){
			obj->bRemoveFromLayout = bIgnoreLayout;
		}
				
		void eventsIn(guiCallbackData & data);

		static vector <ofControlPanel *> globalPanelList;
		static ofControlPanel * getPanelInstance(string panelName);

		void addXmlAssociation( guiBaseObject* object, string xmlName, int defaultValue );
		
		bool hidden;
		bool usingXml;
		bool bUseTTFFont;
		bool minimize;
		bool saveDown;
		bool incrementSave;
		bool restoreDown;
		bool bDraggable;
	
		bool bDoSaveRestore;

		int selectedPanel;
		int currentPanel;

		ofPoint prevMouse;

		int sliderWidth;
		int lastFrameDrawn;
		
		bool bEventsSetup;
		bool eventsEnabled;
		bool bIgnoreLayout;
		
		ofPoint mouseDownPoint;

		bool dragging;
		bool bNewPanelSelected;
		
		string incrementSaveName;

};
