#include "ofControlPanel.h"
#include "ofEvents.h"

float ofControlPanel::borderWidth = 5;
float ofControlPanel::topSpacing  = 15;
float ofControlPanel::tabWidth = 35;
float ofControlPanel::tabHeight = 10;

vector <ofControlPanel *> ofControlPanel::globalPanelList;

//----------------------------
ofControlPanel::ofControlPanel(){
    dragging        = false;
    minimize        = false;
    selectedPanel   = 0;
    currentPanel    = 0;
    bUseTTFFont     = false;
    usingXml        = true;
    saveDown        = false;
    restoreDown     = false;
    incrementSave   = false;
    hidden          = false;
    bDraggable      = true;
	eventsEnabled	= false;
	bEventsSetup	= false;
	bIgnoreLayout	= false;
	lastFrameDrawn	= 0;
	
    currentXmlFile = "";
    incrementSaveName = "";
    xmlObjects.clear();
	
	ofControlPanel::globalPanelList.push_back(this);
}

//-----------------------------
ofControlPanel::~ofControlPanel(){

    for(unsigned int i = 0; i < guiObjects.size(); i++){
        if( guiObjects[i] != NULL ){
			delete guiObjects[i];
			guiObjects[i] = NULL;
		}
    }
	guiObjects.clear();
	
	for(int i = 0; i < ofControlPanel::globalPanelList.size(); i++){
		if( ofControlPanel::globalPanelList[i] != NULL && ofControlPanel::globalPanelList[i]->name == name ){			
			ofControlPanel::globalPanelList.erase( ofControlPanel::globalPanelList.begin()+i, ofControlPanel::globalPanelList.begin()+i+1);
			break;
		}
	}
	
	for(int i = 0; i < customEvents.size(); i++){
		if( customEvents[i] != NULL ){
			delete customEvents[i];
			customEvents[i] = NULL;
		}
	}
	
	customEvents.clear();
}

//-----------------------------
ofControlPanel * ofControlPanel::getPanelInstance(string panelName){
	for(int i = 0; i < ofControlPanel::globalPanelList.size(); i++){
		if( ofControlPanel::globalPanelList[i] != NULL && ofControlPanel::globalPanelList[i]->name == panelName ){
			return ofControlPanel::globalPanelList[i];
		}
	}
	return NULL;
}	
		
//-----------------------------
void ofControlPanel::setup(string controlPanelName, float panelX, float panelY, float width, float height, bool doSaveRestore, bool locked){
			
	bDoSaveRestore = doSaveRestore;
	name = controlPanelName;

	setPosition(panelX, panelY);
	setDimensions(width, height);
	setShowText(true);

	fgColor			= gFgColor;
	outlineColor	= gOutlineColor;				
	bgColor			= gBgColor;
	textColor		= gTextColor;	 
	
	if( panels.size() == 0 ){
		addPanel(controlPanelName, 5, locked);
	} 

	ofAddListener(ofEvents().mousePressed, this, &ofControlPanel::mousePressed);
	ofAddListener(ofEvents().mouseReleased, this, &ofControlPanel::mouseReleased);
	ofAddListener(ofEvents().mouseDragged, this, &ofControlPanel::mouseDragged);
	ofAddListener(ofEvents().keyPressed, this, &ofControlPanel::keyPressed);
	ofAddListener(ofEvents().keyReleased, this, &ofControlPanel::keyReleased);
}

//-----------------------------
void ofControlPanel::loadFont(string fontName, int fontsize ){
    guiTTFFont.loadFont(fontName, fontsize);
    bool okay = guiTTFFont.isLoaded();
	guiBaseObject::setFont(&guiTTFFont);

    if(okay){
		printf("font loaded okay!\n");
        bUseTTFFont = true;
        for(unsigned int i = 0; i < guiObjects.size(); i++){
            guiObjects[i]->setFont(&guiTTFFont);
        }
    }else{
		printf("ahhhhhh why does my font no work!\n");
	}
}

//---------------------------------------------

void ofControlPanel::setSize( int new_width, int new_height )
{
	// set width
	setDimensions( new_width, new_height );
	// set underlying panel widths
	for ( int i=0; i<panels.size(); i++ )
	{
		panels[i]->setDimensions( (boundingBox.width - borderWidth*2) -1, boundingBox.height - topSpacing*3);
	}
}


