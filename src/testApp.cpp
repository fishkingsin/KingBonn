#include "testApp.h"
#include "MSASpaceTime.h"
float nearThreshold = 0;
float farThreshold = 3000;
int pixelStep = 3;          // how many pixels to step through the depth map when iterating
int numScanFrames = 240;    // duration (in frames) for full scan

// physical boundaries of space time continuum
ofVec3f spaceBoundaryMin    = ofVec3f(-400, -400, 400);
ofVec3f spaceBoundaryMax    = ofVec3f(400, 400, farThreshold);

// spatial resolution for space time continuum
ofVec3f spaceNumCells;


bool doSaveMesh = false;

//bool doDrawPointCloud = true;
//bool doSlitScan = true;
//bool doDebugInfo = false;

//bool usingKinect;   // using kinect or webcam

int gradientMode = 0;
string gradientModeStr = "";

int kinectAngle;
float inputWidth, inputHeight;

msa::SpaceTime<ofMesh> spaceTime;   // space time continuum

ofMesh scanMesh;    // final mesh
//--------------------------------------------------------------
void setGradientMode(int g) {
    gradientMode = g;
    spaceTime.clear();
    
    
    switch(gradientMode) {
        case 1:
            gradientModeStr = "left-right";
            spaceNumCells.set(500, 1, 1);
            break;
            
        case 2:
            gradientModeStr = "right-left";
            spaceNumCells.set(500, 1, 1);
            break;
            
        case 3:
            gradientModeStr = "top-bottom";
            spaceNumCells.set(1, 500, 1);
            break;
            
        case 4:
            gradientModeStr = "bottom-top";
            spaceNumCells.set(1, 500, 1);
            break;
            
        case 5:
            gradientModeStr = "front-back";
            spaceNumCells.set(1, 1, 500);
            break;
            
        case 6:
            gradientModeStr = "back-front";
            spaceNumCells.set(1, 1, 500);
            break;
            
        case 7:
            gradientModeStr = "spherical";
            spaceNumCells.set(30, 30, 30);
            break;
            
        case 8:
            gradientModeStr = "random";
            spaceNumCells.set(40, 40, 40);
            break;
            
        case 9:
            gradientModeStr = "oldest";
            spaceNumCells.set(2);
            break;
            
        default:
            gradientModeStr = "most recent";
            spaceNumCells.set(2);
            break;
            
            
    }
    
    ofLog(OF_LOG_VERBOSE, "setGradientMode: " + ofToString(gradientMode) + " " + gradientModeStr + " (" + ofToString(spaceNumCells.x) + ", " + ofToString(spaceNumCells.y) + ", " + ofToString(spaceNumCells.z) + ")");
}
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
    ofSetLogLevel(OF_LOG_VERBOSE);
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
    inputWidth = kinect.getWidth();
    inputHeight = kinect.getHeight();
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
    
