#pragma once

#include "ofMain.h"
#include "ofxGALib.h"
#include "ofxOpenCv.h"
#include "Drawer.h"

class GAProblem : public ofThread 
{
public:
    GAProblem();
	string rootDir;
	void pickBest(ofImage& mImg1, ofImage& mImg2);
	float fitnessTest(const vector<float>& values);
    virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage) = 0;
	virtual void createPixelsFinal(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage) {
		createPixels(pixels, values, baseImage);
	}
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

	float width;
	float height;
	int mLevels;

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
	int mTimes;

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
	vector<ofImage> textures;
	string name;

	void loadImage(const string& filename, float maxWidth);

private:
	vector<BlobInfo*> blobs;
	void createBlobCvGray(ofxCvGrayscaleImage& cvImg);
	void createBlobs(ofxCvColorImage& cvImgColor, float threshold);
};

class CollageProblem : public GAProblem
{
public:
	CollageProblem();
	virtual void setup();
	virtual void setRanges();
	virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);
	virtual void createPixelsFinal(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);
	
	ofFbo mFbo;
	vector<ImageCache> mImages;
};

class StripProblem : public GAProblem
{
public:
	virtual void setup();
	virtual void setRanges();
	virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage);

	ofFbo mFbo;
};
