#include "TestProblem.h"

#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"

float StrokeProblem::gDeltaSize = 5;
float StrokeProblem::gMinSize = 5;
float StrokeProblem::gMaxSize = 5;
float StrokeProblem::gRotation = 1;

enum RangeType {
	RT_X,
	RT_Y,
	RT_DEG,
	RT_SCALE,
	RT_IMAGE,
	RT_BLOB,
	RT_MAX
};

StrokeProblem::StrokeProblem() : GAProblem()
{
}

void StrokeProblem::setup()
{
	GAProblem::setup();
}

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

void StrokeProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& imgBase, int width, int height)
{
    assert(values.size() == mRanges.size() * mRepeat);
	pixResult.clear();
    pixResult.setFromPixels(imgBase.getPixels(), imgBase.getWidth(), imgBase.getHeight(), imgBase.getPixelsRef().getNumChannels());
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
//        float g = values[i++];
//        float b = values[i++];

        float sang = values[i++];
        float x = values[i++] * mWorkingWidth;
        float y = values[i++] * mWorkingHeight;

        float dx = values[i++] * gDeltaSize;
        float dy = values[i++] * gDeltaSize;

        float sz = ofLerp(gMinSize, gMaxSize, values[i++]);

        ofFloatColor col = ColorLook::instance().getPalette(r);

        mDraw.stroke(pixResult, mBrush[1], col, sz, sang * gRotation, ofVec2f(x, y), ofVec2f(x + (dx * sz), y + (dy * sz)));
    }
}

