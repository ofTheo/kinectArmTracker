#include "trueVectorField.h"

static int rasterPos(float val){
    int val1 = (int)val + 1;
    int val2 = (int)val;

    if( fabs(val1 - val) < fabs(val2 - val) ){
        return val1;
    }
    return val2;
}


//------------------------------------------------------------------------
trueVectorField::trueVectorField(){
    sqrtOf2 = sqrt(2.0);
}

//------------------------------------------------------------------------
void trueVectorField::setup(int _fieldw, int _fieldh, float _scalew, float _scaleh){

    fieldw = _fieldw;
    fieldh = _fieldh;
    scalew = _scalew;
    scaleh = _scaleh;

    FIELD_SIZE = fieldw * fieldh;
    field = new ofVec3f[FIELD_SIZE];

    clear();
}

//------------------------------------------------------------------------
ofVec3f * trueVectorField::getField(){
    return field;
}

//------------------------------------------------------------------------
void trueVectorField::clear(){
    for (int i = 0; i < FIELD_SIZE; i++){
        field[i].set(0,0,0);
    }
}

//------------------------------------------------------------------------
void trueVectorField::limit(float value){
	float valSq = value * value;
	
    for (int i = 0; i < FIELD_SIZE; i++){
        if(field[i].lengthSquared() > valSq){
            field[i].normalize();
            field[i] *= value;
        }
    }
}


//------------------------------------------------------------------------
void trueVectorField::fadeField(float fadeAmount){
    for (int i = 0; i < FIELD_SIZE; i++){
        field[i]*= fadeAmount;
        if( fabs(field[i].length()) < 0.00001) field[i] = 0;
    }
}


//------------------------------------------------------------------------
void trueVectorField::randomizeField(float scale){
    for (int i = 0; i < FIELD_SIZE; i++){
        field[i].set(ofRandomf(), ofRandomf(), 0.0f);
        field[i].normalize();
        field[i] *= scale;
    }
}

//------------------------------------------------------------------------
void trueVectorField::blurField(float pct){

    int stride = 0;

    ofVec3f val;

    int ii, ss, ee, nn, ww, nw, ne, sw, se;

    for (int y = 1; y < fieldh-1; y++){

        stride = fieldw * y;

        for (int x = 1; x < fieldw-1; x++){

            val.set(0, 0, 0);
            int num = 0;

            ii = stride + x;
            nn = ii - fieldw;
            ss = ii + fieldw;
            ee = ii + 1;
            ww = ii - 1;
            nw = nn + 1;
            ne = nn -1;
            sw = ss +1;
            se = ss -1;


            val = field[nn] + field[ss] + field[ee] + field[ww] + field[nw] + field[ne] + field[sw] + field[se];
            val *= 0.125;

            field[ii] = field[ii]*(1.0 - pct) + val*pct;
        }
    }
}

//------------------------------------------------------------------------
void trueVectorField::addIntoField(ofVec3f pos, ofVec3f vector, float strength){

    pos.x = MIN(1, pos.x);
    pos.y = MIN(1, pos.y);
    pos.x = MAX(0, pos.x);
    pos.y = MAX(0, pos.y);

    pos.x *= (float)fieldw;
    pos.y *= (float)fieldh;

    ofVec3f p;
    ofVec3f v;

    ofVec3f add;
    p.set(pos.x, pos.y, 0);
    v.set(vector.x, vector.y, 0);

    int x1 = (int)p.x;
    int x2 = x1 + 1;

    int y1 = (int)p.y;
    int y2 = y1 + 1;

    y1 = MAX(0, y1);
    x1 = MAX(0, x1);

    float sqrt2 = sqrt(2.0);

    for(int y = y1; y < y2; y++){
        if(y >= fieldh) break;
        for(int x = x1; x < x2; x++){
            if(x >= fieldw)break;

            float dist = sqrt( (x-p.x)*(x-p.x) + (y-p.y)*(y-p.y) ) / sqrt2;

            //printf("dist is %f \n", dist);

            int index  = y * fieldw + x;
            add = v * (1.0 - dist) + field[index]*dist;
            field[index] = add * strength + field[index] * (1.0 - strength);

        }
    }

}

//------------------------------------------------------------------------
ofVec3f trueVectorField::getVelFromPos(float xPos, float yPos){

    if(xPos < 0 || xPos > 1)return ofVec3f(0,0);
    if(yPos < 0 || yPos > 1)return ofVec3f(0,0);


    xPos *= (float)fieldw;
    yPos *= (float)fieldh;

    ofVec3f vel;
    vel.set(0,0);

    int x1 = (int)xPos;
    int x2 = x1 + 1;

    int y1 = (int)yPos;
    int y2 = y1+1;

    //printf("x1 is %i  x2 is %i y1 is %i y2 is %i\n", x1, x2, y1, y2);

    float dist;

    float total  = 0.0;

    //printf("-----\n");

    for(int y = y1; y <= y2; y++){
        if(y >= fieldh) break;
        for(int x = x1; x <= x2; x++){
            if(x >= fieldw)break;

            float dist = (sqrtOf2 - (sqrt( (x-xPos)*(x-xPos) + (y-yPos)*(y-yPos) ) ))/sqrtOf2;

            //printf("dist is %f \n", dist);

            total += dist;

            int index  = y * fieldw + x;

            vel.x += field[index].x * dist*0.25;
            vel.y += field[index].y * dist*0.25;

        }
    }

    //printf("total is %f \n", total);

    //printf("-----\n");

    return vel;

}

//------------------------------------------------------------------------
void trueVectorField::draw(float x, float y, float w, float h, float scaleVector){

    float wPct = w / (float)fieldw;
    float hPct = h / (float)fieldh;

    for (int j = 0; j < fieldh; j++){

        float py = y + hPct * (float)j;

        for (int i = 0; i < fieldw; i++){

            int pos = j * fieldw + i;
            float px = x + wPct * (float)i;
            ofLine(px, py, px + field[pos].x * scaleVector, py + field[pos].y * scaleVector);
        }
    }
}




