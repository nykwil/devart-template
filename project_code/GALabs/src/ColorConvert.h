#pragma once

#include "ofMain.h"

class ColorHSV;
class ColorHSL;
class ColorxyY;
class ColorLinearRGB;
class ColorYUV;
class ColorYIQ;
class ColorXYZ;
class ColorLab;
class ColorLuv;
class ColorLChuv;
class ColorLChab;

class ColorCompare {
public:
	// CIE Delta E 1976
	// JND: ~2.3
	static float deltaE1976(const ColorLab& lab1, const ColorLab& lab2);

	// CIE Delta E 1994
	static float deltaE1994(const ColorLab& lab1, const ColorLab& lab2, int type);

	// CIE Delta E 2000
	// Note: maximum is about 158 for colors in the sRGB gamut.
	static float deltaE2000(const ColorLChab& lch1, const ColorLChab& lch2);

	static float HsbDiff(const ofColor& c1, const ofColor& c2);

	static float GetBright(const ofColor& col) {
		return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
	}

	static float BrightDiff(const ofColor& c1, const ofColor& c2);
};

class ColorRGB {
public:
	float R;
	float G;
	float B;
	bool clamped;

	ColorRGB(float R, float G, float B, bool clamped);
	ColorRGB(const ofFloatColor& color, bool clamped);

	ColorHSV toHSV();
	ColorHSL toHSL();

	float toLinearRGBc(float c) {
		if(c > 0.04045f) 
			return pow((c + 0.055f) / 1.055f, 2.4f);
		return c / 12.92f;
	}
	
	ColorLinearRGB toLinearRGB();
	ColorYUV toYUV();
	ColorYIQ toYIQ();
};

class ColorLinearRGB
{
public:
	float R;
	float G;
	float B;
	bool clamped;

	ColorLinearRGB(float R, float G, float B, bool clamped);
	
	float toRGBc(float c) {
		if(c > 0.0031308f) 
			return pow(c, 1.f / 2.4f) * 1.055f - 0.055f;
			
		return c * 12.92f;
	}

	ColorRGB toRGB();
	ColorXYZ toXYZ();
};

class ColorXYZ {
public:
	float X;
	float Y;
	float Z;
	bool clamped;

	ColorXYZ(float X, float Y, float Z, bool clamped);
	ColorLinearRGB toLinearRGB();

	ColorxyY toxyY();

	float toLabc(float c);

	ColorLab toLab();
	ColorLuv toLuv();
};

class ColorLChab
{
public:
    float L;
    float C;
    float h;
    bool clamped;

	ColorLChab(float L, float C, float h, bool clamped);
	ColorLab toLab();
};

class ColorLShuv
{
public:
    float L;
    float S;
    float h;
    bool clamped;

	ColorLShuv(float L, float S, float h, bool clamped);
	ColorLChuv toLChuv();
};

class ColorLChuv
{
public:
    float L;
    float C;
    float h;
    bool clamped;

	ColorLChuv(float L, float C, float h, bool clamped);
    ColorLShuv toLShuv();
	ColorLuv toLuv();
};

class ColorLuv
{
public:
    float L;
    float u;
    float v;
    bool clamped;

	ColorLuv(float L, float u, float v, bool clamped);
	ColorLChuv toLChuv();
	ColorXYZ toXYZ();
};	

class ColorLab
{
public:
    float L;
    float a;
    float b;
    bool clamped;

	ColorLab(float L, float a, float b, bool clamped);

	float toXYZc(float c);
	ColorXYZ toXYZ();
	ColorLChab toLChab();
};

class ColorxyY
{
public:
    float x;
    float y;
    float Y;
    bool clamped;

    ColorxyY(float x, float y, float Y, bool clamped);
	ColorXYZ toXYZ();
};

class ColorHSV
{
public:
    float H;
    float S;
    float V;
    bool clamped;

    ColorHSV(float H, float S, float V, bool clamped);
	ColorRGB toRGB();
};

class ColorHSL
{
public:
    float H;
    float S;
    float L;
    bool clamped;

    ColorHSL(float H, float S, float L, bool clamped);
	ColorRGB toRGB();
};

class ColorYUV
{
public:
    float Y;
    float U;
    float V;
    bool clamped;

    ColorYUV(float Y, float U, float V, bool clamped);
	ColorRGB toRGB();
};

class ColorYIQ
{
public:
    float Y;
    float I;
    float Q;
    bool clamped;

    ColorYIQ(float Y, float I, float Q, bool clamped);
	ColorRGB toRGB();
};
