#include "TestProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"

void FboProblem::setRanges()
{
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // x
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // y
    mRanges.push_back(RangeInfo(4, 20, 50)); // w
    mRanges.push_back(RangeInfo(4, 20, 50)); // h
}

void FboProblem::drawValues(const vector<float>& values)
{
    ofFill();
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
        float g = values[i++];
        float b = values[i++];

        float x = values[i++];
        float y = values[i++];

        float w = values[i++];
        float h = values[i++];

        ofFloatColor col(r,g,b);
        ofSetColor(col);
        ofRect(x * mWorkingWidth, y * mWorkingHeight, w, h);
        //        ofCircle(x,y,z);
        // ofBox(x,y,z,w);
    }
}

void FboProblem::createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height)
{
    assert(values.size() == mRanges.size() * mRepeat);
	pixels.clear();

	ofFbo mFbo;
	mFbo.allocate(width, height);
	mFbo.begin();
	ofClear(255,255,255, 0);
	
    ofFill();
    ofSetColor(255);
    baseImage.draw(0,0);
    drawValues(values);
    mFbo.end();
    mFbo.readToPixels(pixels);
}

//////////////////////////////////////////////////////////////////////////

void BrushProblem::setRanges()
{
    mRanges.clear();
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // x
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // y
    mRanges.push_back(RangeInfo(4, 20, 50)); // size
    mRanges.push_back(RangeInfo(4, 0, TWO_PI)); // ang
}

void BrushProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& imgBase, int width, int height)
{
    assert (values.size() == mRanges.size() * mRepeat);
	pixResult.clear();
    pixResult.setFromPixels(imgBase.getPixels(), imgBase.getWidth(), imgBase.getHeight(), imgBase.getPixelsRef().getNumChannels());
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
        float g = values[i++];
        float b = values[i++];
        float x = values[i++] * mWorkingWidth;
        float y = values[i++] * mWorkingHeight;
        float sz = values[i++];
        float ang = values[i++];
        mDraw.renderBrush(pixResult, ofVec2f(x, y), mBrush[1], sz, ang, ofFloatColor(r, g, b));
    }
}