//---------------------------------------------
guiTypePanel * ofControlPanel::addPanel(string panelName, int numColumns, bool locked){
    guiTypePanel * panelPtr = new guiTypePanel();
    panelPtr->setup(panelName);
    panelPtr->setPosition(borderWidth, topSpacing);
    panelPtr->setDimensions(boundingBox.width - borderWidth*2, boundingBox.height - topSpacing*3);
    if( locked )panelPtr->lock();
    else panelPtr->unlock();

    if( numColumns > 1 ){
        for(int i = 1; i < numColumns; i++){
            panelPtr->addColumn(30);
        }
    }

    panels.push_back(panelPtr);
    panelTabs.push_back(ofRectangle());

    guiObjects.push_back(panelPtr);
    if( bUseTTFFont ){
        panelPtr->setFont(&guiTTFFont);
    }

    return panelPtr;
}

// ############################################################## //
// ##
// ##      Control where things are added to
// ##
// ############################################################## //


//---------------------------------------------
void ofControlPanel::setWhichPanel(int whichPanel){
    if( whichPanel < 0 || whichPanel >= (int) panels.size() )return;
    currentPanel = whichPanel;
    setWhichColumn(0);
}

//---------------------------------------------
void ofControlPanel::setWhichPanel(string panelName){
    for(int i = 0; i < (int) panels.size(); i++){
        if( panels[i]->name == panelName){
            setWhichPanel(i);
            setWhichColumn(0);
            return;
        }
    }
}

//---------------------------------------------
void ofControlPanel::setWhichColumn(int column){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return;
    panels[currentPanel]->selectColumn(column);
}

//---------------------------------------------
int ofControlPanel::getSelectedPanel(){
	return selectedPanel;
}

//---------------------------------------------
void ofControlPanel::setSelectedPanel(int whichPanel){
	if( whichPanel >= 0 && whichPanel < panels.size()){
		selectedPanel = whichPanel;
	}
}

// ############################################################## //
// ##
// ##       Gui elements
// ##
// ############################################################## //

//---------------------------------------------
void ofControlPanel::addXmlAssociation( guiBaseObject* object, string xmlName, int paramCount ){
	// check for already-existing xml name
	for( int i=0; i<xmlObjects.size(); i++ ){
		if( xmlObjects[i].xmlName == xmlName ){
			ofLog(OF_LOG_ERROR, "ofControlPanel::add*: duplicate xmlName '%s', not adding\n", xmlName.c_str() );
			return;
		}
	}
	xmlObjects.push_back( xmlAssociation(object, xmlName, paramCount ) );
}

//---------------------------------------------
guiBaseObject * ofControlPanel::addGuiBaseObject( guiBaseObject * element,  int varsToAddToXml){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

	setLayoutFlag(element);

	if( varsToAddToXml > 0 ){
		addXmlAssociation(element, element->xmlName, varsToAddToXml);
    }
	
	guiObjects.push_back(element);

    if( bUseTTFFont ){
        element->setFont(&guiTTFFont);
    }

    panels[currentPanel]->addElement(element);
	return element;
}

//---------------------------------------------
guiTypeToggle * ofControlPanel::addToggle(string name, string xmlName, bool defaultValue){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    guiTypeToggle * tmp = new guiTypeToggle();
    tmp->setup(name, xmlName, (bool)defaultValue);

	addGuiBaseObject(tmp, 1);
    return tmp;
}

//---------------------------------------------
guiTypeMultiToggle * ofControlPanel::addMultiToggle(string name, string xmlName, int defaultBox, vector <string> boxNames){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    //add a new multi toggle to our list
    guiTypeMultiToggle * tmp = new guiTypeMultiToggle();
    tmp->setup(name, xmlName, defaultBox, boxNames);

	addGuiBaseObject(tmp, 1);
    return tmp;
}

//---------------------------------------------
guiTypeSlider * ofControlPanel::addSlider(string sliderName, string xmlName, float value , float min, float max, bool isInt){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    //add a new slider to our list
    guiTypeSlider * tmp = new guiTypeSlider();
    tmp->setup(sliderName, xmlName, value, min, max);
	
    //we can say we want to an int or a float!
    if(isInt){
        tmp->setTypeInt();
    }else{
        tmp->setTypeFloat();
    }

	addGuiBaseObject(tmp, 1);
    return tmp;
}

//-------------------------------
guiType2DSlider * ofControlPanel::addSlider2D(string sliderName, string xmlName, float valueX, float valueY, float minX, float maxX, float minY, float maxY, bool isInt)
{
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    //add a new slider to our list
    guiType2DSlider * tmp = new guiType2DSlider();
    tmp->setup(sliderName, xmlName, valueX, minX, maxX, valueY, minY, maxY);

    //we can say we want to an int or a float!
    if(isInt){
        tmp->setTypeInt();
    }else{
        tmp->setTypeFloat();
    }

	addGuiBaseObject(tmp, 2);
    return tmp;
}

