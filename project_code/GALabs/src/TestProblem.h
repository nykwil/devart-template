#pragma once

#include "Drawer.h"
#include "GAProblem.h"

class FboProblem : public GAProblem
{
public:
    virtual void setup();
    virtual void setRanges();
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height);
    void drawValues(const vector<float>& values);
};

class BrushProblem : public GAProblem
{
public:
    virtual void setRanges();
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height);
	Drawer mDraw;
	ofImage mBrush[2];
};

class StrokeProblem : public GAProblem
{
public:
	StrokeProblem();
    static float gDeltaSize;
    static float gMinSize;
    static float gMaxSize;
    static float gRotation;
	virtual void setup();
    virtual void setRanges();
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height);
	Drawer mDraw;
	ofImage mBrush[2];
};

class BlobInfo {
public:
	ofImage texture;
	ofPolyline line;
	ofMesh mesh;
	ofPoint centroid;
};

class ImageCache {
public:
	ofImage image;
	string name;
	vector<BlobInfo*> blobs;

	void loadImage(const string& filename, float maxWidth);
	void deleteBlobs();
	void addBlobs(float threshold);

private:
	void createBlobCvGray(ofxCvGrayscaleImage& cvImg);
};

class CollageProblem : public GAProblem
{
public:
	CollageProblem();
	virtual void setup();
	virtual void setRanges();
	virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height);
	virtual void debugDraw();
	
	vector<ImageCache> mImages;
};

class StripProblem : public GAProblem
{
public:
	StripProblem();

	virtual void setRanges();
	virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height);
	virtual void debugDraw();
};
