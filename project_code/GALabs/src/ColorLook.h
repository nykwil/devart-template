#pragma once
#include "ofMain.h"
#include "ColorConvert.h"

class ColorLook
{
public:
	static unsigned int getIndex(unsigned char r, unsigned char g, unsigned char b) {
		return (r >> 3 << (5 * 2)) | (g >> 3 << 5) | b;
	}

	static ColorLook& instance();

	ColorLook();

	static ofColor getColor(unsigned int index) {
		int r = index >> (5 * 2) << 3;
		index -= r << (5 * 2);
		int g = index >> 5 << 3;
		index -= g << 5;
		int b = index << 3;
		return ofColor(r, g, b);
	}

	int getSize() {
		return palette.size();
	}

	ofColor getPalette(float f) {
		return palette[(int)((f - 0.000001f) * palette.size())];
	}

	float getDelta(const ofColor& col1, const ofColor& col2);

	float getDeltaFromIndex(int i1, int i2);

	void buildPalette(ofImage& img);

	float getMaxDelta() { return maxDelta; };

private:
	vector<unsigned char> colorLookup;
	int maxColor;
	vector<ofColor> palette;
	float maxDelta;
};
