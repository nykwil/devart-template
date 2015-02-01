#pragma once

#include "ofMain.h"
#include "ofxGALib.h"
#include "ofxOpenCv.h"

float myMap01(float value, float outputMin, float outputMax, bool clamp); 
float myMap01(float value, float outputMin, float outputMax); 

class GAProblem : public ofThread 
{
public:
    GAProblem();
	string mRootDir;
	void pickBest(ofImage& mImg1, ofImage& mImg2);
	float fitnessTest(const vector<float>& values);

	void updateWorkImage();
	template<typename T>
	void updateCompImage(T& imgCmp)
	{
		static int iter = 0;
		if (iter % ITERS_PER_UPDATE == 0 && mutexComp.tryLock())
		{
			_mImgComp.setFromPixels(imgCmp.getPixels());
			mutexComp.unlock();
		}
		++iter;
	}

	virtual void createPixels(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage, int width, int height) = 0;
	void fillRandom(vector<float>& values);

	float compareImg(ofImage& imgNew, int method);

	float compHsb(ofImage &imgNew);

	float compBright(ofImage &imgNew);

	float compLookup(ofImage &imgNew);

	float compColorDelta(ofImage &imgNew);

	float compCanny(ofImage& imgNew);

	void createCanny(ofImage &imgNew, ofxCvGrayscaleImage &canny);

	virtual void setup();

	virtual void setRanges() = 0;
	virtual void threadedFunction();
	void go();
	void pushValuesFinal(const vector<float>& workingValues, const ofPixelsRef workingPixels);
	virtual void debugDraw() {}

    list<ofPixels> mFinalLayers;
    list< vector<float> > mFinalValues;
	ofPixels mWorkingPixels;
	ofPixels mFinalPixels;

    ofImage mImgCompare;
    ofImage mImgOrig;

	ofImage mLastWorking;
	ofImage mLastFinal;

    ofxGALib mGALib;
	int mPopSize;
	int mNGen;

	int mLevels;

	void getCompImg(ofImage& img);
    void getWorkImg(ofImage& img);
    void getFinalImg(ofImage& img);

    ofMutex mutexWork;
    ofMutex mutexComp;
    ofMutex mutexFinal;

    ofImage _mImgWork;
    ofImage _mImgComp;
    ofImage _mImgFinal;

    int mCompMethod;
    bool mUseDna;
    bool bFlattenAndSave;
    int mCompareWidth;
    int mCompareHeight;
	float mWorkingWidth;
	float mWorkingHeight;
	int mFinalWidth;
	int mFinalHeight;


    int mRepeat;
	int mTimes;

    vector<RangeInfo> mRanges;
};

