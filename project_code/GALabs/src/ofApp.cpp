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

bool bUseCam = false;
bool bDrawBest = true;
bool bRun = true;

void ofApp::setup() {

	mDna = new CollageProblem();
	mDna->setup();

	gui.setup();
	gui.setDefaultKeys(true);
}

void ofApp::draw() {
	ofBackground(0);

	static ofImage imgBest;
	static ofImage imgWork;
	static ofImage imgLast;

	if (bRun) {
		mDna->go();
	}
	else if (bDrawBest && !mDna->isThreadRunning()) {
		static vector<float> workingValues;

		mDna->fillRandom(workingValues);
		mDna->fitnessTest(workingValues);
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
	if (key == 'c')
		bUseCam = !bUseCam;
	else if (key == 't') {
		bDrawBest = !bDrawBest;
	}
	else if (key == 'd') {
		bRun = !bRun; 
	}
	else if (key == 's') {
		mDna->startThread(true, false);
	}
}

void ofApp::exit()
{
	mDna->waitForThread(true);
}
