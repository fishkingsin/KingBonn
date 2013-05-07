#include "testApp.h"
//--------------------------------------------------------------
void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c) {
	ofVec3f normal = ((b - a).cross(c - a)).normalize();
    mesh.addVertex(a);
	mesh.addNormal(normal);
	mesh.addTexCoord(b);
	mesh.addVertex(b);
	mesh.addNormal(normal);
	mesh.addTexCoord(c);
	mesh.addVertex(c);
}

//--------------------------------------------------------------
void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d) {
	addFace(mesh, a, b, c);
	addFace(mesh, a, c, d);
}
//--------------------------------------------------------------
void testApp::setup(){
    kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	
	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
    
	
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
    
    receiver.setup(7170);
    
    // Setup post-processing chain
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    
    // Setup light
    //	light.setPosition(0, 0, 2000);
    
    orbit.set(0,0,0);
    canon.start();
	canon.addPictureTakenListener(this, &testApp::onPictureTaken);
    timeTry = 0;
    
    mode = POINT;
    minRange = 0;
    maxRange = 10000;
    setGUI1();
    
    
	// load the bilboard shader
	// this is used to change the
	// size of the particle
	billboardShader.load("Billboard");
    dir.allowExt("png");
	numEntry = dir.listDir("images/");
    
	// we need to disable ARB textures in order to use normalized texcoords
	ofDisableArbTex();
	texture.loadImage(dir.getPath(0));
    ofEnableArbTex();
	ofEnableAlphaBlending();
    
    particle.getVertices().resize(NUM_BILLBOARDS);
	particle.getColors().resize(NUM_BILLBOARDS);
	particle.getNormals().resize(NUM_BILLBOARDS,ofVec3f(0));
	
	// ------------------------- billboard particles
	for (int i=0; i<NUM_BILLBOARDS; i++) {
		
		billboardVels[i].set(ofRandomf(), -1.0, ofRandomf());
        particle.getVertices()[i].set(ofRandom(-500, 500),
                                      ofRandom(-500, 500),
                                      ofRandom(-500, 500));
		
		particle.getColors()[i].set(ofColor::fromHsb(ofRandom(96, 160), 0, 255));
	    
		
	}
    particle.setUsage( GL_DYNAMIC_DRAW );
	particle.setMode(OF_PRIMITIVE_POINTS);
    
}
void testApp::onPictureTaken(roxlu::CanonPictureEvent& ev) {
	cout << ev.getFilePath() << endl;
}
//--------------------------------------------------------------
void testApp::update(){
//    if(!canon.isLiveViewActive() && canon.isSessionOpen() && timeTry<5) {
//		canon.startLiveView();
//        timeTry++;
//        if(timeTry==5)
//        {
//            ofLogError("ofxCanon") << "5 time faile to try stop reconnect";
//        }
//	}
    if(kinect.isConnected())
    {
        kinect.update();
        
        // there is a new frame and we are connected
        if(kinect.isFrameNew())
        {
            
        }}
    if(bOsc)
    {
        // check for waiting messages
        while(receiver.hasWaitingMessages()){
            // get the next message
            ofxOscMessage m;
            receiver.getNextMessage(&m);
            
            // check for mouse moved message
            if(m.getAddress() == "/orbit"){
                orbit.x = m.getArgAsInt32(0);
                orbit.y = m.getArgAsInt32(1);
                orbit.z = m.getArgAsInt32(2);
            }
        }
        cam.orbit(-orbit.y, orbit.x, camDistance);
    }
    float t = (ofGetElapsedTimef()) * 0.9f;
	float div = 250.0;
	
	for (int i=0; i<NUM_BILLBOARDS; i++) {
		
		// noise
		ofVec3f vec(ofSignedNoise(t, particle.getVertex(i).y/div, particle.getVertex(i).z/div),
                    ofSignedNoise(particle.getVertex(i).x/div, t, particle.getVertex(i).z/div),
                    ofSignedNoise(particle.getVertex(i).x/div, particle.getVertex(i).y/div, t));
		
		vec *= 10 * ofGetLastFrameTime();
		billboardVels[i] += vec;
		particle.getVertices()[i] += billboardVels[i];
		billboardVels[i] *= 0.94f;
    	particle.setNormal(i,ofVec3f(10,0,0));
	}
}

