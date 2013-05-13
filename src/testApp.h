#pragma once

#include "ofMain.h"

#include "ofxKinect.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "ofxDelaunay.h"
#include "ofxUI.h"
#include "ofxPostProcessing.h"
#include "ofxDuration.h"
//#include "Canon.h"
//#define NUM_BILLBOARDS 10000
#define NUM_STRIP 200
#define LOC_LENGTH 10
#define LENGTH LOC_LENGTH*2


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
    void fireStrip(float x ,float y, float vx, float vy);
    ofxKinect kinect;

    float angle;
    ofEasyCam cam;

    ofxOscReceiver receiver;
    ofxOscSender sender;
    ofxPostProcessing post;
    ofxUICanvas gui;
    

    ofVec3f orbit;
    enum MODE
    {
        POINT,
        SLITSCAN,
        TRIANGLE,

        DISPLACEMENT,
    };
    MODE mode;
    ofxUICanvas *gui1;
    void setGUI1();
    void guiEvent(ofxUIEventArgs &e);
    float nearThreshold,farThreshold,camDistance,meshDistance,bbNormal,depthScale,maxHeight;
    bool bOsc;
    
    ofShader billboardShader;
    ofShader displacement;
    ofImage colormap, bumpmap;
    ofImage texture;
    ofDirectory dir;
    int numEntry;
    
    vector<string>renderMode;
    vector<string>gradientModeRadioOption;
    bool doPause ;
//    bool canonDraw;
//    ofTexture live_texture;
    ofMesh mesh;    // final mesh
    ofMesh scanMesh;
    
    ofVbo vbo;
    ofVec3f pos[NUM_STRIP];
    ofVec3f acc[NUM_STRIP];
    ofVec3f vec[NUM_STRIP];
    float age[NUM_STRIP];
    ofVec3f strip[NUM_STRIP*LENGTH];
    ofVec3f loc[NUM_STRIP*LOC_LENGTH];
	ofFloatColor color[NUM_STRIP*LENGTH];
    int total;
    int count;
    ofVec3f attraction;
    ofVec3f centerPoint;
    ofxDelaunay triangulation;
    ofxDuration duration;
    void trackUpdated(ofxDurationEventArgs& args);
	string ip;
	int port;

    float rgbAlpha;

    ofVec2f rgbPos;
    shared_ptr<DofPass> Ptr;
};