//---------------------------------------------
guiTypeDrawable * ofControlPanel::addDrawableRect(string name, ofBaseDraws * drawablePtr, int drawW, int drawH){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    guiTypeDrawable * tmp = new guiTypeDrawable();
    tmp->setup(name, drawablePtr, drawW, drawH);

	addGuiBaseObject(tmp, 0);
    return tmp;
}

//---------------------------------------------
guiTypeVideo * ofControlPanel::addVideoRect(string name, ofVideoPlayer * drawablePtr, int drawW, int drawH){
    if( currentPanel < 0 || currentPanel >= panels.size() )return NULL;
 
	guiTypeVideo * tmp = new guiTypeVideo();
    tmp->setup(name, drawablePtr, drawW, drawH);

	addGuiBaseObject(tmp, 0);
    return tmp;
}


//---------------------------------------------
guiTypeCustom * ofControlPanel::addCustomRect(string name, guiCustomImpl * customPtr, int drawW, int drawH){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    guiTypeCustom * tmp = new guiTypeCustom();
    tmp->setup(name, customPtr, drawW, drawH);

	addGuiBaseObject(tmp, 0);
    return tmp;
}

//---------------------------------------------
guiTypeTextDropDown * ofControlPanel::addTextDropDown(string name, string xmlName, int defaultBox, vector <string> boxNames){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    //add a new multi toggle to our list
    guiTypeTextDropDown * tmp = new guiTypeTextDropDown();
    tmp->setup(name, xmlName, defaultBox, boxNames);

	addGuiBaseObject(tmp, 0);
    return tmp;
}

//---------------------------------------------
guiTypeVairableLister * ofControlPanel::addVariableLister(string name, vector <guiVariablePointer> & varsIn){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    //add a new multi toggle to our list
    guiTypeVairableLister * tmp = new guiTypeVairableLister();
    tmp->setup(name, varsIn);
	
	addGuiBaseObject(tmp, 0);
    return tmp;
}

//-----------------------------------------------------
guiTypeChartPlotter * ofControlPanel::addChartPlotter(string name, guiStatVarPointer varPtr, float width, float height, int maxNum, float minValY, float maxValY){
    if( currentPanel < 0 || currentPanel >= (int) panels.size() )return NULL;

    //add a new multi toggle to our list
    guiTypeChartPlotter * tmp = new guiTypeChartPlotter();
    tmp->setup(name, varPtr, width, height, maxNum, minValY, maxValY);

	addGuiBaseObject(tmp, 0);
    return tmp;
}

// ############################################################## //
// ##
// ##       events 
// ##
// ############################################################## //

//THIS SHOULD BE CALLED AFTER ALL GUI SETUP CALLS HAVE HAPPENED
//---------------------------------------------
void ofControlPanel::setupEvents(){
	eventsEnabled = true;
	for(int i = 0; i < guiObjects.size(); i++){
		ofAddListener(guiObjects[i]->guiEvent, this, &ofControlPanel::eventsIn);
	}
	
	//setup an event group for each panel
	for(int i = 0; i < panels.size(); i++){
	
		vector <string> xmlNames;
		
		for(int j = 0; j < panels[i]->children.size(); j++){
			xmlNames.push_back(panels[i]->children[j]->xmlName);
		}
		
		string groupName = "PANEL_EVENT_"+ofToString(i);
		createEventGroup(groupName, xmlNames);
		printf("creating %s\n", groupName.c_str());
	}
	
	bEventsSetup = true;
}

// Create a single common event which fired whenever any of the gui elements represented by xmlNames is changed
//---------------------------------------------
ofEvent <guiCallbackData> & ofControlPanel::createEventGroup(string eventGroupName, vector <string> xmlNames){
	customEvents.push_back( new guiCustomEvent() );
	customEvents.back()->group = eventGroupName;
	customEvents.back()->names = xmlNames;
	return customEvents.back()->guiEvent;	
}		

//this takes a single GUI name and makes it an event group - ie if you wanted to listen to just one slider or button
//---------------------------------------------
ofEvent <guiCallbackData> & ofControlPanel::createEventGroup(string xmlName){
	vector <string> xmlNames;
	xmlNames.push_back(xmlName);
	customEvents.push_back( new guiCustomEvent() );
	customEvents.back()->group = xmlName;
	customEvents.back()->names = xmlNames;
	return customEvents.back()->guiEvent;
}		

