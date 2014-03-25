#pragma once

#include "ofMain.h"
#include "ofxGALib.h"
#include "ofxOpenCv.h"

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

class GAProblem : public ofThread 
{
public:
    GAProblem();
	void pickBest(ofImage& mImg1, ofImage& mImg2);
	float fitnessTest(const vector<float>& values);
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage) = 0;
    void fillRandom(vector<float>& values);

	float compareImg(ofImage& img1, ofImage& img2, int method);
	virtual void setup();
	virtual void setRanges() = 0;
	virtual void threadedFunction();
	void go();
	void pushValues( const vector<float>& workingValues, const ofPixelsRef workingPixels );

    list<ofImage> mLayers;
    list< vector<float> > mLayerValues;
    ofPixels mWorkingPixels;
    ofImage mImgCompare;
    ofImage mImgOrig;

    ofxGALib mGALib;
	float mBestValue;
	int mPopSize;
	int mNGen;

    void getBestImg( ofImage& img );
    void getWorkImg( ofImage& img );
    void getLastImg( ofImage& img );

    ofMutex mutexWork;
    ofMutex mutexBest;
    ofMutex mutexLast;

    ofImage _mImgWork;
    ofImage _mImgBest;
    ofImage _mImgLast;

    int mCompMethod;
    bool mUseDna;
    bool bFlattenAndSave;
    int mCompareWidth;
    int mCompareHeight;
    int mRepeat;

    vector<RangeInfo> mRanges;
};

class FboProblem : public GAProblem
{
public:
    virtual void setup();
    virtual void setRanges();
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);
    void drawValues(const vector<float>& values);
    ofFbo mFbo;
};

class BrushProblem : public GAProblem
{
public:
    virtual void setRanges();
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);
	Drawer mDraw;
	ofImage mBrush[2];
};

class StrokeProblem : public GAProblem
{
public:
    static float gDeltaSize;
    static float gMinSize;
    static float gMaxSize;
    static float gRotation;
	virtual void setup();
    virtual void setRanges();
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);
	Drawer mDraw;
	ofImage mBrush[2];
};

class BlobInfo {
public:
	ofPolyline line;
	ofMesh mesh;
	ofPoint centroid;
};

class ImageCache {
public:
	ofImage image;
	ofxCvColorImage cvImgColor;
	ofxCvGrayscaleImage cvImgGrayscale;
	vector<BlobInfo*> blobs;
	ofxCvContourFinder contourFinder;
	ofTessellator tess;

	void loadImage(string filename, float width, float height);

	void createBlobCvGray(ofxCvGrayscaleImage& cvImg);
	void createBlobImage(ImageCache& img);
	void createBlobs(float threshold);
	float getThreshold() {
		return threshold;
	}

private:
	float threshold;
};

class CollageProblem : public GAProblem
{
public:
	virtual void setup();
	virtual void setRanges();
	virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);
	
	float width;
	float height;
	ofFbo mFbo;
	vector<ImageCache> mImages;
};
