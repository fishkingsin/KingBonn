#pragma once

#include "ofMain.h"
//#include "ofxBonjour.h"
#include "ofxOsc.h"
//#include "ofxKinect.h"
#include "ofxXmlSettings.h"
#include "ofxDelaunay.h"
#include "ofxDuration.h"
#include "ofxSyphon.h"
#include "ofxTween.h"
#include "ofxControlPanel.h"
//using namespace ofxBonjour;
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
//    ofxKinect kinect;
//    Server bonjourServer;
//    float angle;
};
