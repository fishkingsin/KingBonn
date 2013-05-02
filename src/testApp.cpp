#include "testApp.h"
//--------------------------------------------------------------
void testApp::exit() {
//	kinect.setCameraTiltAngle(0); // zero the tilt on exit
//	kinect.close();
//	bonjourServer.startService( "_ecs._tcp.", "ecs", 7170 );
}
//--------------------------------------------------------------
void testApp::setup(){
//    kinect.setRegistration(true);
//    
//	kinect.init();
//	//kinect.init(true); // shows infrared instead of RGB video image
//	//kinect.init(false, false); // disable video image (faster fps)
//	kinect.open();
//    angle = 0;
//	kinect.setCameraTiltAngle(angle);
}

//--------------------------------------------------------------
void testApp::update(){
//    kinect.update();
	
	// there is a new frame and we are connected
    
}

//--------------------------------------------------------------
void testApp::draw(){
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
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
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