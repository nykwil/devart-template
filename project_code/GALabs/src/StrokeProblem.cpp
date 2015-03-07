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

const int NUM_POINTS = 3;

static LineStrip strip;

static float mWeiStart = 10;
static float mWeiDist = 1.f;
static float mWeiEnd = 0.1f;

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
	gui.addSlider("MinSize", gMinSize, 0.f, 100.f);
	gui.addSlider("MaxSize", gMaxSize, 0.f, 100.f);
	gui.addSlider("WeiStart", mWeiStart, 0.f, 20.f);
	gui.addSlider("WeiDist", mWeiDist, 0.f, 20.f);
	gui.addSlider("WeiEnd ", mWeiEnd , 0.0f, 20.f);
}

void StrokeProblem::setup() {
	GAProblem::setup();
}

void StrokeProblem::setRanges() {
	mRanges.clear();
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // COL
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // MINSIZE
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // MAXSIZE
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // SEED
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // BLEND
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // ALPHA

	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // x
	mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // y

	for (int in = 0; in < NUM_POINTS; ++in) {
		mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // DIR
		mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // DIST
	}
}

// weight is a function of size and length

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

	float minDist =  width * 0.001f;
	float maxDist = width * 0.25f;

	int i = 0;
	for (int is = 0; is < mRepeat; ++is) {
		strip.clear();
		strip.mFillColor = ColorLook::instance().getPalette(values[i++]); // COL
		float minSize = ofLerp(gMinSize, gMaxSize, values[i++]); // MINSIZE
		float maxSize = ofLerp(gMinSize, gMaxSize, values[i++]); // MAXSIZE
		int seed = (int)(values[i++] * 10000.f); // SEED
		int blend = (int)(values[i++] * 3); // BLEND
		strip.mFillColor.a = (int)(values[i++] * 255); // ALPHA
		strip.mWeight.clear();

		float x = values[i++];
		float y = values[i++];

		ofVec3f v = ofVec3f(ofLerp(widthMin, widthMax, x), 
			ofLerp(heightMin, heightMax, y), 
			0.0f);


		strip.mWeight.push_back(ofLerp(minSize, maxSize, ofRandom(1.f)) * (width / 600.f) * mWeiStart);
		strip.addVertex(v);
		
		ofSeedRandom(seed);
		for (int in = 0; in < NUM_POINTS; ++in) {
			float dir = ofLerp(-PI, PI, values[i++]); // DIR
			float dist = ofLerp(minDist, maxDist, values[i++]); // DIST
			v += ofVec3f(sin(dir), cos(dir), 0) * dist;

			strip.mWeight.push_back(ofLerp(minSize, maxSize, ofRandom(1.f)) * 
				(width / 600.f) * 
				(mWeiDist / dist) *
				(mWeiEnd * (1.0f - (float)in / (float)NUM_POINTS))
				);
			
			strip.addVertex(v);
		}

		if (blend == 0) {
			ofEnableBlendMode(OF_BLENDMODE_ADD);
		}
		else if (blend == 1) {
			ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
		}
		else if (blend == 2) {
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		}
		else if (blend == 3) {
			ofEnableBlendMode(OF_BLENDMODE_SCREEN);
		}
		strip.draw();
		ofDisableAlphaBlending();
	}

	fbo.end();
	fbo.readToPixels(pixels);
}

