#pragma once
#include "ofMain.h"
#include "ColorConvert.h"

class ColorLook
{
public:
	static unsigned int getIndex(unsigned char r, unsigned char g, unsigned char b)
	{
		return (r >> 3 << (5 * 2)) | (g >> 3 << 5) | b;
	}

	static ColorLook& instance();

	static ofColor getColour(unsigned int index)
	{
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

	ofColor getPalette(float f)
	{
		return getColour(palette[(int)((f - 0.000001f) * palette.size())]);
	}

	void buildColourLookup()
	{
		ofColor c(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
		unsigned int i = getIndex(c.r, c.g, c.b);
		ofColor co = getColour(i);

		unsigned int maxi = getIndex(255, 255, 255);
		colourLookup.resize(maxi + 1);
		for (unsigned int i1 = 0; i1 <= maxi; ++i1) {
			colourLookup[i1].resize(i1 + 1);
		}
	}

	float getDelta(const ofColor& col1, const ofColor& col2)
	{
		int i1 = getIndex(col1.r,col1.g,col1.b);
		int i2 = getIndex(col2.r,col2.g,col2.b);

		if (i1 == i2)
			return 255;
		else if (i2 > i1)
			std::swap(i1, i2);

		if (colourLookup[i1][i2] == 0)
		{
			ofColor c1 = getColour(i1);
			ofColor c2 = getColour(i2);
			float delta = ColorCompare::deltaE1976(
				ColorRGB(c1.r / 255.f, c1.g / 255.f, c1.b / 255.f, true).toLinearRGB().toXYZ().toLab(),
				ColorRGB(c2.r / 255.f, c2.g / 255.f, c2.b / 255.f, true).toLinearRGB().toXYZ().toLab()
				);

			colourLookup[i1][i2] = (ColorCompare::getMaxDelta() - delta) / ColorCompare::getMaxDelta() * 255.9f;
		}
		return colourLookup[i1][i2];
	}

	void buildPalette(ofImage& img) 
	{
		vector<bool> adds(getIndex(255, 255, 255) + 1);

		for (int x = 0; x < img.getWidth(); ++x) {
			for (int y = 0; y < img.getHeight(); ++y) {
				ofColor col = img.getColor(x, y);
				unsigned short i = getIndex(col.r, col.g, col.b);
				if (!adds[i])
					palette.push_back(i);

				adds[i] = true;
			}
		}
	}

	void sort();

	vector< unsigned short > palette;

private:
	vector< vector< unsigned char > > colourLookup;
};
