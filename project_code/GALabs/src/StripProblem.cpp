#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"

void StripProblem::setRanges() {
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b
	mRanges.push_back(RangeInfo(4, 0.f, 1.f));   // a
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // x
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // y
    mRanges.push_back(RangeInfo(4, 20, 50)); // w
    mRanges.push_back(RangeInfo(4, 0, 360)); // ang
}

void StripProblem::createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height) {
	assert(values.size() == mRanges.size() * mRepeat);

	pixels.clear();

	ofFbo mFbo;
	mFbo.allocate(width, height);
	mFbo.begin();
	ofClear(255,255,255, 0);

	ofSetColor(255);
	ofFill();
	baseImage.draw(0,0);

	int i = 0;
	for (int is = 0; is < mRepeat; ++is)
	{
		float r = values[i++];
		float g = values[i++];
		float b = values[i++];
		float a = values[i++];

		float x = values[i++] * mWorkingWidth;
		float y = values[i++] * mWorkingHeight;

		float w = values[i++];
		float ang = values[i++];

		ofFloatColor col(r,g,b,a);
		ofSetColor(col);
		ofTranslate(x, y);
		ofRotateZ(ang);
		ofRect(-w / 2, mWorkingHeight, w, mWorkingHeight * 2.f);
		ofPopMatrix();
		//        ofCircle(x,y,z);
		// ofBox(x,y,z,w);
	}

    mFbo.end();
    mFbo.readToPixels(pixels);
}

