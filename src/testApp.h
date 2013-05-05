#pragma once

#include "ofMain.h"

#include "ofxKinect.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "ofxDelaunay.h"
#include "ofxUI.h"
#include "ofxPostProcessing.h"
#include "Canon.h"
#define NUM_BILLBOARDS 640*480
class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
	
	void drawPointCloud();

    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    ofxKinect kinect;
    ofxDelaunay triangulation;
    float angle;
    ofEasyCam cam;
    ofMesh mesh;
    ofxOscReceiver receiver;
    ofxPostProcessing post;
    ofxUICanvas gui;
    
//    ofLight light;
    ofVec3f orbit;
    
    
    roxlu::Canon canon;
    void onPictureTaken(roxlu::CanonPictureEvent& ev);
    int timeTry;
    enum MODE
    {
        POINT,
        TRIANGLE,
        BILLBOARD,
        DISPLACEMENT,
    };
    MODE mode;
    ofxUICanvas *gui1;
    void setGUI1();
    void guiEvent(ofxUIEventArgs &e);
    float minRange,maxRange,camDistance,meshDistance,bbNormal;
    bool bOsc;
    
    ofShader billboardShader;
    ofImage texture;
    ofDirectory dir;
    int numEntry;


};