//---------------------------------------------
void ofControlPanel::enableEvents(){
	if( !bEventsSetup ){
		setupEvents();
	}
	eventsEnabled = true;
}

//---------------------------------------------
void ofControlPanel::disableEvents(){
	eventsEnabled = false;
}

// Get an event object for just a panel
//---------------------------------------------
ofEvent <guiCallbackData> & ofControlPanel::getEventsForPanel(int panelNo){
	if( panelNo < panels.size() ){
		return getEventGroup("PANEL_EVENT_"+ofToString(panelNo));
	}else{			
		return guiEvent;
	}
}

//---------------------------------------------
ofEvent <guiCallbackData> & ofControlPanel::getAllEvents(){
	return guiEvent;
 } 


// Use the name you made for your custom group to get back the event object
//---------------------------------------------
ofEvent <guiCallbackData> & ofControlPanel::getEventGroup(string eventGroupName){
	for(int i = 0; i < customEvents.size(); i++){
		if( eventGroupName == customEvents[i]->group ){
			return customEvents[i]->guiEvent;
		}
	}
	
	//if we don't find a match we return the global event
	ofLog(OF_LOG_ERROR, "error eventGroup %s does not exist - returning the global event instead", eventGroupName.c_str());
	return guiEvent;
}

//This is protected
//---------------------------------------------
void ofControlPanel::eventsIn(guiCallbackData & data){
	ofMessage msg = data.getXmlName();
	ofSendMessage(msg);

	if( !eventsEnabled ) return;
	
	//we notify the ofControlPanel event object - aka the global ALL events callback
	ofNotifyEvent(guiEvent, data, this);
	
	//we then check custom event groups
	for(int i = 0; i < customEvents.size(); i++){
		for(int k = 0; k < customEvents[i]->names.size(); k++){
			if( customEvents[i]->names[k] == data.getXmlName() ){
				ofNotifyEvent(customEvents[i]->guiEvent, data, this);
			}
		}
	}
}
		
// ############################################################## //
// ##
// ##       get and set values
// ##
// ############################################################## //

//---------------------------------------------
void ofControlPanel::setValueB(string xmlName, bool value,  int whichParam){
    for(int i = 0; i < (int) guiObjects.size(); i++){
        if( guiObjects[i]->xmlName == xmlName){
            if( whichParam >= 0  ){
                guiObjects[i]->value.setValue(value, whichParam);
                return;
            }
        }
    }
}

//---------------------------------------------
void ofControlPanel::setValueI(string xmlName, int value,  int whichParam){
    for(int i = 0; i < (int) guiObjects.size(); i++){
        if( guiObjects[i]->xmlName == xmlName){
            if( whichParam >= 0  ){
                guiObjects[i]->value.setValue(value, whichParam);
                return;
            }
        }
    }
}

//---------------------------------------------
void ofControlPanel::setValueF(string xmlName, float value,  int whichParam){
    for(int i = 0; i < (int) guiObjects.size(); i++){
        if( guiObjects[i]->xmlName == xmlName){
            if( whichParam >= 0  ){
                guiObjects[i]->value.setValue(value, whichParam);
                return;
            }
        }
    }
}


//---------------------------------------------
bool ofControlPanel::getValueB(string xmlName, int whichParam){
    for(int i = 0; i < (int) xmlObjects.size(); i++){
        if( xmlObjects[i].guiObj != NULL && xmlName == xmlObjects[i].xmlName ){
            if( whichParam >= 0 && whichParam < xmlObjects[i].numParams ){
                return xmlObjects[i].guiObj->value.getValueB(whichParam);
            }
        }
    }
    ofLog(OF_LOG_WARNING, "ofControlPanel - parameter requested %s doesn't exist - returning 0", xmlName.c_str());
    return 0;
}

//---------------------------------------------
float ofControlPanel::getValueF(string xmlName, int whichParam){
    for(int i = 0; i < (int) xmlObjects.size(); i++){
        if( xmlObjects[i].guiObj != NULL && xmlName == xmlObjects[i].xmlName ){
            if( whichParam >= 0 && whichParam < xmlObjects[i].numParams ){
                return xmlObjects[i].guiObj->value.getValueF(whichParam);
            }
        }
    }
    ofLog(OF_LOG_WARNING, "ofControlPanel - parameter requested %s doesn't exist - returning 0", xmlName.c_str());
    return 0;
}

