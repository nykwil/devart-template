#include "ColorLook.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"

//TODO:
// save values for reconstruction
// make a historgram
// sort the historgram into a list
// choose only colors from the historgram by index
// make a look up matrix with all the distances of colors.
// better sort

ColorLook mCL;

ColorLook& ColorLook::instance() {
	return mCL;
}

struct pred {
// 	bool operator()(const ofColor& c1, const ofColor& c2) const {
// 		static ofColor black = ofColor(255,255,255);
// 
// 		float delta1 = ColorLook::instance().getDelta(black, c1);
// 		float delta2 = ColorLook::instance().getDelta(black, c2);
// 		float f = delta1 < delta2;
// 		return f;
// 	}

// 	bool operator()(const ofColor& c1, const ofColor& c2) const {
// 		static ofColor black = ofColor(255,255,255);
// 
// 		float delta1 = ColorCompare::HsbDiff(black, c1);
// 		float delta2 = ColorCompare::HsbDiff(black, c2);
// 		float f = delta1 < delta2;
// 		return f;
// 	}

	bool operator()(const ofColor& c1, const ofColor& c2) const {
		static ofColor black = ofColor(255,255,255);

		float delta1 = ColorCompare::BrightDiff(black, c1);
		float delta2 = ColorCompare::BrightDiff(black, c2);
		float f = delta1 < delta2;
		return f;
	}

	//     bool operator()(const ofColor& c1, const ofColor& c2) const {
//         static ofColor black = ofColor(255,255,255);
// 
// 		float delta1 = ColorCompare::deltaE1976(
// 			ColorRGB(1.f, 1.f, 1.f, true).toLinearRGB().toXYZ().toLab(),
// 			ColorRGB(c1, true).toLinearRGB().toXYZ().toLab()
// 			);
// 
// 		assert (delta1 < ColorLook::instance().getMaxDelta());
// 
// 		float delta2 = ColorCompare::deltaE1976(
// 			ColorRGB(1.f, 1.f, 1.f, true).toLinearRGB().toXYZ().toLab(),
// 			ColorRGB(c2, true).toLinearRGB().toXYZ().toLab()
// 			);
// 
// 		assert (delta2 < ColorLook::instance().getMaxDelta());
// 
// 		float f = delta1 < delta2;
//         return f;
//     }
};

void ColorLook::buildPalette(ofImage& img) {
	vector<bool> adds(getIndex(255, 255, 255) + 1);

	for (int x = 0; x < img.getWidth(); ++x) {
		for (int y = 0; y < img.getHeight(); ++y) {
			ofColor col = img.getColor(x, y);
			unsigned int i = getIndex(col.r, col.g, col.b);
			if (!adds[i]) {
				palette.push_back(col);
			}

			adds[i] = true;
		}
	}
	std::sort(palette.begin(), palette.end(), pred());
}

float ColorLook::getDelta(const ofColor& col1, const ofColor& col2)
{
	int i1 = getIndex(col1.r, col1.g, col1.b);
	int i2 = getIndex(col2.r, col2.g, col2.b);

	return getDeltaFromIndex(i1, i2);
}

float ColorLook::getDeltaFromIndex(int i1, int i2)
{
	if (i1 == i2)
		return 255;
	else if (i2 > i1)
		std::swap(i1, i2);

	if (colorLookup[i1 * maxColor + i2] == 0) 
	{
		ofColor c1 = getColor(i1);
		ofColor c2 = getColor(i2);
		float delta = ColorCompare::deltaE1976(
			ColorRGB(c1, true).toLinearRGB().toXYZ().toLab(),
			ColorRGB(c2, true).toLinearRGB().toXYZ().toLab()
			);

		if (delta > maxDelta) {
			cout << "What" << endl;
		}

		float d = (maxDelta - delta) / maxDelta * 255.9999f;
		colorLookup[i1 * maxColor + i2] = (unsigned char)d;
		return d;
	}
 	else {
 		return (float)colorLookup[i1 * maxColor + i2];
 	}
}

ColorLook::ColorLook()
{
	maxColor = getIndex(255,255,255) + 1;
	colorLookup.resize(maxColor * maxColor);

	float delta = ColorCompare::deltaE1976(
		ColorRGB(ofColor(0), true).toLinearRGB().toXYZ().toLab(),
		ColorRGB(ofColor(255), true).toLinearRGB().toXYZ().toLab()
		);

	maxDelta = 200;
}
