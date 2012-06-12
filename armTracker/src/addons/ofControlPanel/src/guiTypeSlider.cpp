#include "guiTypeSlider.h"

//------------------------------------------------
void guiTypeSlider::setup(string sliderName, string xmlNameIn, float defaultVal, float min, float max){
	value.addValue(defaultVal, min, max);
	name = sliderName;
	xmlName = xmlNameIn;
	
	setDimensions(180, 14);
}

//-----------------------------------------------
void guiTypeSlider::updateValue(){
	notify();
}

//---------------------------------------------
void guiTypeSlider::updateBoundingBox(){
	hitArea.y = boundingBox.y;
	if( bShowText ){
		hitArea.height = boundingBox.height = displayText.getTextHeight() + 3.0;
	}
}

//-----------------------------------------------.
void guiTypeSlider::updateGui(float x, float y, bool firstHit, bool isRelative){
	if( state == SG_STATE_SELECTED){
		if( !isRelative ){
			float pct = ( x - ( hitArea.x ) ) / hitArea.width;
			value.setValueAsPct( pct );
		}else if( !firstHit ){
			float pct = value.getPct();
			pct += (x * 0.02) / hitArea.width;
			value.setValueAsPct( pct );
		}
		notify();
	}
}

//-----------------------------------------------.
void guiTypeSlider::render(){
	ofPushStyle();
		glPushMatrix();
			//draw the background
			ofFill();
			glColor4fv(bgColor.getColorF());
			ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

			//draw the foreground
			glColor4fv(fgColor.getColorF());
			ofRect(hitArea.x, hitArea.y, hitArea.width * value.getPct(), hitArea.height);

			//draw the outline
			ofNoFill();
			glColor4fv(outlineColor.getColorF());
			ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

			glColor4fv(textColor.getColorF());			
			displayText.renderString(name, hitArea.x + 2.0, hitArea.y + displayText.getTextSingleLineHeight() );
			
			float rightAlignVarsX = hitArea.width - displayText.getTextWidth(varsString);
			displayText.renderString(varsString, hitArea.x + rightAlignVarsX - 2.0, hitArea.y + displayText.getTextSingleLineHeight() );
			
		glPopMatrix();
	ofPopStyle();
}