//    particle.getVertices().resize(NUM_BILLBOARDS);
//	particle.getColors().resize(NUM_BILLBOARDS);
//	particle.getNormals().resize(NUM_BILLBOARDS,ofVec3f(0));
//	
//	// ------------------------- billboard particles
//	for (int i=0; i<NUM_BILLBOARDS; i++) {
//		
//		billboardVels[i].set(ofRandomf(), -1.0, ofRandomf());
//        particle.getVertices()[i].set(ofRandom(-500, 500),
//                                      ofRandom(-500, 500),
//                                      ofRandom(-500, 500));
//		
//		particle.getColors()[i].set(ofColor::fromHsb(ofRandom(96, 160), 0, 255));
//	    
//		
//	}
//    particle.setUsage( GL_DYNAMIC_DRAW );
//	particle.setMode(OF_PRIMITIVE_POINTS);
    spaceTime.setMaxFrames(numScanFrames);
    
}
void testApp::onPictureTaken(roxlu::CanonPictureEvent& ev) {
	cout << ev.getFilePath() << endl;
}
//--------------------------------------------------------------
void testApp::update(){
    if(!canon.isLiveViewActive() && canon.isSessionOpen() && timeTry<5) {
		canon.startLiveView();
        timeTry++;
        if(timeTry==5)
        {
            ofLogError("ofxCanon") << "5 time faile to try stop reconnect";
        }
	}
    if(kinect.isConnected())
    {
        kinect.update();
        
        // there is a new frame and we are connected
        if(kinect.isFrameNew())
        {
            if(doPause == false) {
                if(mode==SLITSCAN) {
                    // construct space time continuum
                    msa::SpaceT<ofMesh> *space = new msa::SpaceT<ofMesh>(spaceNumCells, spaceBoundaryMin, spaceBoundaryMax);
                    
                    ofPixelsRef pixelsRef = kinect.getPixelsRef();// grabber->getPixelsRef();
                    // iterate all vertices of mesh, and add to relevant quantum cells
                    for(int j=0; j<inputHeight; j += pixelStep) {
                        for(int i=0; i<inputWidth; i += pixelStep) {
                            ofVec3f p;
                            ofFloatColor c;
                            bool doIt;

                            p = kinect.getWorldCoordinateAt(i, j);
                            c = kinect.getColorAt(i, j);
                            doIt = kinect.getDistanceAt(i, j) > 0;

                            if(ofInRange(p.z, nearThreshold, farThreshold) && doIt) {
                                ofVec3f index = space->getIndexForPosition(p);
                                ofMesh &cellMesh = space->getDataAtIndex(index);
                                cellMesh.addVertex(p);
                                cellMesh.addColor(c);
                            }
                        }
                    }
                    
                    // add space to space time continuum
                    spaceTime.addSpace(space);
                    
                    // update mesh
                    scanMesh.clear();
                    for(int i=0; i<spaceNumCells.x; i++) {
                        for(int j=0; j<spaceNumCells.y; j++) {
                            for(int k=0; k<spaceNumCells.z; k++) {
                                float t;
                                switch(gradientMode) {
                                    case 0:
                                        t = 0;  // use most recent mesh
                                        break;
                                        
                                    case 1:
                                        t = i * 1.0f/spaceNumCells.x; // left to right
                                        break;
                                        
                                    case 2:
                                        t = 1.0f - i * 1.0f/spaceNumCells.x; // right to left
                                        break;
                                        
                                    case 3:
                                        t = j * 1.0f/spaceNumCells.y; // up to down
                                        break;
                                        
                                    case 4:
                                        t = 1.0f - j * 1.0f/spaceNumCells.y; // down to up
                                        break;
                                        
                                    case 5:
                                        t = k * 1.0f/spaceNumCells.z; // front to back
                                        break;
                                        
                                    case 6:
                                        t = 1.0f - k * 1.0f/spaceNumCells.z; //  back to front
                                        break;
                                        
                                    case 7:
                                    {
                                        // spherical
                                        float tx = i/spaceNumCells.x * 2 - 1;
                                        float ty = j/spaceNumCells.y * 2 - 1;
                                        float tz = k/spaceNumCells.z * 2 - 1;
                                        t = tx * tx + ty * ty + tz * tz;
                                        //                                    t = sqrt(t);
                                    }
                                        break;
                                        
                                    case 8:
                                        t = ofRandomuf();
                                        break;
                                        
                                    case 9:
                                        t = 1;
                                        break;
                                        
                                }
                                
                                t = ofClamp(t, 0, 1);
                                
                                ofMesh &cellMesh = spaceTime.getSpaceAtTime(t)->getDataAtIndex(i, j, k);
                                
                                scanMesh.addVertices(cellMesh.getVertices());
                                scanMesh.addColors(cellMesh.getColors());
                                

                            } // k
                        } // j
                    } // i
                    
                }
            }
        }
    }
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
//    float t = (ofGetElapsedTimef()) * 0.9f;
//	float div = 250.0;
//	
//	for (int i=0; i<NUM_BILLBOARDS; i++) {
//		
//		// noise
//		ofVec3f vec(ofSignedNoise(t, particle.getVertex(i).y/div, particle.getVertex(i).z/div),
//                    ofSignedNoise(particle.getVertex(i).x/div, t, particle.getVertex(i).z/div),
//                    ofSignedNoise(particle.getVertex(i).x/div, particle.getVertex(i).y/div, t));
//		
//		vec *= 10 * ofGetLastFrameTime();
//		billboardVels[i] += vec;
//		particle.getVertices()[i] += billboardVels[i];
//		billboardVels[i] *= 0.94f;
//    	particle.setNormal(i,ofVec3f(10,0,0));
//	}
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
//	billboardShader.begin();
//	ofEnablePointSprites();
//	texture.getTextureReference().bind();
//	particle.draw();
//	texture.getTextureReference().unbind();
//	ofDisablePointSprites();
//    billboardShader.end();
	
    
    // end scene and draw
    //    if(mode == POINT|| mode == TRIANGLE)
    //    {
    post.end();
    // set gl state back to original
    glPopAttrib();
    
//
    
    //    }else if(BILLBOARD == mode)
    //    {
    //        cam.end();
    //    }
    
    if(canon.isCameraConnected()&& canonDraw)
    {
        if(canon.isLiveViewActive() )
        {
            float scale = 1;
            if(canon.getLivePixels().isAllocated())ofGetWidth()/canon.getLivePixels().getWidth();
            ofPushMatrix();
            ofScale(scale, scale);
            canon.drawLiveView();
            ofPopMatrix();
        }
        else{
            canon.startLiveView();
        }
    }
    glDisable(GL_DEPTH_TEST);
}
void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
//    if(mode==TRIANGLE)
//    {
//        triangulation.reset();
//        
//        int step = 10;
//        for(int y = 0; y < h; y += step) {
//            for(int x = 0; x < w; x += step) {
//                if(kinect.getDistanceAt(x, y) > minRange && kinect.getDistanceAt(x, y) < maxRange)
//                {
//                    triangulation.addPoint(kinect.getWorldCoordinateAt(x, y));
//                    
//                }
//            }
//        }
//        triangulation.triangulate();
//        ofSetColor(255);
//        ofSetLineWidth(1);
//        ofPushMatrix();
//        // the projected points are 'upside down' and 'backwards'
//        ofScale(1, -1, -1);
//        ofTranslate(0, 0, meshDistance); // center the points a bit
//        glEnable(GL_DEPTH_TEST);
//        triangulation.triangleMesh.drawWireframe();
//        triangulation.triangleMesh.drawVertices();
//        glDisable(GL_DEPTH_TEST);
//        ofPopMatrix();
//    }
//    else
    if(mode==POINT )//|| mode ==BILLBOARD)
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
                    
