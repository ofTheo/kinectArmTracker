#include "ofMain.h"

//------------
class trueVectorField : public ofBaseDraws{

  public:

        trueVectorField & operator =( const trueVectorField & vecIn){
            for(int i = 0; i < FIELD_SIZE; i++){
                field[i] = vecIn.field[i];
            }
            return *this;
        }

        trueVectorField & operator -=( const trueVectorField & vecIn){
            for(int i = 0; i < FIELD_SIZE; i++){
                field[i] -= vecIn.field[i];
            }
            return *this;
        }

        trueVectorField & operator +=( const trueVectorField & vecIn){
            for(int i = 0; i < FIELD_SIZE; i++){
                field[i] += vecIn.field[i];
            }
            return *this;
        }

      	//------------------------------------------------------------------------
    	trueVectorField();
    	void setup(int _fieldw, int _fieldh, float _scalew, float _scaleh);
    	ofVec3f * getField();
    	void clear();
    	void limit(float value);
    	void fadeField(float fadeAmount);
		void randomizeField(float scale);
		void blurField(float pct);
		void addIntoField(ofVec3f pos, ofVec3f vector, float strength);
        ofVec3f getVelFromPos(float xPos, float yPos);
        void draw(float x, float y, float w, float h, float scaleVector);
        void draw(float x, float y, float w, float h){
            ofSetColor(0xFF00FF);
            draw(x, y, w, h, 380);
        }

		//------------------------------------------------------------------------
		void setIntoField(ofVec3f pos, ofVec3f vector){

    		pos.x *= (float)fieldw;
    		pos.y *= (float)fieldh;

            int x = ofClamp(pos.x, 0, fieldw-1);
            int y = ofClamp(pos.y, 0, fieldh-1);

            int index    = y * fieldw + x;
            field[index] = vector;

		}

        void draw(float x, float y){

        }

        float getWidth(){
            return 0;
        }

        float getHeight(){
            return 0;
        }

        int 	   	FIELD_SIZE;
        int         fieldw;
        int         fieldh;
        float       scalew;
        float       scaleh;
        ofVec3f *  field;
        float 	    sqrtOf2;

};

