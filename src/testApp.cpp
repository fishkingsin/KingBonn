#include "testApp.h"
//--------------------------------------------------------------
void testApp::exit() {
//	kinect.setCameraTiltAngle(0); // zero the tilt on exit
//	kinect.close();
//	bonjourServer.startService( "_ecs._tcp.", "ecs", 7170 );
}
//--------------------------------------------------------------
void testApp::setup(){
<<<<<<< HEAD
//    kinect.setRegistration(true);
//    
//	kinect.init();
//	//kinect.init(true); // shows infrared instead of RGB video image
//	//kinect.init(false, false); // disable video image (faster fps)
//	kinect.open();
//    angle = 0;
//	kinect.setCameraTiltAngle(angle);
=======
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
	light.setPosition(1000, 1000, 2000);
>>>>>>> added shader, added camera orbit , added porintCloud
}

//--------------------------------------------------------------
void testApp::update(){
<<<<<<< HEAD
//    kinect.update();
	
	// there is a new frame and we are connected
=======
    kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew())
    {
        
    }
    // check for waiting messages
	while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);
        
		// check for mouse moved message
		if(m.getAddress() == "/orbit"){
            orbit.x = m.getArgAsInt32(0);
            orbit.y = m.getArgAsInt32(1);
        }
    }
//    cam.orbit(orbit.x, orbit.y, 1000);
>>>>>>> added shader, added camera orbit , added porintCloud
    
}

//--------------------------------------------------------------
void testApp::draw(){
<<<<<<< HEAD
    drawPointCloud();
}
//--------------------------------------------------------------
void testApp::drawPointCloud() {
//    int w = 640;
//    int h = 480;
//    ofMesh mesh;
//    mesh.setMode(OF_PRIMITIVE_POINTS);
//    int step = 2;
//    for(int y = 0; y < h; y += step) {
//        for(int x = 0; x < w; x += step) {
//            if(kinect.getDistanceAt(x, y) > 0) {
//                mesh.addColor(kinect.getColorAt(x,y));
//                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
//            }
//        }
//    }
//    glPointSize(3);
//    ofPushMatrix();
//    // the projected points are 'upside down' and 'backwards'
//    ofScale(1, -1, -1);
//    ofTranslate(0, 0, -1000); // center the points a bit
//    glEnable(GL_DEPTH_TEST);
//    mesh.drawVertices();
//    glDisable(GL_DEPTH_TEST);
//    ofPopMatrix();
=======
    ofBackground(0);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    // copy enable part of gl state
    glPushAttrib(GL_ENABLE_BIT);
    
    // setup gl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    light.enable();
    
    // begin scene to post process
    post.begin(cam);
    
    drawPointCloud();
    // end scene and draw
    post.end();
    
    // set gl state back to original
    glPopAttrib();
}
void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
//    triangulation.reset();
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS );
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) > 0 && kinect.getDistanceAt(x, y) < 2000)
            {
//                triangulation.addPoint(kinect.getWorldCoordinateAt(x, y));

				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
    
//    for(int y = 0; y < h; y += step) {
//		for(int x = 0; x < w; x += step) {
//			mesh.addIndex(x+y*w);				// 0
//			mesh.addIndex((x+1)+y*w);			// 1
//			mesh.addIndex(x+(y+1)*w);			// 10
//			
//			mesh.addIndex((x+1)+y*w);			// 1
//			mesh.addIndex((x+1)+(y+1)*w);		// 11
//			mesh.addIndex(x+(y+1)*w);			// 10
//		}
//	}
    
//    triangulation.triangulate();
	glPointSize(5);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1000); // center the points a bit
	glEnable(GL_DEPTH_TEST);
//    triangulation.draw();
	mesh.drawVertices();
    mesh.drawWireframe();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
>>>>>>> added shader, added camera orbit , added porintCloud
}
//--------------------------------------------------------------
<<<<<<< HEAD
void testApp::keyPressed(int key){
    
=======
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	

>>>>>>> added shader, added camera orbit , added porintCloud
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
    unsigned idx = key - '0';
    if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());
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