//                    if(mode==BILLBOARD)
//                    {
                        mesh.addNormal(ofVec3f(bbNormal,0,0));
//                    }
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
        
//        if(mode==BILLBOARD)
//        {
            billboardShader.begin();
            ofEnableAlphaBlending();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            ofEnablePointSprites();
            texture.getTextureReference().bind();
            mesh.draw();
            
            texture.getTextureReference().unbind();
            ofDisablePointSprites();
            
            billboardShader.end();
//
//        }
//        else if (mode == POINT){
//            glPointSize(5);
//            mesh.drawVertices();
//        }
        
        
        ofPopMatrix();
    }
    else if (mode == SLITSCAN)
    {
        glPointSize(5);
        ofPushMatrix();
        // the projected points are 'upside down' and 'backwards'
        ofScale(1, -1, -1);
//        float s = 0.8;
//        ofScale(s, s, s);
        ofTranslate(0, 0, -1000); // center the points a bit
        glEnable(GL_DEPTH_TEST);
        
        scanMesh.drawVertices();
        
        glDisable(GL_DEPTH_TEST);
        ofPopMatrix();
        
        
    }
//        else if (mode ==DISPLACEMENT)
//    {
//        ofVboMesh mesh;
//        
//        mesh.setUsage( GL_DYNAMIC_DRAW );
//        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
//        
//        int step = 2;
//        for(int y = 0; y < h-step; y += step) {
//            for(int x = 0; x < w-step; x += step) {
//                float nwDis = kinect.getDistanceAt(x, y);
//                float neDis = kinect.getDistanceAt(x+step, y);
//                float swDis = kinect.getDistanceAt(x, y+step);
//                float seDis = kinect.getDistanceAt(x+step, y+step);
//                
//                if( nwDis> minRange && nwDis < maxRange &&
//                   neDis> minRange && neDis < maxRange &&
//                   swDis> minRange && swDis < maxRange &&
//                   seDis> minRange && seDis < maxRange )
//                {
//                    //                    mesh.addColor(kinect.getColorAt(x,y));
//                    //                    mesh.addVertex();
//                    ofVec3f nw = kinect.getWorldCoordinateAt(x, y);//ofVec3f( x, y , 0);
//                    ofVec3f ne = kinect.getWorldCoordinateAt(x+step, y);//ofVec3f( x + step, y, 0);
//                    ofVec3f sw = kinect.getWorldCoordinateAt(x, y+step);//ofVec3f( x, y + step, 0);
//                    ofVec3f se =kinect.getWorldCoordinateAt(x+step, y+step) ;//ofVec3f( x + step, y + step, 0);
//                    
//                    addFace(mesh, nw, ne, se, sw);
//                }
//            }
//        }
//        
//        ofPushMatrix();
//        ofScale(1, -1, -1);
//        
//        ofTranslate(0, 0, meshDistance); // center the points a bit
//        kinect.getTextureReference().bind();
//        mesh.draw();
//        kinect.getTextureReference().unbind();
//        //        mesh.drawWireframe();
//        //        mesh.drawVertices();
//        
//        
//        ofPopMatrix();
//        
//    }
    
    
    
    
    
    
}
void testApp::exit() {
    canon.endLiveView();
    
    gui1->saveSettings("GUI/GUI1_Settings.xml");
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}
void testApp::setGUI1()
{
    
	
    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = (ofGetWidth()*0.25)-xInit;
    int dim = 16;
    
	
    vector<string> names;
    
	gui1 = new ofxUICanvas(0, 0, ofGetWidth()*0.5, ofGetHeight());
    gui1->addFPS();
	gui1->addWidgetDown(new ofxUILabel("PANEL 1: BASICS", OFX_UI_FONT_MEDIUM));
    gui1->addToggle("ENABLE_OSC",&bOsc);
    gui1->addSlider("MIN_RANGE", 0, 10000, &minRange, length-xInit, dim);
    gui1->addSlider("MAX_RANGE", 0, 10000, &maxRange, length-xInit, dim);
    gui1->addSlider("CAM_DISTANCE", 0, 10000, & camDistance, length-xInit, dim);
    gui1->addSlider("MESH_DISTANCE", -10000, 10000, & meshDistance, length-xInit, dim);
    gui1->addSlider("BILLBOARD_NORM",0, 100, &bbNormal, length-xInit, dim);
    gui1->addToggle("DRAW_CANOM", &canonDraw);
    renderMode.push_back("POINT");
        renderMode.push_back("SLITSCAN");
    gui1->addRadio("RENDER_MODE", renderMode);
    gui1->addSpacer(length-xInit, dim);
    gui1->addLabel("SLITSCAN",OFX_UI_FONT_MEDIUM);
//    gui1->addToggle("SLITSCAN",&doSlitScan);
//    gui1->addToggle("USE_KINECT",&usingKinect);   // using kinect or webcam
    gui1->addToggle("PAUSE", &doPause);
    gui1->addSlider("KINECT_ANGLE",-30,30,kinectAngle, length-xInit, dim);

    gradientModeRadioOption.push_back("0: most recent");
    gradientModeRadioOption.push_back("1: left-right");
    gradientModeRadioOption.push_back("2: right-left");
    gradientModeRadioOption.push_back("3: top-bottom");
    gradientModeRadioOption.push_back("4: bottom-top");
    gradientModeRadioOption.push_back("5: front-back");
    gradientModeRadioOption.push_back("6: back-front");
    gradientModeRadioOption.push_back("7: spherical");
    gradientModeRadioOption.push_back("8: random");
    gradientModeRadioOption.push_back("9: oldest");
    gui1->addRadio("GRADIENT_MODE", gradientModeRadioOption);

    ofxUILabel * lable = new ofxUILabel("SHADER",OFX_UI_FONT_LARGE);
    gui1->addWidgetEastOf (lable,"PANEL 1: BASICS");
	for (unsigned i = 0; i < post.size(); ++i)
    {

		gui1->addWidgetPosition(new ofxUILabelToggle(post[i]->getName(), false, length-xInit),
                               OFX_UI_WIDGET_POSITION_DOWN,
                               OFX_UI_ALIGN_RIGHT,
							   false);
	}
    
	ofAddListener(gui1->newGUIEvent,this,&testApp::guiEvent);
    gui1->setVisible(false);
    gui1->loadSettings("GUI/GUI1_Settings.xml");
}
void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
	cout << "got event from: " << name << endl;
    if(kind == OFX_UI_WIDGET_TOGGLE)
    {
        for(int  i = 0; i < gradientModeRadioOption.size() ; i++)
        {
            if(name == gradientModeRadioOption[i])
            {
                setGradientMode(i);
            }
        }
        
            if(name =="POINT")
            {
                mode = POINT;
            }else if(name == "SLITSCAN")
            {
                mode = SLITSCAN;
            }
    }
    else{
        for (unsigned i = 0; i < post.size(); ++i)
		{
            
			if(post[i]->getName()==e.widget->getName())
			{
				post[i]->setEnabled(((ofxUIToggle*)e.widget)->getValue());
			}
		}
    }
    
    
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key)
    {
        case 27:
            canon.endLiveView();
            break;
        case ' ':
            
            
            
            ofDisableArbTex();
            
            texture.loadImage(dir.getPath(int(ofRandom(numEntry))));
            ofEnableArbTex();
            
            break;
        case 'p':
            mode = POINT;
            break;
//        case 't':
//            mode = TRIANGLE;
//            break;
//        case 'b':
//            mode = BILLBOARD;
//            break;
//        case 'd':
//            mode = DISPLACEMENT;
//            break;
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