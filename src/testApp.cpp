#include "testApp.h"
#include "MSASpaceTime.h"
const float FovX = 1.0144686707507438;
const float FovY = 0.78980943449644714;
const float XtoZ = tanf(FovX / 2) * 2;
const float YtoZ = tanf(FovY / 2) * 2;
const unsigned int Xres = 640;
const unsigned int Yres = 480;
int pixelStep = 4;          // how many pixels to step through the depth map when iterating
int numScanFrames = 240;    // duration (in frames) for full scan

// physical boundaries of space time continuum
ofVec3f spaceBoundaryMin;
ofVec3f spaceBoundaryMax;

// spatial resolution for space time continuum
ofVec3f spaceNumCells;


bool doPause = false;
bool doDrawPointCloud = true;
bool doSlitScan = true;


int gradientMode = 0;
string gradientModeStr = "";

int kinectAngle;
float inputWidth, inputHeight;

msa::SpaceTime<ofMesh> spaceTime;   // space time

GLUquadricObj *quadric;
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
ofVec3f ConvertProjectiveToRealWorld(float x, float y, float z) {
	return ofVec3f((x / Xres - .5f) * z * XtoZ,
                   (y / Yres - .5f) * z * YtoZ,
                   z);
}

ofVec3f getNormal(ofVec3f& a, ofVec3f& b, ofVec3f& c) {
	ofVec3f side1 = a - b;
	ofVec3f side2 = c - b;
	ofVec3f normal = side1.cross(side2);
	normal.normalize();
	return normal;
}
//--------------------------------------------------------------
void testApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    // load the bilboard shader
	// this is used to change the
	// size of the particle
    displacement.load("simplicity");
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
    sender.setup("",7171);
    // Setup post-processing chain
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    Ptr = post.createPass<DofPass>();
    Ptr->setEnabled(false);
    //    post.createPass<KaleidoscopePass>()->setEnabled(false);
    //    post.createPass<NoiseWarpPass>()->setEnabled(false);
    //    post.createPass<PixelatePass>()->setEnabled(false);
    //    post.createPass<EdgePass>()->setEnabled(false);
    
    orbit.set(0,0,0);
    
    mode = POINT;
    nearThreshold = 0;
    farThreshold = 10000;
    setGUI1();
    spaceBoundaryMin    = ofVec3f(-400, -400, 400);
    spaceBoundaryMax    = ofVec3f(400, 400, farThreshold);
    
	
    
    ofDisableArbTex();
    colormap.loadImage("color.png");
    bumpmap.loadImage("bump.png");
    ofEnableArbTex();
    quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    
    displacement.begin();
    //	displacement.setUniform1f("brightness", 1);
    //	displacement.setUniform1f("contrast", 1);
    //	displacement.setUniform1f("saturation", 0.5);
    //	displacement.setUniform1i("invert",false);
    //	displacement.setUniform1f("alpha", 1.0 );
    //	displacement.setUniform1i("tex", 0);
    //	shader.setUniformTexture("tex", fbo.getTextureReference(), 1);
	displacement.end();
	billboardShader.load("Billboard");
    dir.allowExt("png");
	numEntry = dir.listDir("images/");
    
	// we need to disable ARB textures in order to use normalized texcoords
	ofDisableArbTex();
	texture.loadImage(dir.getPath(0));
    ofEnableArbTex();
	ofEnableAlphaBlending();
    
    for (int j=0; j<NUM_STRIP; j++)
    {
        pos[j].set(ofRandom(0, ofGetWidth()),ofRandom(0,ofGetHeight()),0);
        vec[j].set(0,0,0);
        acc[j].set(ofRandom(-1,1),ofRandom(-1,1),0);
        age[j] = 1;
        float h = ofRandom(100,200);
        for (int i=0; i<LENGTH; i++)
        {
            int index = i+(j*LENGTH);
            strip[index].set(ofGetWidth()*0.5,ofGetHeight()*0.5,0);
            float brightness = sinf(PI*float((i*0.5)*1.f/LENGTH*0.5f))*255;
            color[index].set(ofColor::fromHsb(h,255, 255,brightness));
        }
        
        for (int i=0; i<LOC_LENGTH; i++)
        {
            int index = i+(j*LOC_LENGTH);
            loc[index].set(0,0,0);
        }
        
        
    }
    total = NUM_STRIP*LENGTH;
    vbo.setVertexData(strip, total, GL_DYNAMIC_DRAW);
	vbo.setColorData(color, total, GL_DYNAMIC_DRAW);
    count = 0;
    
    spaceTime.setMaxFrames(numScanFrames);
    setGradientMode(0);
    
    port = 12345;
    duration.setup(port);
	//ofxDuration is an OSC receiver, with special functions to listen for Duration specific messages
	//optionally set up a font for debugging
	duration.setupFont("GUI/NewMedia Fett.ttf", 12);
	ofAddListener(duration.events.trackUpdated, this, &testApp::trackUpdated);
}
//--------------------------------------------------------------

