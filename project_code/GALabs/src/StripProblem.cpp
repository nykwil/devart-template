#include "TestProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"

static float gMinLineWidth = 0.f;
static float gMaxLineWidth = 20.f;
static float gMinAlpha = 0.f;
static float gMaxAlpha = 120.f;

enum StripRangeType {
	RT_X,
	RT_Y,
	RT_ANG,
	RT_WIDTH,
	RT_COLOR,
	RT_ALPHA,
	RT_BLEND,
	RT_MAX
};

StripProblem::StripProblem() : GAProblem() {
	gui.addTitle("-- Strip --");
	gui.addSlider("MinLineWidth", gMinLineWidth, 0.0f, 5.0f);
	gui.addSlider("MaxLineWidth", gMaxLineWidth, 0.0f, 50.0f);
	gui.addSlider("MinAlpha", gMinAlpha, 0.0f, 255.0f);
	gui.addSlider("MaxAlpha", gMaxAlpha, 0.0f, 255.0f);
}

void StripProblem::setRanges() {
	mRanges.resize(RT_MAX);

	mRanges[RT_X] = RangeInfo(4, 0, 1.f); // x
	mRanges[RT_Y] = RangeInfo(4, 0, 1.f); // y
	mRanges[RT_ANG] = RangeInfo(4, 0.f, 360.0f); // y
	mRanges[RT_WIDTH] = RangeInfo(4, 0.f, 1.0f); // width
	mRanges[RT_COLOR] = RangeInfo(4, 0.f, 0.99999f); // color
	mRanges[RT_ALPHA] = RangeInfo(4, 0.f, 0.99999f); // alpha
	mRanges[RT_BLEND] = RangeInfo(4, 0.f, 0.99999f); // blend
}

void StripProblem::createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height) {
	assert(values.size() == mRanges.size() * mRepeat);
	assert(baseImage.getWidth() == width && baseImage.getHeight() == height);

	pixels.clear();

	ofFbo::Settings setts;
	setts.width = width;
	setts.height = height;
	setts.internalformat = GL_RGB;

	ofFbo mFbo;
	mFbo.allocate(setts);
	mFbo.begin();
	ofClear(255,255,255,255);

	ofSetColor(255);
	ofBlendMode(OF_BLENDMODE_DISABLED);
	baseImage.draw(0, 0);

	int i = 0;
	for (int is = 0; is < mRepeat; ++is) {
		float x = values[is * RT_MAX + RT_X] * width;
		float y = values[is * RT_MAX + RT_Y] * height;
		float ang = values[is * RT_MAX + RT_ANG];
		float w = myMap01(values[is * RT_MAX + RT_WIDTH], gMinLineWidth, gMaxLineWidth) * (width / 600);
		ofColor col = ColorLook::instance().getPalette(values[is * RT_MAX + RT_COLOR]);
		float a = myMap01(values[is * RT_MAX + RT_ALPHA], gMinAlpha, gMaxAlpha);
		int blend = (int)(values[is * RT_MAX + RT_BLEND] * 3);

		if (blend == 0) {
			ofBlendMode(OF_BLENDMODE_ALPHA);
		}
		else if (blend == 1) {
			ofBlendMode(OF_BLENDMODE_ADD);
		}
		else if (blend == 2) {
			ofBlendMode(OF_BLENDMODE_SUBTRACT);
		}
		else if (blend == 3) {
			ofBlendMode(OF_BLENDMODE_SCREEN);
		}

		col.a = a;
		ofSetColor(col);
		ofPushMatrix();
		ofTranslate(x, y);
		ofRotateZ(ang);
// 		if (ang > 180) {
// 			ofRotateZ(90);
// 		}
		ofRect(-(width + w), -w / 2, (width + w) * 2, w);
		// ofCircle(x,y,z);
		// ofBox(x,y,z,w);
		ofPopMatrix();
	}

    mFbo.end();
    mFbo.readToPixels(pixels);
}

void StripProblem::debugDraw()
{
	ofSetColor(255);
	ofBlendMode(OF_BLENDMODE_DISABLED);
	ofClear(0);

	float x = 0;
	float y = 0;

	int iy = (sqrt(ColorLook::instance().getSize()) + 1);
	float w = ofGetHeight() / (float)ColorLook::instance().getSize() * iy;
	for (int i = 0; i < ColorLook::instance().getSize(); ++i) {
		ofColor c = ColorLook::instance().getPalette((float)i / ColorLook::instance().getSize());
		ofSetColor(c);
		ofRect(x, y, w, w);
		if (x > ofGetWidth()) {
			x = 0;
			y += w;
		}
		else {
			x += w;
		}
	}
}
