#include "ofApp.h"
#include "ofxSimpleGuiToo.h"
#include "ofxFatLine.h"

bool bRun = false;
bool bDebug = false;
bool bDrawAll = true;
ofImage imgOther;
ofImage imgWork;
ofImage imgFinal;

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

int width;
int height;

ofPixels pixResult;
ofImage baseImage;

bool terrible = false;;

void ofApp::setup() {

	gui.addToggle("Run", bRun);
	gui.addToggle("Debug", bDebug);
	gui.addToggle("DrawAll", bDrawAll);
	
	ofSetFrameRate(30);

	if (terrible) {
		return;
	}

	mDna = new CollageProblem();
	mDna->setup();
	width = mDna->mWorkingWidth;
	height = mDna->mWorkingHeight;

	ofSetWindowShape(width * 2, height * 2);

	gui.setup();
	gui.setDefaultKeys(true);

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

	//	fatline.draw();

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
			mDna->getCompImg(imgOther);
			if (imgOther.getWidth() > 0) {
				imgOther.draw(0, 0, width, height);
				ofDrawBitmapString("Other", 0, 10);
			}	

			mDna->getWorkImg(imgWork);
			imgWork.draw(width, 0, width, height);
			ofDrawBitmapString("Work", width, 10);

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