//---------------------------------------------
int ofControlPanel::getValueI(string xmlName, int whichParam){
    for(int i = 0; i < (int) xmlObjects.size(); i++){
        if( xmlObjects[i].guiObj != NULL && xmlName == xmlObjects[i].xmlName ){
            if( whichParam >= 0 && whichParam < xmlObjects[i].numParams ){
                return xmlObjects[i].guiObj->value.getValueI(whichParam);
            }
        }
    }
    ofLog(OF_LOG_WARNING, "ofControlPanel - parameter requested %s doesn't exist - returning 0", xmlName.c_str());
    return 0;
}

//---------------------------------------------
bool ofControlPanel::hasValueChanged(string xmlName, int whichParam){
    for(int i = 0; i < (int) guiObjects.size(); i++){
        if( guiObjects[i]->xmlName == xmlName){
            if( whichParam >= 0  ){
				return guiObjects[i]->value.hasValueChanged(whichParam);
            }
        }
    }
	return false;
}

//---------------------------------------------
bool ofControlPanel::hasValueChangedInPanel(string whichPanel){
	
	guiTypePanel * panel = NULL;
	
	for(int i = 0; i < panels.size(); i++){
		if( panels[i]->name == whichPanel ){
			panel = panels[i];
			break;
		}
	}

	if( panel == NULL ){
		return false;
	}

	 for(int i = 0; i < (int)panel->children.size(); i++){
		for(int k = 0; k < panel->children[i]->value.getNumValues(); k++){
			if( panel->children[i]->value.hasValueChanged(k) ){
				return true;
			}
		}
    }
	return false;
}

//---------------------------------------------
bool ofControlPanel::newPanelSelected(){
	return bNewPanelSelected;
}

//---------------------------------------------
string ofControlPanel::getSelectedPanelName(){
    if( selectedPanel < 0 || selectedPanel >= panels.size() )return "no panel";
	return panels[selectedPanel]->name;
}



// ############################################################## //
// ##
// ##       Settings to/from xml
// ##
// ############################################################## //

//-----------------------------
void ofControlPanel::setIncrementalSave(string incrementalFileBaseName){
    incrementSaveName = incrementalFileBaseName;
    incrementSave = true;
}
//-----------------------------
void ofControlPanel::disableIncrementalSave(){
    incrementSave = false;
}

//-----------------------------
void ofControlPanel::loadSettings(string xmlFile){
    for(unsigned int i = 0; i < guiObjects.size(); i++)
		guiObjects[i]->loadSettings(xmlFile);

    currentXmlFile = xmlFile;

    settingsDirectory = currentXmlFile;

    int posLastSlash = settingsDirectory.rfind("/");
    if( posLastSlash > 0) settingsDirectory.erase(settingsDirectory.begin()+ posLastSlash+1, settingsDirectory.end()  );
    else settingsDirectory = "";

    settings.loadFile(currentXmlFile);
    usingXml = true;

    for(unsigned int i = 0; i < xmlObjects.size(); i++){
        if( xmlObjects[i].guiObj != NULL ){
            int numParams = xmlObjects[i].numParams;

            for(int j = 0; j < numParams; j++){
                string str = xmlObjects[i].xmlName+":val_"+ofToString(j);
                float val = settings.getValue(str, xmlObjects[i].guiObj->value.getValueF(j));

                xmlObjects[i].guiObj->setValue(val, j);
            }
            xmlObjects[i].guiObj->updateValue();
        }
    }
}

//-----------------------------
void ofControlPanel::reloadSettings(){
    for(unsigned int i = 0; i < guiObjects.size(); i++)
	{
		//printf("loading setting for guiObject %s\n", guiObjects[i]->name.c_str() );
		guiObjects[i]->reloadSettings();
	}

    if( currentXmlFile != "" ){

        bool loadedOK = settings.loadFile(currentXmlFile);
        if(loadedOK)
        {
            usingXml = true;

            for(unsigned int i = 0; i < xmlObjects.size(); i++){
                if( xmlObjects[i].guiObj != NULL ){
                    int numParams = xmlObjects[i].numParams;
					//printf("loading setting for xmlObject %s\n", xmlObjects[i].xmlName.c_str() );
					
                    for(int j = 0; j < numParams; j++){
                        string str = xmlObjects[i].xmlName+":val_"+ofToString(j);
                        float val = settings.getValue(str, xmlObjects[i].guiObj->value.getValueF(j));

						//printf("  %s -> %f\n", xmlObjects[i].xmlName.c_str(), val );
                        xmlObjects[i].guiObj->setValue(val, j);
                    }
                    xmlObjects[i].guiObj->updateValue();
                }
            }

        } else {
            ofLog(OF_LOG_ERROR,"Could not load %s.",currentXmlFile.c_str());
        }

    }
}

