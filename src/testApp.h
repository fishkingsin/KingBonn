#pragma once

#include "ofMain.h"
<<<<<<< HEAD
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
=======
#include "ofxKinect.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "ofxDelaunay.h"
<<<<<<< HEAD
>>>>>>> update project
=======
#include "ofxUI.h"
#include "ofxPostProcessing.h"
>>>>>>> added shader, added camera orbit , added porintCloud
class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
<<<<<<< HEAD
	void exit();
	
	void drawPointCloud();
    
=======
    void exit();
	
	void drawPointCloud();
>>>>>>> added shader, added camera orbit , added porintCloud
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
<<<<<<< HEAD
//    ofxKinect kinect;
//    Server bonjourServer;
//    float angle;
=======
    ofxKinect kinect;
    ofxDelaunay triangulation;
    float angle;
    ofEasyCam cam;
    ofxOscReceiver receiver;
    ofxPostProcessing post;
    ofxUICanvas gui;
    
    ofLight light;
    ofVec3f orbit;
>>>>>>> added shader, added camera orbit , added porintCloud
};
