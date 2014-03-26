#include "ColorLook.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"

//TODO:
// save values for reconstruction
// make a historgram
// sort the historgram into a list
// choose only colors from the historgram by index
// make a look up matrix with all the distances of colours.
// better sort

ColorLook mCL;

ColorLook& ColorLook::instance()
{
	return mCL;
}

struct pred
{
    bool operator()(unsigned short const & a, unsigned short const & b) const
    {
        static int iblack = ColorLook::getIndex(255,255,255);
        return mCL.getDelta(a, iblack) < mCL.getDelta(b, iblack);
    }
};


void ColorLook::sort()
{
	buildColourLookup();        
	std::sort(palette.begin(), palette.end(), pred());
}
