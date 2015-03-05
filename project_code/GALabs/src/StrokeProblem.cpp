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

const int NUM_POINTS = 5;

static LineStrip strip;
static int mWeiNum = 10;
static float mWeiAdd = 1.f;
static float mWeiNoise = 0.1f;

StrokeProblem::StrokeProblem() : GAProblem() {
	gui.addPage("Stroke");
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
	gui.addSlider("MinSize", gMinSize, 0.f, 100.f);
	gui.addSlider("MaxSize", gMaxSize, 0.f, 100.f);
	gui.addSlider("WeiAdd", mWeiAdd, 0.f, 2.f);
	gui.addSlider("WeiNoise ", mWeiNoise , 0.001f, 2.f);
}

void StrokeProblem::setup() {
	GAProblem::setup();
}

void StrokeProblem::setRanges() {
	mRanges.clear();
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // COL
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // SIZE
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // SEED
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // BLEND
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // ALPHA

	for (int in = 0; in < NUM_POINTS; ++in) {
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

	float widthMin = -width * 0.25f;
	float widthMax = width - widthMin;
	float heightMin = -height * 0.25f;
	float heightMax = height - heightMin;

	int i = 0;
	for (int is = 0; is < mRepeat; ++is) {
		strip.clear();
		strip.mFillColor = ColorLook::instance().getPalette(values[i++]); // COL
		float sz = ofLerp(gMinSize, gMaxSize, values[i++]); // SIZE
		int seed = (int)(values[i++] * 10000.f); // SEED
		int blend = (int)(values[i++] * 3); // BLEND
		strip.mFillColor.a = (int)(values[i++] * 255); // ALPHA

		for (int in = 0; in < NUM_POINTS; ++in) {
			float x = myMap01(values[i++], widthMin, widthMax);
			float y = myMap01(values[i++], widthMin, widthMax);
			strip.addVertex(ofVec3f(x, y, 0));
		}
		
		if (blend == 0) {
			ofBlendMode(OF_BLENDMODE_ADD);
		}
		else if (blend == 1) {
			ofBlendMode(OF_BLENDMODE_SUBTRACT);
		}
		else if (blend == 2) {
			ofBlendMode(OF_BLENDMODE_ALPHA);
		}
		else if (blend == 3) {
			ofBlendMode(OF_BLENDMODE_SCREEN);
		}

		strip.mWeight.clear();
		ofSeedRandom(seed);
		for (int i = 0; i < mWeiNum; ++i) {
			strip.mWeight.push_back((mWeiAdd + ofRandom(1.f)) * sz * (width / 600.f));
		}
		strip.draw();
	}

	fbo.end();
	fbo.readToPixels(pixels);
}

