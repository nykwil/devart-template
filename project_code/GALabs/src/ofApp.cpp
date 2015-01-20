#include "ofApp.h"
#include "ofxSimpleGuiToo.h"
#include "TestProblem.h"
#include "rbMath.h"
//#include "ofxFatLine.h"
#include "ofEasyCam.h"
#include "LineStrip.h"
ofEasyCam mCam;

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

bool terrible = true;



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

LineStrip strip;
int mWeiNum = 10;
float mWeiScale = 10.f;
float mWeiAdd = 1.f;
float mWeiNoise = 0.1f;

void ofApp::setup() {

	gui.addToggle("Run", bRun);
	gui.addToggle("Debug", bDebug);
	gui.addToggle("DrawAll", bDrawAll);

	gui.addToggle("DrawMesh", strip.bDrawMesh);
	gui.addToggle("DrawWire", strip.bDrawWire);
	gui.addToggle("DrawLine", strip.bDrawLine);
	gui.addToggle("DrawNormal", strip.bDrawNormal);
	gui.addToggle("DrawTangeant", strip.bDrawTangeant);
	gui.addToggle("DrawPoints", strip.bDrawPoints);
	gui.addToggle("DrawOutline", strip.bDrawOutline);
	gui.addToggle("DrawLinePoints", strip.bDrawLinePoints);
	gui.addToggle("DrawOrigLine", strip.bDrawOrigLine);
	gui.addSlider("DefaultWidth", strip.mDefaultWidth, 1.f, 100.f);
	gui.addSlider("SmoothingSize", strip.mSmoothingSize, 0.f, 1.f);
	gui.addSlider("Spacing", strip.mSpacing, 0, 100.f);
	gui.addSlider("SmoothingShape", strip.mSmoothingShape, 0.f, 1.f);
	gui.addSlider("OutSmoothingSize", strip.mOutSmoothingSize, 0.f, 1.f);
	gui.addSlider("OutSpacing", strip.mOutSpacing, 0, 100.f);
	gui.addSlider("AngStep", strip.mAngStep, 0.01f, TWO_PI);
	gui.addSlider("WeiNum", mWeiNum, 3, 30);
	gui.addSlider("WeiScale", mWeiScale, 0.f, 100.f);
	gui.addSlider("WeiAdd", mWeiAdd, 0.f, 2.f);
	gui.addSlider("WeiNoise ", mWeiNoise , 0.001f, 2.f);

	ofSetFrameRate(30);

	if (terrible) {
		strip.addVertex(ofVec3f(100, 500, 0));
		strip.addVertex(ofVec3f(300, 100, 0));
		strip.addVertex(ofVec3f(600, 400, 0));
		strip.addVertex(ofVec3f(800, 150, 0));
	}
	else {
		//	mDna = new CollageProblem();
		mDna = new StripProblem();
		mDna->setup();
		width = mDna->mWorkingWidth;
		height = mDna->mWorkingHeight;

		ofSetWindowShape(width * 2, height * 2);
	}

	gui.setup();
	gui.loadFromXML();
	gui.setDefaultKeys(true);
}

void ofApp::draw() {
	ofBackground(0);

	if (false) {
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
	}
	else if (terrible) {
		//mCam.begin();
		//ofEnableDepthTest();
		strip.draw();
		//mCam.end();
	}
	else {
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
	else if (key == 'p') {
		strip.clear();
		for (int i = 0; i < 5; ++i) {
			strip.addVertex(ofVec3f(ofRandom(50, 800), ofRandom(50, 800), 0));
		}
	}

	else if (key == 'o') {
		strip.mWeight.clear();
		float f = ofRandom(1.f);
		for (int i = 0; i < mWeiNum; ++i) {
			strip.mWeight.push_back((mWeiAdd + ofRandom(1.f)) * mWeiScale);
		}
	}
}

void ofApp::exit()
{
	// mDna->waitForThread(true);
}
