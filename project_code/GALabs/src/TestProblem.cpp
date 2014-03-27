#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"

void FboProblem::setup()
{
    GAProblem::setup();

    mFbo.allocate(width, height);
    mFbo.begin();
    ofClear(255,255,255, 0);
    mFbo.end();
}

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

void FboProblem::drawValues( const vector<float>& values )
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
        ofRect(x * width, y * width, w, h);
        //        ofCircle(x,y,z);
        // ofBox(x,y,z,w);
    }
}

void FboProblem::createPixels( ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage )
{
    assert (values.size() == mRanges.size() * mRepeat);
    assert(mFbo.isAllocated());
    mFbo.begin();
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

void BrushProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& imgBase)
{
    assert (values.size() == mRanges.size() * mRepeat);
    pixResult.setFromPixels(imgBase.getPixels(), imgBase.getWidth(), imgBase.getHeight(), imgBase.getPixelsRef().getNumChannels());
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
        float g = values[i++];
        float b = values[i++];
        float x = values[i++] * width;
        float y = values[i++] * height;
        float sz = values[i++];
        float ang = values[i++];
        mDraw.renderBrush(pixResult, ofVec2f(x, y), mBrush[1], sz, ang, ofFloatColor(r, g, b));
    }
}

//////////////////////////////////////////////////////////////////////////

void StrokeProblem::setRanges()
{
    mRanges.clear();
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    //    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    //    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b

    mRanges.push_back(RangeInfo(4, -PI, PI)); // ang
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // x
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // y

    mRanges.push_back(RangeInfo(4, -1.f, 1.f)); // dx
    mRanges.push_back(RangeInfo(4, -1.f, 1.f)); // dy
    mRanges.push_back(RangeInfo(4, 0, 1.f)); // size
}

float StrokeProblem::gDeltaSize = 5;
float StrokeProblem::gMinSize = 5;
float StrokeProblem::gMaxSize = 5;
float StrokeProblem::gRotation = 1;

void StrokeProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& imgBase)
{
    assert (values.size() == mRanges.size() * mRepeat);
    pixResult.setFromPixels(imgBase.getPixels(), imgBase.getWidth(), imgBase.getHeight(), imgBase.getPixelsRef().getNumChannels());
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
//        float g = values[i++];
//        float b = values[i++];

        float sang = values[i++];
        float x = values[i++] * width;
        float y = values[i++] * height;

        float dx = values[i++] * gDeltaSize;
        float dy = values[i++] * gDeltaSize;

        float sz = ofLerp(gMinSize, gMaxSize, values[i++]);

        ofFloatColor col = ColorLook::instance().getPalette(r);

        mDraw.stroke(pixResult, mBrush[1], col, sz, sang * gRotation, ofVec2f(x, y), ofVec2f(x + (dx * sz), y + (dy * sz)));
    }
}

void StrokeProblem::setup()
{
	GAProblem::setup();
//	mBrush[0].loadImage(ofToDataPath("brushtest.png"));
//	mBrush[1].loadImage(ofToDataPath("softbrush2.png"));
	//    mBrush[1].loadImage(ofToDataPath("circlebrush.png"));
}