//-------------------------------
void ofControlPanel::saveSettings(string xmlFile,  bool bUpdateXmlFile){
    for(int i = 0; i < (int) guiObjects.size(); i++)guiObjects[i]->saveSettings(xmlFile);

    for(int i = 0; i < (int) xmlObjects.size(); i++){
        if( xmlObjects[i].guiObj != NULL ){
            int numParams = xmlObjects[i].numParams;

            for(int j = 0; j < numParams; j++){
                string str = xmlObjects[i].xmlName+":val_"+ofToString(j);
                settings.setValue(str, xmlObjects[i].guiObj->value.getValueF(j));
            }
        }
    }

    if( incrementSave ){
        string xmlName = incrementSaveName;
        xmlName += ofToString(ofGetYear()) +"-"+ ofToString(ofGetMonth()) +"-"+ ofToString(ofGetDay()) +"-"+ ofToString(ofGetHours()) +"-"+ ofToString(ofGetMinutes())+"-"+ ofToString(ofGetSeconds());
        xmlName += ".xml";
        xmlName = settingsDirectory + xmlName;

        settings.saveFile(xmlName);
    }
	//printf("saving %s\n", xmlFile.c_str());
    settings.saveFile(xmlFile);
	if( bUpdateXmlFile ){
		currentXmlFile = xmlFile;
	}
    usingXml = true;
}

//-----------------------------
void ofControlPanel::saveSettings(){
    for(int i = 0; i < (int) guiObjects.size(); i++)
    {
        guiObjects[i]->saveSettings();
    }

    for(int i = 0; i < (int) xmlObjects.size(); i++)
    {
        if( xmlObjects[i].guiObj != NULL )
        {
            int numParams = xmlObjects[i].numParams;

            for(int j = 0; j < numParams; j++){
                string str = xmlObjects[i].xmlName+":val_"+ofToString(j);
                settings.setValue(str, xmlObjects[i].guiObj->value.getValueF(j));
            }
        }
    }
    if( incrementSave ){
        string xmlName = incrementSaveName;
        xmlName += ofToString(ofGetYear()) +"-"+ ofToString(ofGetMonth()) +"-"+ ofToString(ofGetDay()) +"-"+ ofToString(ofGetHours()) +"-"+ ofToString(ofGetMinutes())+"-"+ ofToString(ofGetSeconds());
        xmlName += ".xml";
        xmlName = settingsDirectory + xmlName;

        settings.saveFile(xmlName);
    }
    if(currentXmlFile == "") {
        currentXmlFile = "controlPanelSettings.xml";
    }
	//printf("saving %s\n", currentXmlFile.c_str());
	
    settings.saveFile(currentXmlFile);
    usingXml = true;
}

//-------------------------------
void ofControlPanel::setXMLFilename(string xmlFile)
{
    currentXmlFile = xmlFile;
}

// ############################################################## //
// ##
// ##       Visibilty
// ##
// ############################################################## //


//---------------------------
void ofControlPanel::setMinimized(bool bMinimize){
    minimize = bMinimize;
}

//---------------------------
void ofControlPanel::setDraggable(bool bDrag){
    bDraggable = bDrag;
}

//-------------------------------
void ofControlPanel::show(){
    hidden = false;
}

//-------------------------------
void ofControlPanel::hide(){
    hidden = true;
}

//-------------------------------
void ofControlPanel::toggleView(){
    hidden = !hidden;
}

// ############################################################## //
// ##
// ##       Mouse Events
// ##
// ############################################################## //