//--------------------------------------------------------------
void testApp::draw(){
    
    ofBackground(0);
    
    // copy enable part of gl state
    
    // begin scene to post process
    //    if(mode == POINT || mode == TRIANGLE)
    //    {
    glPushAttrib(GL_ENABLE_BIT);
    
    // setup gl state
    glEnable(GL_DEPTH_TEST);
    //        glEnable(GL_CULL_FACE);
    //    light.enable();
    
    post.begin(cam);
    
    //    }else if(BILLBOARD == mode)
    //    {
    //        cam.begin();
    //    }
    if(kinect.isConnected())
    {
        drawPointCloud();
    }
	billboardShader.begin();
	ofEnablePointSprites();
	texture.getTextureReference().bind();
	particle.draw();
	texture.getTextureReference().unbind();
	ofDisablePointSprites();
    billboardShader.end();
	
    
    // end scene and draw
    //    if(mode == POINT|| mode == TRIANGLE)
    //    {
    post.end();
    // set gl state back to original
    glPopAttrib();
    
    glDisable(GL_DEPTH_TEST);
    
    //    }else if(BILLBOARD == mode)
    //    {
    //        cam.end();
    //    }
    if(!canon.isLiveViewActive())
    {
        canon.drawLiveView();
    }
}
void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
    if(mode==TRIANGLE)
    {
        triangulation.reset();
        
        int step = 10;
        for(int y = 0; y < h; y += step) {
            for(int x = 0; x < w; x += step) {
                if(kinect.getDistanceAt(x, y) > minRange && kinect.getDistanceAt(x, y) < maxRange)
                {
                    triangulation.addPoint(kinect.getWorldCoordinateAt(x, y));
                    
                }
            }
        }
        triangulation.triangulate();
        ofSetColor(255);
        ofSetLineWidth(1);
        ofPushMatrix();
        // the projected points are 'upside down' and 'backwards'
        ofScale(1, -1, -1);
        ofTranslate(0, 0, meshDistance); // center the points a bit
        glEnable(GL_DEPTH_TEST);
        triangulation.triangleMesh.drawWireframe();
        triangulation.triangleMesh.drawVertices();
        glDisable(GL_DEPTH_TEST);
        ofPopMatrix();
    }
    else if(mode==POINT || mode ==BILLBOARD)
    {
        ofVboMesh mesh;
        
        mesh.setUsage( GL_DYNAMIC_DRAW );
        mesh.setMode(OF_PRIMITIVE_POINTS);
        
        int step = 2;
        for(int y = 0; y < h; y += step) {
            for(int x = 0; x < w; x += step) {
                if(kinect.getDistanceAt(x, y) > minRange && kinect.getDistanceAt(x, y) < maxRange)
                {
                    mesh.addColor(kinect.getColorAt(x,y));
                    mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
                    if(mode==BILLBOARD)
                    {
                        mesh.addNormal(ofVec3f(bbNormal,0,0));
                    }
                }
            }
        }
        
        ofPushMatrix();
        //        ofRotateX( orbit.x );
        //		ofRotateY( orbit.y );
        //		ofRotateZ( orbit.z );
        // the projected points are 'upside down' and 'backwards'
        ofScale(1, -1, -1);
        
        ofTranslate(0, 0, meshDistance); // center the points a bit
        
        if(mode==BILLBOARD)
        {
            billboardShader.begin();
            ofEnableAlphaBlending();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            ofEnablePointSprites();
            texture.getTextureReference().bind();
            mesh.draw();
            
            texture.getTextureReference().unbind();
            ofDisablePointSprites();
            
            billboardShader.end();
            
        }
        else if (mode == POINT){
            glPointSize(5);
            mesh.drawVertices();
        }
        
        
        ofPopMatrix();
    }else if (mode ==DISPLACEMENT)
    {
        ofVboMesh mesh;
        
        mesh.setUsage( GL_DYNAMIC_DRAW );
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        int step = 2;
        for(int y = 0; y < h-step; y += step) {
            for(int x = 0; x < w-step; x += step) {
                float nwDis = kinect.getDistanceAt(x, y);
                float neDis = kinect.getDistanceAt(x+step, y);
                float swDis = kinect.getDistanceAt(x, y+step);
                float seDis = kinect.getDistanceAt(x+step, y+step);
                
                if( nwDis> minRange && nwDis < maxRange &&
                   neDis> minRange && neDis < maxRange &&
                   swDis> minRange && swDis < maxRange &&
                   seDis> minRange && seDis < maxRange )
                {
                    //                    mesh.addColor(kinect.getColorAt(x,y));
                    //                    mesh.addVertex();
                    ofVec3f nw = kinect.getWorldCoordinateAt(x, y);//ofVec3f( x, y , 0);
                    ofVec3f ne = kinect.getWorldCoordinateAt(x+step, y);//ofVec3f( x + step, y, 0);
                    ofVec3f sw = kinect.getWorldCoordinateAt(x, y+step);//ofVec3f( x, y + step, 0);
                    ofVec3f se =kinect.getWorldCoordinateAt(x+step, y+step) ;//ofVec3f( x + step, y + step, 0);
                    
                    addFace(mesh, nw, ne, se, sw);
                }
            }
        }
        
        ofPushMatrix();
        ofScale(1, -1, -1);
        
        ofTranslate(0, 0, meshDistance); // center the points a bit
        kinect.getTextureReference().bind();
        mesh.draw();
        kinect.getTextureReference().unbind();
        //        mesh.drawWireframe();
        //        mesh.drawVertices();
        
        
        ofPopMatrix();
        
    }
    
    
    
    
    
    
}
void testApp::exit() {
    canon.endLiveView();

    gui1->saveSettings("GUI/GUI1_Settings.xml");
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}
void testApp::setGUI1()
{
    
	
	float dim = 16;
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = 255-xInit;
    
	
    vector<string> names;
    
	gui1 = new ofxUICanvas(0, 0, length+xInit, ofGetHeight());
    
	gui1->addWidgetDown(new ofxUILabel("PANEL 1: BASICS", OFX_UI_FONT_LARGE));
    gui1->addToggle("ENABLE_OSC",&bOsc);
    gui1->addSlider("MIN_RANGE", 0, 10000, &minRange);
    gui1->addSlider("MAX_RANGE", 0, 10000, &maxRange);
    gui1->addSlider("CAM_DISTANCE", 0, 10000, & camDistance);
    gui1->addSlider("MESH_DISTANCE", -10000, 10000, & meshDistance);
    gui1->addSlider("BILLBOARD_NORM",0, 100, &bbNormal);
	ofAddListener(gui1->newGUIEvent,this,&testApp::guiEvent);
    gui1->setVisible(false);
    gui1->loadSettings("GUI/GUI1_Settings.xml");
}
void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
	cout << "got event from: " << name << endl;
    
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key)
    {
        case ' ':
            
            
            
            ofDisableArbTex();
            
            texture.loadImage(dir.getPath(int(ofRandom(numEntry))));
            ofEnableArbTex();
            
            break;
        case 'p':
            mode = POINT;
            break;
        case 't':
            mode = TRIANGLE;
            break;
        case 'b':
            mode = BILLBOARD;
            break;
        case 'd':
            mode = DISPLACEMENT;
            break;
        case '\t':
            gui1->toggleVisible();
			break;
            
        default:
            unsigned idx = key - '0';
            if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());
            break;
    }
    
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}