void testApp::trackUpdated(ofxDurationEventArgs& args){
	ofLogVerbose("Duration Event") << "track type " << args.track->type << " updated with name " << args.track->name << " and value " << args.track->value << endl;
    if (args.track->name == "/CENTER_X") {
        centerPoint.x = args.track->value;
    }else if (args.track->name == "/CENTER_Y") {
        centerPoint.y = args.track->value;
    }
    else if (args.track->name == "/CENTER_Z") {
        centerPoint.z = args.track->value;
    }
    else if (args.track->name == "/RGB_ALPHA") {
        rgbAlpha = args.track->value;
    }
}

//--------------------------------------------------------------
void testApp::update(){
    float t = (ofGetElapsedTimef()) * 0.9f;
    float div = 250.0;
    
    for (int j=0; j<NUM_STRIP; j++)
    {
        if(age[j]>0)
        {
            ofVec3f _vec(ofSignedNoise(t, pos[j].y/div, pos[j].z/div),
                         ofSignedNoise(pos[j].x/div, t, pos[j].z/div),
                         ofSignedNoise(pos[j].x/div, pos[j].y/div,t));
            _vec *=  ofGetLastFrameTime()*50;
            vec[j]+=_vec;
            acc[j] = (attraction-acc[j])*0.1;
            vec[j]+=acc[j];
            vec[j]*=0.9;
            ofVec3f Off;
            float radius = 10;
            for (int i=LOC_LENGTH-1; i>=1; i--)
            {
                int index = i+(j*LOC_LENGTH);
                loc[index].set(loc[index-1]);
            }
            for (int i=0; i<LOC_LENGTH; i++)
            {
                int index = i+(j*LOC_LENGTH);
                int index2 = (i*2)+(j*LENGTH);
                
                
                radius = sinf(PI*float(i*1.f/LOC_LENGTH))*15;
                {
                    ofVec3f perp0 = loc[index] - loc[index+1];
                    ofVec3f perp1 = perp0.getCrossed( ofVec3f( 0, 1, 0 ) ).getNormalized();
                    ofVec3f perp2 = perp0.getCrossed( perp1 ).getNormalized();
                    perp1 = perp0.getCrossed( perp2 ).getNormalized();
                    Off.x        = perp1.x * radius*age[j];
                    Off.y       = perp1.y * radius*age[j];
                    Off.z        = perp1.z * radius*age[j];
                    
                    strip[(index2)]=loc[index]-Off;
                    
                    strip[(index2+1)]=loc[index]+Off;
                }
            }
            loc[j*LOC_LENGTH] = pos[j];
            pos[j]+=vec[j];
            age[j]-=0.02;
        }
        else
        {
            for (int i=0; i<LOC_LENGTH; i++)
            {
                int index = i+(j*LOC_LENGTH);
                loc[index].set(pos[j]);
                int index2 = (i*2)+(j*LENGTH);
                strip[(index2)]=loc[index];
                
                strip[(index2+1)]=loc[index];
            }
        }
        
        
    }
    if(kinect.isConnected())
    {
        kinect.update();
        
        // there is a new frame and we are connected
        if(kinect.isFrameNew())
        {
            if(doSlitScan && mode == SLITSCAN) {
                // construct space time continuum
                msa::SpaceT<ofMesh> *space = new msa::SpaceT<ofMesh>(spaceNumCells, spaceBoundaryMin, spaceBoundaryMax);
                
                
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
            if (mode == DISPLACEMENT)
            {
                
                int width = kinect.getWidth();
                int height = kinect.getHeight();
                float* distancePixels = kinect.getDistancePixels(); // distance in centimeters
                mesh.clear();
                mesh.setMode(OF_PRIMITIVE_TRIANGLES);
                float maxZ = 9999;
                for(int y = 0; y < height - 1; y++) { // don't go to the end
                    for(int x = 0; x < width - 1; x++) { // don't go to the end
                        
                        // get indices for each corner
                        int nwi = (y + 0) * width + (x + 0);
                        int nei = (y + 0) * width + (x + 1);
                        int sei = (y + 1) * width + (x + 1);
                        int swi = (y + 1) * width + (x + 0);
                        
                        // get z values for each corner
                        float nwz = distancePixels[nwi];
                        float nez = distancePixels[nei];
                        float sez = distancePixels[sei];
                        float swz = distancePixels[swi];
                        
                        if(nwz > 0 && nez > 0 && sez > 0 && swz > 0 && nwz>nearThreshold && nwz<
                           farThreshold) {
                            
                            // ignore empty depth pixels
                            // get real world locations for each corner
                            ofVec3f nwv = ConvertProjectiveToRealWorld(x + 0, y + 0, nwz);
                            ofVec3f nev = ConvertProjectiveToRealWorld(x + 1, y + 0, nez);
                            ofVec3f sev = ConvertProjectiveToRealWorld(x + 1, y + 1, sez);
                            ofVec3f swv = ConvertProjectiveToRealWorld(x + 0, y + 1, swz);
                            if(maxZ<nwz)attraction =  nwv;
                            // compute normal for the upper left
                            ofVec3f normal = getNormal(nwv, nev, swv);
                            
                            // add the upper left triangle
                            mesh.addNormal(normal);
                            mesh.addTexCoord(ofVec2f(x,y));
                            mesh.addVertex(nwv);
                            mesh.addNormal(normal);
                            mesh.addTexCoord(ofVec2f(x+1,y));
                            mesh.addVertex(nev);
                            mesh.addNormal(normal);
                            mesh.addTexCoord(ofVec2f(x,y+1));
                            mesh.addVertex(swv);
                            
                            // add the bottom right triangle
                            mesh.addNormal(normal);
                            mesh.addTexCoord(ofVec2f(x+1,y));
                            mesh.addVertex(nev);
                            mesh.addNormal(normal);
                            mesh.addTexCoord(ofVec2f(x+1,y+1));
                            mesh.addVertex(sev);
                            mesh.addNormal(normal);
                            mesh.addTexCoord(ofVec2f(x,y+1));
                            mesh.addVertex(swv);
                        }
                    }
                }
            }
            else if(mode==TRIANGLE)
            {
                triangulation.reset();
                
                int step = 10;
                int w = kinect.getWidth();
                int h = kinect.getHeight();
                float maxZ = 9999;
                for(int y = 0; y < h; y += step) {
                    for(int x = 0; x < w; x += step) {
                        if(kinect.getDistanceAt(x, y) > nearThreshold && kinect.getDistanceAt(x, y) < farThreshold)
                        {
                            
                            ofVec3f cur = kinect.getWorldCoordinateAt(x, y);
                            if(maxZ<cur.z)attraction = cur;
                            
                            triangulation.addPoint(kinect.getWorldCoordinateAt(x, y));
                            
                        }
                    }
                }
                triangulation.triangulate();
            }
            else if(mode==POINT )
            {
                
                mesh.clear();
                
                mesh.setMode(OF_PRIMITIVE_POINTS);
                int w = kinect.getWidth();
                int h = kinect.getHeight();
                int step = 2;
                float maxZ = 9999;
                for(int y = 0; y < h; y += step) {
                    for(int x = 0; x < w; x += step) {
                        if(kinect.getDistanceAt(x, y) > nearThreshold && kinect.getDistanceAt(x, y) < farThreshold)
                        {
                            ofVec3f cur = kinect.getWorldCoordinateAt(x, y);
                            if(maxZ<cur.z)attraction =  cur;
                            mesh.addColor(kinect.getColorAt(x,y));
                            mesh.addVertex(cur);
                            
                            mesh.addNormal(ofVec3f(bbNormal,0,0));
                            
                        }
                    }
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
                //                orbit.x += (m.getArgAsInt32(0)-orbit.x)*0.1f;
                orbit.y += (m.getArgAsInt32(1)-orbit.y)*0.1f;
                //                orbit.z += (m.getArgAsInt32(2)-orbit.z)*0.1f;
            }
            else if(m.getAddress() == "/mode"){
                int _mode = m.getArgAsInt32(0);
                switch(_mode)
                {
                    case 0:
                        mode = POINT;
                        break;
                    case 1:
                        mode = DISPLACEMENT;
                        break;
                    case 2:
                        mode = SLITSCAN;
                        break;
                    case 3:
                        mode = TRIANGLE;
                        break;
                    default:
                        mode = POINT;
                        break;
                        
                }
                ofxUIRadio* radio = (ofxUIRadio*)gui1->getWidget("RENDER_MODE");
                vector<ofxUIToggle*> toggles = radio->getToggles();
                for(int i = 0 ; i < toggles.size() ; i++)
                {
                    if(mode == i)
                    {
                        toggles[i]->setValue(true);
                    }
                    else{
                        toggles[i]->setValue(false);
                    }
                }
            }
            else if(m.getAddress() == "/slitscan"){
                string _mode = m.getArgAsString(0);
                for(int  i = 0; i < gradientModeRadioOption.size() ; i++)
                {
                    if(_mode == gradientModeRadioOption[i])
                    {
                        ofxUIRadio* radio = (ofxUIRadio*)gui1->getWidget("GRADIENT_MODE");
                        vector<ofxUIToggle*> toggles = radio->getToggles();
                        for(int j = 0 ; j < toggles.size() ; j++)
                        {
                            if(j == i)
                            {
                                toggles[j]->setValue(true);
                            }
                            else{
                                toggles[j]->setValue(false);
                            }
                        }
                        setGradientMode(i);
                    }
                }
                
                
            }
            else if(m.getAddress() == "/mouse"){
                float x = m.getArgAsFloat(0)*ofGetWidth();
                float y = m.getArgAsFloat(1)*ofGetHeight();
                float vx = m.getArgAsFloat(2);
                float vy = m.getArgAsFloat(3);
                
                fireStrip( x , y,  vx, vy);
                
            }
        }
        cam.lookAt(ofVec3f(0,0,meshDistance));
        cam.setPosition(centerPoint);
        //        cam.orbit(0,0, camDistance,centerPoint);
    }
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    ofBackground(0);
    
    
    glPushAttrib(GL_ENABLE_BIT);
    
    // setup gl state
    glEnable(GL_DEPTH_TEST);
    
    post.begin(cam);
    
    if(kinect.isConnected())
    {
        drawPointCloud();
    }
    
    post.end();
    
    glPopAttrib();
    
    
    glDisable(GL_DEPTH_TEST);
    ofPushStyle();
    ofSetColor(0,0,0,rgbAlpha);
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    ofPopStyle();
    if(rgbAlpha>0)
    {
        ofPushStyle();
        ofSetColor(255, 255, 255,rgbAlpha);
        
        ofPushMatrix();
        // the projected points are 'upside down' and 'backwards'
        ofTranslate(rgbPos.x,rgbPos.y);
        ofRotateX(orbit.x);
        ofRotateY(orbit.y);
        ofRotateZ(orbit.z);
        
        kinect.draw(-320,-240 , 640,480);
        ofPopMatrix();
        ofPopStyle();
    }
    vbo.bind();
	vbo.updateVertexData(strip, total);
	vbo.updateColorData(color, total);
    
    
    for (int j=0; j<NUM_STRIP; j++)
        
    {
        int index = j * LENGTH;
        
        vbo.draw(GL_TRIANGLE_STRIP, index,LENGTH);
        
    }
    
    
	vbo.unbind();
    if(ofGetLogLevel()==OF_LOG_VERBOSE)
    {
        
        glEnable(GL_DEPTH_TEST);
        
        displacement.begin();
        ofPushMatrix();
        displacement.setUniformTexture("colormap", colormap, 1);
        displacement.setUniformTexture("bumpmap", bumpmap, 2);
        displacement.setUniform1i("maxHeight",ofGetMouseX());
        displacement.setUniform1f("iGlobalTime",ofGetElapsedTimef());
        displacement.setUniform3f("iResolution",ofGetWidth(),ofGetHeight(),0);
        
        displacement.setUniform1f("iGlobalTimeX",ofSignedNoise(ofGetElapsedTimef()));
        displacement.setUniform1f("iGlobalTimeY",ofSignedNoise(ofGetElapsedTimef()));
        ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
        ofRotateY(360*sinf(float(ofGetFrameNum())/500.0f));
        ofRotate(-90,1,0,0);
        gluSphere(quadric, 150, 400, 400);
        ofPopMatrix();
        displacement.end();
        
        
        glDisable(GL_DEPTH_TEST);
        if(kinect.isConnected())
        {
            kinect.drawDepth(0, 0, 320,240);
            kinect.draw(320, 0, 320, 240);
        }
        duration.draw(0,0, ofGetWidth(), ofGetHeight());
    }
}
void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    
    ofRotateX(orbit.x);
    ofRotateY(orbit.y);
    ofRotateZ(orbit.z);
    ofScale(1, -1, -1);
    ofTranslate(0, 0, meshDistance); // center the points a bit
    
    if(mode==TRIANGLE)
    {
        
        ofSetColor(255);
        ofSetLineWidth(1);
        
        
        triangulation.triangleMesh.drawWireframe();
        triangulation.triangleMesh.drawVertices();
        
    }
    else
        if(mode==POINT )
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
        else if (mode == SLITSCAN)
        {
            glPointSize(5);
            
            scanMesh.drawVertices();
            
            
            
        }
        else if (mode == DISPLACEMENT)
        {
            
            displacement.begin();
            displacement.setUniformTexture("colormap", colormap, 1);
            displacement.setUniformTexture("bumpmap", bumpmap, 2);
            displacement.setUniform1i("maxHeight",maxHeight);
            displacement.setUniform1f("iGlobalTimeX",ofGetElapsedTimef());
            displacement.setUniform1f("iGlobalTimeY",ofSignedNoise(ofGetElapsedTimef()));
            //            kinect.getTextureReference().bind();
            mesh.draw();
            //            kinect.getTextureReference().unbind();
            displacement.end();
            
            
        }
    
    ofPopMatrix();
    
    
    
    
}
void testApp::fireStrip(float x ,float y, float vx, float vy)
{
    int ran = ofRandom(1,3);
    
    for(int j = 0 ; j < ran ; j++)
    {
        count++;
        
        count%=NUM_STRIP;
        sin(ofRandomf()*TWO_PI)*50;
        pos[count].set(x+sin(ofRandomf()*TWO_PI)*ofRandom(-50,50), y+cos(ofRandomf()*TWO_PI)*ofRandom(-50,50));
        vec[count].set(vx,vy,0);
        vec[count]*=2;
        acc[count].set((x-pos[count].x)*0.01, (y-pos[count].y)*0.01);
        age[count] = 1;
        
        for (int i=0; i<LOC_LENGTH; i++)
        {
            int index = i+(count*LOC_LENGTH);
            loc[index].set(pos[count]);
            int index2 = (i*2)+(count*LENGTH);
            strip[(index2)]=loc[index];
            
            strip[(index2+1)]=loc[index];
            
            
        }
        
        
        
    }
}
void testApp::exit() {
    //    canon.endLiveView();
    
    gui1->saveSettings("GUI/GUI1_Settings.xml");
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}
void testApp::setGUI1()
{   float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = (ofGetWidth()*0.25)-xInit;
    int dim = 16;
    
	
    vector<string> names;
    
	gui1 = new ofxUICanvas(0, 0, ofGetWidth()*0.5, ofGetHeight());
    gui1->addFPS();
	gui1->addWidgetDown(new ofxUILabel("PANEL 1: BASICS", OFX_UI_FONT_MEDIUM));
    gui1->addToggle("ENABLE_OSC",&bOsc);
    gui1->addSlider("KINECT_ANGLE",-30.0,30.0,0.0,length-xInit, dim);
    gui1->addSlider("centerPoint_X",-10000,10000,&centerPoint.x,length-xInit, dim);
    gui1->addSlider("centerPoint_Y",-10000,10000,&centerPoint.y,length-xInit, dim);
    gui1->addSlider("centerPoint_Z",-10000,10000,&centerPoint.z,length-xInit, dim);
    gui1->addSlider("nearThreshold", 0, 10000, &nearThreshold, length-xInit, dim);
    gui1->addSlider("farThreshold", 0, 3000, &farThreshold, length-xInit, dim);
    gui1->addSlider("CAM_DISTANCE", 0, 10000, & camDistance, length-xInit, dim);
    gui1->addSlider("MESH_DISTANCE", -10000, 10000, & meshDistance, length-xInit, dim);
    gui1->addSlider("depthScale", 0, 1, &depthScale, length-xInit, dim);
    gui1->addSlider("BILLBOARD_NORM",-100, 100, &bbNormal, length-xInit, dim);
    //    gui1->addToggle("DRAW_CANOM", &canonDraw);
    renderMode.push_back("POINT");
    renderMode.push_back("DISPLACEMENT");
    renderMode.push_back("TRIANGLE");
    renderMode.push_back("SLITSCAN");
    gui1->addRadio("RENDER_MODE", renderMode);
    gui1->addSlider("maxHeight", -100,100, &maxHeight);
    gui1->addLabel("SLITSCAN",OFX_UI_FONT_MEDIUM);
    //    gui1->addToggle("SLITSCAN",&doSlitScan);
    //    gui1->addToggle("USE_KINECT",&usingKinect);   // using kinect or webcam
    gui1->addToggle("PAUSE", &doPause);
    
    gui1->addSlider("KINECT_ALPHA",0,255,&rgbAlpha, length-xInit, dim);
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
    gui1->addWidgetPosition(new ofxUISlider("DOF_FOCUS", -2.0,2.0,0.0, length-xInit,dim),
                            OFX_UI_WIDGET_POSITION_DOWN,
                            OFX_UI_ALIGN_RIGHT,
                            false);
    gui1->addWidgetPosition(new ofxUISlider("RGB_POS_X", -1024,1024,&rgbPos.x, length-xInit,dim),
                            OFX_UI_WIDGET_POSITION_DOWN,
                            OFX_UI_ALIGN_RIGHT,
                            false);
    gui1->addWidgetPosition(new ofxUISlider("RGB_POS_Y", -1024,1024,&rgbPos.y, length-xInit,dim),
                            OFX_UI_WIDGET_POSITION_DOWN,
                            OFX_UI_ALIGN_RIGHT,
                            false);
    
    vector<string>logLevel;
    logLevel.push_back("OF_LOG_VERBOSE");
    logLevel.push_back("OF_LOG_NOTICE");
    logLevel.push_back("OF_LOG_WARNING");
    logLevel.push_back("OF_LOG_ERROR");
    logLevel.push_back("OF_LOG_FATAL_ERROR");
    logLevel.push_back("OF_LOG_SILENT");
    //    ofxUIRadio("LOG_LEVEL", logLevel);
    
    gui1->addWidgetPosition(new ofxUIRadio("LOG_LEVEL", logLevel, OFX_UI_ORIENTATION_VERTICAL, dim, dim),
                            OFX_UI_WIDGET_POSITION_DOWN,
                            OFX_UI_ALIGN_RIGHT,
                            false);
    
    
	ofAddListener(gui1->newGUIEvent,this,&testApp::guiEvent);
    gui1->setVisible(false);
    gui1->loadSettings("GUI/GUI1_Settings.xml");
}
void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
	ofLogVerbose("guiEvent") << "got event from: " << name << endl;
    
    if(name == "DOF_FOCUS")
    {
        Ptr->setFocus(((ofxUISlider*)e.widget)->getScaledValue());
        //        ((shared_ptr<DofPass> )post[dofIndex])->setFocus(((ofxUISlider*)e.widget)->getScaledValue());
    }
    else if(name == "farThreshold")
    {
        spaceBoundaryMax    = ofVec3f(400, 400, farThreshold);
    }
    else if(name == "KINECT_ANGLE")
    {
        kinect.setCameraTiltAngle(((ofxUISlider*)e.widget)->getScaledValue());
    }
    else if(kind == OFX_UI_WIDGET_TOGGLE)
    {
        for(int  i = 0; i < gradientModeRadioOption.size() ; i++)
        {
            if(name == gradientModeRadioOption[i])
            {
                cout << i << "got event from: " << name << endl;
                setGradientMode(i);
            }
        }
        
        if(name == "OF_LOG_VERBOSE")ofSetLogLevel(OF_LOG_VERBOSE);
        else if(name == "OF_LOG_NOTICE")ofSetLogLevel(OF_LOG_NOTICE);
        else if(name == "OF_LOG_WARNING")ofSetLogLevel(OF_LOG_WARNING);
        else if(name == "OF_LOG_ERROR")ofSetLogLevel(OF_LOG_ERROR);
        else if(name == "OF_LOG_FATAL_ERROR")ofSetLogLevel(OF_LOG_FATAL_ERROR);
        else if(name == "OF_LOG_SILENT")ofSetLogLevel(OF_LOG_SILENT);
        
        
        if(name =="POINT")
        {
            mode = POINT;
            
        }if(name == "DISPLACEMENT")
        {
            mode = DISPLACEMENT;
            
        }else if(name == "TRIANGLE")
        {
            mode = TRIANGLE;
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
        case 'f':
            ofToggleFullscreen();
            break;
        case 'r':
            displacement.unload();
            displacement.load("simplicity");
            break;
        case 27:
            //            canon.endLiveView();
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
        case 'd':
            mode = DISPLACEMENT;
            break;
        case '\t':
            gui1->toggleVisible();
            if(gui1->isVisible())
            {
                ofShowCursor();
            }
            else{
                ofHideCursor();
            }
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
    if(ofGetLogLevel()==OF_LOG_VERBOSE)
        fireStrip(x,y,0,0);
    
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