//-------------------------------
bool ofControlPanel::mousePressed(float x, float y, int button){
    if( hidden ) return false;

	bool hitSomething	  =	false;
    bool tabButtonPressed = false;
	
	ofRectangle checkRect = boundingBox;
	if( minimize ){
		checkRect.height = topBar.height;
	}	

	//we do this so people can check if mouse is interacting with panel
	if( isInsideRect(x, y, checkRect) ){
		hitSomething = true;
	}

    if( isInsideRect(x, y, minimizeButton)){
        minimize = !minimize;
    }else if( usingXml && isInsideRect(x, y, saveButton) ){
        saveSettings();
        saveDown = true;
		printf("saving settings!\n");
    }else if( usingXml && isInsideRect(x, y, restoreButton) ){
		printf("restoring settings!\n");
        reloadSettings();
        restoreDown = true;
    }else if( isInsideRect(x, y, topBar) && bDraggable){
        dragging = true;
        mouseDownPoint.set(x - boundingBox.x, y-boundingBox.y, 0);
    }else if(!minimize){
		int lastSelectedPanel = selectedPanel;
        for(int i = 0; i < (int) panels.size(); i++){
            if( isInsideRect(x, y, panelTabs[i]) ){
                selectedPanel = i;
				if( lastSelectedPanel != selectedPanel ){
					bNewPanelSelected = true;
				}else{
					bNewPanelSelected = false;
				}
                tabButtonPressed = true;
                break;
            }
        }
    }

    if(minimize == false && tabButtonPressed == false && isInsideRect(x, y, boundingBox) ){
        for(int i = 0; i < (int) panels.size(); i++){
             if( i == selectedPanel )panels[i]->checkHit( x - hitArea.x, y - hitArea.y, button);
        }
    }

    prevMouse.set(x, y);

	return hitSomething;
}


//-------------------------------
bool ofControlPanel::mouseDragged(float x, float y, int button){
    if( hidden ) return false;
	
	//we do this so people can check if mouse is interacting with panel
	bool isDragging = dragging;
		
	if( !isDragging ){
		ofRectangle checkRect = boundingBox;
		if( minimize ){
			checkRect.height = topBar.height;
		}	
		if( isInsideRect(x, y, checkRect) ){
			isDragging = true;
		}
	}

    if(dragging)setPosition( MAX(0, x - mouseDownPoint.x), MAX(0, y -mouseDownPoint.y));
    else if(!minimize){
        for(int i = 0; i < (int) panels.size(); i++){
            if( i == selectedPanel ){

                if(button){
                    panels[i]->updateGui( x - prevMouse.x, y - prevMouse.y, false, true);
                }else{
                    panels[i]->updateGui( x - hitArea.x, y - hitArea.y, false, false);
                }
            }
        }
    }

    prevMouse.set(x, y);
	return isDragging;
}

//-------------------------------
void ofControlPanel::mouseReleased(){
    if( hidden ) return;

    for(int i = 0; i < (int) panels.size(); i++){
        panels[i]->release();
    }
    dragging        = false;
    saveDown        = false;
    restoreDown     = false;
}
//-------------------------------
bool ofControlPanel::keyPressed(int k){

	if ( hidden ) return false;
	if ( minimize ) return false;
	for( int i=0; i<(int)panels.size(); i++ ){
		bool eaten = panels[i]->keyPressed( k );
		if ( eaten ){
			// stop + bail
			return true;
		}
	}
	return false;
	
}

//-------------------------------
void ofControlPanel::keyReleased(int k){
	
	for( int i=0; i<(int)panels.size(); i++ ){
		panels[i]->keyPressed( k );
	}
	
}

// ############################################################## //
// ##
// ##       Updater
// ##
// ############################################################## //

//-------------------------------
void ofControlPanel::update(){
    guiBaseObject::update();
	
	if( panels.size() == 0 ) return; 
	
	displayText.setText(getSelectedPanelName());

    topBar           = ofRectangle(boundingBox.x, boundingBox.y, boundingBox.width, MAX(20, displayText.getTextSingleLineHeight() * 1.2 ) );
    minimizeButton   = ofRectangle(boundingBox.x + boundingBox.width - 24, boundingBox.y + 4, 20, 10 );
	if ( !bDoSaveRestore )
	{
		saveButton		= ofRectangle( boundingBox.x, boundingBox.y, 0, 0 );
		restoreButton	= ofRectangle( boundingBox.x, boundingBox.y, 0, 0 );
	}
	else
	{
		saveButton       = ofRectangle(boundingBox.x + displayText.getTextWidth() + 20, boundingBox.y + 4, MAX(40, 8 + displayText.getTextWidth("save")) , MAX(12, displayText.getTextSingleLineHeight()) );
		restoreButton    = ofRectangle(saveButton.x + saveButton.width + 15, boundingBox.y + 4,  MAX(60, 8 + displayText.getTextWidth("restore")) , MAX(12, displayText.getTextSingleLineHeight()) );
	}

	ofControlPanel::topSpacing = MAX(20, topBar.height);

    for(int i = 0; i < (int) panels.size(); i++){
        panels[i]->update();

        panelTabs[i].x      = i * tabWidth + hitArea.x + borderWidth;
        panelTabs[i].y      = hitArea.y + 15 - tabHeight;
        panelTabs[i].width  = tabWidth;
        panelTabs[i].height = tabHeight;

    }
}

