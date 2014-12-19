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

ColorLook& ColorLook::instance() {
	return mCL;
}

struct pred {
    bool operator()(unsigned short const & a, unsigned short const & b) const {
        static int iblack = ColorLook::getIndex(255,255,255);
        return mCL.getDelta(a, iblack) < mCL.getDelta(b, iblack);
    }
};

void ColorLook::sort() {
	std::sort(palette.begin(), palette.end(), pred());
}

void ColorLook::buildPalette(ofImage& img) {
	vector<bool> adds(getIndex(255, 255, 255) + 1);

	for (int x = 0; x < img.getWidth(); ++x) {
		for (int y = 0; y < img.getHeight(); ++y) {
			ofColor col = img.getColor(x, y);
			unsigned short i = getIndex(col.r, col.g, col.b);
			if (!adds[i]) {
				palette.push_back(i);
			}

			adds[i] = true;
		}
	}
	sort();
}
