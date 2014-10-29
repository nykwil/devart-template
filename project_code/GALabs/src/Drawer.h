#pragma once

#include "ofMain.h"


struct StrokePoint
{
	float size;
	ofVec2f pos;
	float weight;
};

class Drawer
{
public:
	static float UNITS_PER_POINT;
	static float CURVENESS;

	Drawer();
	ofColor rotatedPixel(ofImage& brush, ofVec2f pos, float size, float angle);
	void renderBrush(ofPixelsRef pixDest, const ofVec2f& pos, ofImage& imgBrush, float angle, float brushSize, const ofFloatColor& colMult);
	void stroke(ofPixelsRef pixDest, ofImage& imgBrush, const ofFloatColor& colMult, float brushSize, float startAngle, const ofVec2f& startPos, const ofVec2f& endPos);
	void stroke(ofPixelsRef pixDest, ofImage& imgBrush, const ofFloatColor& colMult, float brushSize, const ofPolyline& line);
};