//---------------------------------------------
void ofControlPanel::clearAllChanged(){
	for(int i = 0; i < (int) guiObjects.size(); i++){
		guiObjects[i]->value.clearChangedFlag();
    }
	
	bNewPanelSelected = false;
}

//---------------------------------------------
void ofControlPanel::updateBoundingBox(){
    if(bShowText){
        //we need to update out hitArea because the text will have moved the gui down
        hitArea.y = boundingBox.y + topSpacing;
        boundingBox.height = hitArea.height + displayText.getTextHeight() + titleSpacing;
        boundingBox.width  = MAX( hitArea.width, displayText.getTextWidth() );
    }else{
         //we need to update out hitArea because the text will have moved the gui down
        hitArea.y = boundingBox.y;
        boundingBox.height = hitArea.height;
        boundingBox.width  = hitArea.width;
    }
}


// ############################################################## //
// ##
// ##       Drawing
// ##
// ############################################################## //

//-------------------------------
void ofControlPanel::draw(){
    if( hidden ) return;

	lastFrameDrawn	= ofGetFrameNum();

    ofPushStyle();
    ofEnableAlphaBlending();

        float panelH = boundingBox.height;
        if( minimize ){
            panelH = topBar.height;
        }

        glPushMatrix();
            glTranslatef(boundingBox.x, boundingBox.y, 0);
            //draw the background
            ofFill();
            glColor4fv(bgColor.getColorF());
            ofRect(0, 0, boundingBox.width, panelH);

            //draw the outline
            ofNoFill();
            glColor4fv(outlineColor.getColorF());
            ofRect(0, 0, boundingBox.width, panelH);
            ofLine(0, topBar.height, boundingBox.width, topBar.height);
        glPopMatrix();

        ofRect(minimizeButton.x, minimizeButton.y, minimizeButton.width, minimizeButton.height);

	if ( bDoSaveRestore )
	{
        ofPushStyle();
            ofFill();

            if( saveDown )glColor4fv(fgColor.getSelectedColorF());
            else glColor4fv(fgColor.getColorF());

            ofRect(saveButton.x, saveButton.y, saveButton.width,saveButton.height);
            ofSetColor(255, 255, 255);
		if(bUseTTFFont) {
			guiTTFFont.drawString("save", saveButton.x + 3, saveButton.y + saveButton.height -4);
		}
		else {
			ofDrawBitmapString("save", saveButton.x + 3, saveButton.y + saveButton.height -3);
		}

        ofPopStyle();

        ofPushStyle();
            ofFill();

            if( restoreDown )glColor4fv(fgColor.getSelectedColorF());
            else glColor4fv(fgColor.getColorF());

            ofRect(restoreButton.x, restoreButton.y, restoreButton.width,restoreButton.height);
            ofSetColor(255, 255, 255);
		if(bUseTTFFont) {
			guiTTFFont.drawString("restore", restoreButton.x + 3, restoreButton.y + restoreButton.height -4);
		}
		else {
			ofDrawBitmapString("restore", restoreButton.x + 3, restoreButton.y + restoreButton.height -3);
		}
		ofPopStyle();
	}

        ofPushMatrix();
            ofTranslate(2,0,0);
            glColor4fv(textColor.getColorF());
            guiBaseObject::renderText();
        ofPopMatrix();

        if( !minimize ){

            //don't let gui elements go out of their panels
            glEnable(GL_SCISSOR_TEST);
            glScissor(boundingBox.x, ofGetHeight() - ( boundingBox.y + boundingBox.height - (-2 + topSpacing) ), boundingBox.width - borderWidth , boundingBox.height);

                for(int i = 0; i < (int) panelTabs.size(); i++){
                    if( i == selectedPanel){
                        ofPushStyle();
                            ofFill();
                            glColor4fv(fgColor.getSelectedColorF());
                            ofRect(panelTabs[i].x, panelTabs[i].y, panelTabs[i].width, panelTabs[i].height);
                            glColor4fv(outlineColor.getColorF());
                        ofPopStyle();
                    }
                    glColor4fv(outlineColor.getColorF());
                    ofNoFill();
                    ofRect(panelTabs[i].x, panelTabs[i].y, panelTabs[i].width, panelTabs[i].height);
                }

                glPushMatrix();
                    glTranslatef(hitArea.x, hitArea.y, 0);
                    for(int i = 0; i < (int) panels.size(); i++){
                        if( i == selectedPanel )panels[i]->render();
                    }
                glPopMatrix();

            glDisable(GL_SCISSOR_TEST);
        }

    ofPopStyle();
}



