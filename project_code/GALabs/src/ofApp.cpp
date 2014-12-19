#include "ofApp.h"
#include "ofxSimpleGuiToo.h"
//#include "ofxFatLine.h"

// @TODO clear button
// @TODO clean up variables in collage set reasonable defaults

bool bRun = false;
bool bDebug = false;
bool bDrawAll = true;
ofImage imgComp;
ofImage imgWork;
ofImage imgFinal;
int width;
int height;

ofPixels pixResult;
ofImage baseImage;

bool terrible = false;;

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

void ofApp::setup() {

	gui.addToggle("Run", bRun);
	gui.addToggle("Debug", bDebug);
	gui.addToggle("DrawAll", bDrawAll);
	
	ofSetFrameRate(30);

	if (terrible) {
		return;
	}

//	mDna = new CollageProblem();
	mDna = new StripProblem();
	mDna->setup();
	width = mDna->mWorkingWidth;
	height = mDna->mWorkingHeight;

	ofSetWindowShape(width * 2, height * 2);

	gui.setup();
	gui.setDefaultKeys(true);
}

void ofApp::draw() {
	ofBackground(0);

	if (terrible) {
		ofFbo mFbo;

		ofFbo::Settings sett;
		sett.width = width;
		sett.height = height;
		mFbo.allocate(sett);
		mFbo.begin();

		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255, 0, 255, 255);
		ofRect(10, 10, 800, 800);

		ofSetColor(0,250, 255, 100);
		ofRect(10, 10, 100, 100);

 		ofSetColor(0, 250, 255, 1);
 		ofRect(110, 110, 100, 100);

		// end draw
		mFbo.end();
		mFbo.readToPixels(pixResult);

		baseImage.setFromPixels(pixResult);
		baseImage.draw(0,0);
		
		return;
	}

	if (bRun && !mDna->isThreadRunning()) {
		mDna->go();

		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		ofSetColor(255);

		if (bDrawAll) {
			mDna->getCompImg(imgComp);
			if (imgComp.getWidth() > 0) {
				imgComp.draw(0, 0, width, height);
				ofDrawBitmapString("Comp", 0, 10);
			}	

			mDna->getWorkImg(imgWork);
			if (imgWork.getWidth() > 0) {
				imgWork.draw(width, 0, width, height);
				ofDrawBitmapString("Work", width, 10);
			}	

			mDna->mImgOrig.draw(0, height, width, height + 10);
			ofDrawBitmapString("Orig", 0, height);

			mDna->getFinalImg(imgFinal);
			if (imgFinal.getWidth() > 0) {
				imgFinal.draw(width, height, width, height);
				ofDrawBitmapString("Final", width, height + 10);
			}	
		}
		else {
			mDna->getFinalImg(imgFinal);
			if (imgFinal.getWidth() > 0) {
				imgFinal.draw(0, 0, mDna->mFinalWidth, mDna->mFinalHeight);
			}	
		}

	}
	else if (bDebug) {
		ofBackground(50);
		mDna->debugDraw();
	}

	gui.draw();
}

void ofApp::keyPressed(int key) {
	if (key == 'u') {
		bRun = !bRun; 
	}
	else if (key == 'd') {
		bDebug = !bDebug; 
	}
	else if (key == 's') {
		ofSaveImage(pixResult, "output.png", OF_IMAGE_QUALITY_BEST);
		// mDna->startThread(true, false);
	}
}

void ofApp::exit()
{
	// mDna->waitForThread(true);
}
