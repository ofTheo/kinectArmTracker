/*
 *  guiTypeVairableLister.h
 *  ofControlPanelDemo
 *
 *  Created by theo on 01/04/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "guiBaseObject.h"
#include "guiVariablePointer.h"

class guiTypeVairableLister : public guiBaseObject{

    public:

        guiTypeVairableLister();

        void setup(string listerName, vector <guiVariablePointer> & varsIn);		
		void update();
		void updateBoundingBox();
		void render();
						
		vector <guiVariablePointer> vars;
		float minNameWidth, minVarWidth;
		float typeStartOffset;
};
