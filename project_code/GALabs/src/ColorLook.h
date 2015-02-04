#pragma once
#include "ofMain.h"
#include "ColorConvert.h"

class ColorLook
{
public:
	static const int maxval = 40;

	static unsigned int getIndex(unsigned char r, unsigned char g, unsigned char b) {
		int nr = static_cast<unsigned int>(static_cast<float>(r) / 255.f * static_cast<float>(maxval - 0.001f));
		int ng = static_cast<unsigned int>(static_cast<float>(g) / 255.f * static_cast<float>(maxval - 0.001f));
		int nb = static_cast<unsigned int>(static_cast<float>(b) / 255.f * static_cast<float>(maxval - 0.001f));
		int total = (nr * maxval * maxval) + (ng * maxval) + nb;
		return total;
	}

	static ofColor getColor(unsigned int index) {
		int nr = index / (maxval * maxval);
		index -= nr * maxval * maxval;
		int ng = index / maxval;
		index -= ng * maxval;
		int nb = index;
		return ofColor(static_cast<float>(nr) / maxval * 255.f, static_cast<float>(ng) / maxval * 255.f, static_cast<float>(nb) / maxval * 255.f);
	}

	static ColorLook& instance();

	ColorLook();

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
