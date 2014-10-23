#include "ofApp.h"
#include "ofxSimpleGuiToo.h"

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

bool bRun = false;

void ofApp::setup() {

	gui.addToggle("Run", bRun);
	ofEnableAlphaBlending();

	ofSetFrameRate(30);
	mDna = new CollageProblem();
	mDna->setup();

	gui.setup();
	gui.setDefaultKeys(true);

	ofSetWindowShape(mDna->width * 2, mDna->height * 2);

// 	ofFbo::Settings sett;
// 	sett.width = 100;
// 	sett.height = 100;
// 	fbo.allocate(sett);
// 	fbo.begin();
// 	ofClear(255,255,255, 0);
// 	fbo.end();
// // 	pixResult.allocate(100, 100, 4);
// // 	workingImage.allocate(100, 100, OF_IMAGE_COLOR_ALPHA);
// 
// 	fbo.begin();
// 	ofEnableAlphaBlending();
// 	ofClear(255, 255, 255, 0);
// 	ofSetColor(255, 0, 0, 255);
// 	ofRect(25, 25, 25, 125);
//  	ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
// 	ofSetColor(0, 0, 0, 100);
// 	ofRect(30, 30, 20, 20);
// 	ofSetColor(0, 0, 0, 155);
// 	ofRect(35, 35, 5, 5);
// 	fbo.end();
// 
// 	ofEnableAlphaBlending();
// 
// 	pixResult.clear();
// 	ofPixelsRef pixResultRef = pixResult;
// 	fbo.readToPixels(pixResultRef);
// 	workingImage.setFromPixels(pixResult);
// 	static int sadf = 0;
// 	ofSaveImage(workingImage.getPixels(), "file1.png", OF_IMAGE_QUALITY_BEST);
}

void ofApp::draw() {
	ofBackground(0);

	static ofImage imgBest;
	static ofImage imgWork;
	static ofImage imgLast;

	if (bRun && !mDna->isThreadRunning()) {
		mDna->go();
	}

	mDna->getBestImg(imgBest);
	ofSetColor(255);
	imgBest.draw(0,0);

	mDna->getWorkImg(imgWork);
	ofSetColor(255);
	imgWork.draw(mDna->width, 0);

	ofSetColor(255);
	mDna->mImgOrig.draw(0, mDna->height);

	mDna->getLastImg(imgLast);
	if (imgLast.getWidth() > 0){
		ofSetColor(255);
		imgLast.draw(mDna->width, mDna->height);
	}

	gui.draw();
}

void ofApp::keyPressed(int key) {
	if (key == 'd') {
		bRun = !bRun; 
	}
	else if (key == 's') {
		// mDna->startThread(true, false);
	}
}

void ofApp::exit()
{
	// mDna->waitForThread(true);
}
