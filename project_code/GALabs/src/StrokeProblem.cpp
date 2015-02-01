#include "TestProblem.h"

#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"
#include "LineStrip.h"

float StrokeProblem::gDeltaSize = 5;
float StrokeProblem::gMinSize = 5;
float StrokeProblem::gMaxSize = 5;
float StrokeProblem::gRotation = 1;

static LineStrip strip;
static int mWeiNum = 10;
static float mWeiScale = 10.f;
static float mWeiAdd = 1.f;
static float mWeiNoise = 0.1f;

StrokeProblem::StrokeProblem() : GAProblem() {
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
}

void StrokeProblem::setup() {
	GAProblem::setup();
}

void StrokeProblem::setRanges() {
	mRanges.clear();
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // col
	mRanges.push_back(RangeInfo(4, 0, 1.f)); // size

	for (int in = 0; in < 5; ++in) {
		mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // x
		mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // y
	}
}

void StrokeProblem::createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height) {
	assert(values.size() == mRanges.size() * mRepeat);
	assert(baseImage.getWidth() == width && baseImage.getHeight() == height);

	pixels.clear();

	ofFbo::Settings setts;
	setts.width = width;
	setts.height = height;
	setts.internalformat = GL_RGB;

	ofFbo fbo;
	fbo.allocate(setts);
	fbo.begin();
	ofClear(255,255,255,255);

	ofSetColor(255);
	ofBlendMode(OF_BLENDMODE_DISABLED);
	baseImage.draw(0, 0);

	int i = 0;
	for (int is = 0; is < mRepeat; ++is) {
		strip.clear();
		ofFloatColor col = ColorLook::instance().getPalette(values[i++]);
		float sz = ofLerp(gMinSize, gMaxSize, values[i++]) * mWorkingWidth;
		for (int in = 0; in < 5; ++in) {
			strip.addVertex(ofVec3f(values[i++] * mWorkingWidth, values[i++] * mWorkingHeight, 0));		
		}
		
		strip.mWeight.clear();
		float f = ofRandom(1.f);
		for (int i = 0; i < mWeiNum; ++i) {
			strip.mWeight.push_back((mWeiAdd + ofRandom(1.f)) * mWeiScale);
		}
		ofSetColor(col);
		strip.draw();
	}

	fbo.end();
	fbo.readToPixels(pixels);
}

