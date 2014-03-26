#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"

GAProblem::GAProblem()
{
}

void GAProblem::setup()
{
	ColorLook::instance().sort();

    mImgOrig.loadImage(ofToDataPath("portr4.jpg"));
    mImgOrig.resize(400, 400);
    mImgCompare = mImgOrig;

    mLayers.push_back(ofImage());
    mLayers.back().loadImage(ofToDataPath("emptyblack.jpg"));
    mLayers.back().resize(mImgOrig.getWidth(), mImgOrig.getHeight());
    mLayerValues.push_back(vector<float>());

    mGALib.setFitness(this, &GAProblem::fitnessTest);
    mCompMethod = 5;
	mUseDna = true;
	bFlattenAndSave = true;

	mPopSize = 10;
	mNGen = 10;

    mCompareWidth = 200;
    mCompareHeight = 200;
    mRepeat = 1;
    mImgCompare.resize(mCompareWidth, mCompareHeight);
	ColorLook::instance().buildPalette(mImgCompare);
    setRanges();
}

int ITERS_PER_UPDATE = 10;

float GAProblem::fitnessTest( const vector<float>& values )
{
    mWorkingPixels.clear();
    createPixels(mWorkingPixels, values, mLayers.back());

    static ofImage workingImage;
    workingImage.setFromPixels(mWorkingPixels);
    workingImage.resize(mCompareWidth, mCompareHeight);

    assert(mImgCompare.getWidth() == workingImage.getWidth() && mImgCompare.getHeight() == workingImage.getHeight());

    float fitness = compareImg(mImgCompare, workingImage, mCompMethod);

    static int iter = 0;
    if (iter % ITERS_PER_UPDATE == 0 && mutexWork.tryLock())
    {
        _mImgWork.setFromPixels(mWorkingPixels);
        mutexWork.unlock();
    }
    ++iter;

    if (fitness > mBestValue)
    {
        mutexBest.lock();
        _mImgBest.setFromPixels(mWorkingPixels);
        mutexBest.unlock();

        mBestValue = fitness;
    }
    return fitness;
}

void GAProblem::fillRandom(vector<float>& values)
{
    values.resize(mRanges.size() * mRepeat);
    for (int i = 0; i < values.size(); ++i) {
        values[i] = ofRandom(mRanges[i % mRanges.size()].mMin, mRanges[i % mRanges.size()].mMax);
    }
}

// @TODO fix
float GetBright(ofColor col) {
	return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
}

float GAProblem::compareImg(ofImage& img1, ofImage& img2, int method)
{
    assert(img1.getWidth() == img2.getWidth() && img1.getHeight() == img2.getHeight());

	if (method >= 4) {
        float diff = 0;
		if (method == 7) {
			for (int w = 0; w < img1.getWidth(); ++w) {
				for (int h = 0; h < img1.getHeight(); ++h) {
					diff +=  255 - abs(GetBright(img1.getColor(w, h)) - GetBright(img2.getColor(w, h)));
				}
			}
		}
		if (method == 6) {
			for (int w = 0; w < img1.getWidth(); ++w) {
				for (int h = 0; h < img1.getHeight(); ++h) {
					diff += ColorLook::instance().getDelta(img1.getColor(w, h), img2.getColor(w, h));
				}
			}
		}
		else if (method == 5) {
			for (int w = 0; w < img1.getWidth(); ++w) {
				for (int h = 0; h < img1.getHeight(); ++h) {
					ofFloatColor c1 = img1.getColor(w, h);
					ofFloatColor c2 = img2.getColor(w, h);

					float delta = ColorCompare::deltaE1976(
						ColorRGB(c1.r, c1.g, c1.b, false).toLinearRGB().toXYZ().toLab(),
						ColorRGB(c2.r, c2.g, c2.b, false).toLinearRGB().toXYZ().toLab()
						);
					diff += ColorCompare::getMaxDelta() - delta;
				}
			}
		}
		else if (method == 4) {
			for (int w = 0; w < img1.getWidth(); ++w) {
				for (int h = 0; h < img1.getHeight(); ++h) {
					diff += ColorCompare::HsbDiff(img1.getColor(w, h), img2.getColor(w, h));
				}
			}
		}
		
        return diff;
    }
	else {
		static ofxCvColorImage cvi1;
		static ofxCvColorImage cvi2;
		cvi1.setFromPixels(img1.getPixels(), img1.getWidth(), img1.getHeight());
		cvi2.setFromPixels(img2.getPixels(), img2.getWidth(), img2.getHeight());

		cv::Mat src_base, hsv_base;
		cv::Mat src_test, hsv_test;
		cv::Mat hsv_half_down;

		src_base = cv::cvarrToMat(cvi1.getCvImage());
		src_test = cv::cvarrToMat(cvi2.getCvImage());

		/// Convert to HSV
		cvtColor( src_base, hsv_base, CV_BGR2HSV );
		cvtColor( src_test, hsv_test, CV_BGR2HSV );

		hsv_half_down = hsv_base( cv::Range( hsv_base.rows/2, hsv_base.rows - 1 ), cv::Range( 0, hsv_base.cols - 1 ) );

		/// Using 30 bins for hue and 32 for saturation
		int h_bins = 50; int s_bins = 60;
		int histSize[] = { h_bins, s_bins };

		// hue varies from 0 to 256, saturation from 0 to 180
		float h_ranges[] = { 0, 256 };
		float s_ranges[] = { 0, 180 };

		const float* ranges[] = { h_ranges, s_ranges };

		// Use the o-th and 1-st channels
		int channels[] = { 0, 1 };

		/// Histograms
		cv::MatND hist_base;
		cv::MatND hist_half_down;
		cv::MatND hist_test;

		/// Calculate the histograms for the HSV images
		calcHist( &hsv_base, 1, channels, cv::Mat(), hist_base, 2, histSize, ranges, true, false );
		normalize( hist_base, hist_base, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

		calcHist( &hsv_test, 1, channels, cv::Mat(), hist_test, 2, histSize, ranges, true, false );
		normalize( hist_test, hist_test, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

		double base_test = compareHist( hist_base, hist_test, method );
		return base_test;
	}
}

void GAProblem::threadedFunction()
{
    while (isThreadRunning())
    {
        go();
    }
}

void GAProblem::getBestImg( ofImage& img )
{
    mutexBest.lock();
    img = _mImgBest;
    mutexBest.unlock();
}

void GAProblem::getWorkImg( ofImage& img )
{
    mutexWork.lock();
    img = _mImgWork;
    mutexWork.unlock();
}

void GAProblem::getLastImg( ofImage& img )
{
    mutexLast.lock();
	img = _mImgLast;
    mutexLast.unlock();
}

void saveFloat(const string& filename, vector<float> values)
{
    ofFile file(filename, ofFile::Mode::WriteOnly);
	for (int i = 0; i < values.size(); ++i) {
		string s = ofToString(values[i]);
		if (i != values.size() - 1) {
			s += ", ";
		}
		file.write(s.c_str(), s.length());
	}
}

void GAProblem::go()
{
    static float lastFit = 0;

    if (mUseDna) {
		if (!mGALib.started) {
			mGALib.setup(mRanges, mRepeat, mPopSize, mNGen);
		}
		float startt = ofGetElapsedTimef();
        float result = mGALib.run(1);
		float endt = ofGetElapsedTimef();

		cout << ("time: " + ofToString((endt - startt)* mNGen)).c_str() << endl;;
		float fit = fitnessTest(mGALib.mOut);
		if (mGALib.done()) {
			if (fit > lastFit) {
				mWorkingPixels.clear();
				createPixels(mWorkingPixels, mGALib.mOut, mLayers.back());
				pushValues(mGALib.mOut, mWorkingPixels);
				lastFit = fit;
			}
			mGALib.started = false;
		}
    }
    else {
        static vector<float> workingValues;
        
        fillRandom(workingValues);
        float fit = fitnessTest(workingValues);
        if (fit > lastFit) {
			mWorkingPixels.clear();
			createPixels(mWorkingPixels, workingValues, mLayers.back());
            pushValues(workingValues, mWorkingPixels);
            lastFit = fit;
        }
    }

    if (bFlattenAndSave) {
        static int mLevels = 0;
        while (mLayers.size() > 1) {
            string s = "output/outimg" + ofToString(++mLevels, 2, 5, '0');
            mLayers.front().saveImage(s + ".png");
            mLayers.pop_front();
            saveFloat(s + ".txt", mLayerValues.front());
            mLayerValues.pop_front();
        }
    }
}

void GAProblem::pushValues( const vector<float>& workingValues, const ofPixelsRef workingPixels )
{
    mLayerValues.push_back(workingValues);
    mLayers.push_back(ofImage());
    mLayers.back().setFromPixels(workingPixels);

	mutexLast.lock();
    _mImgLast.setFromPixels(workingPixels);
    mutexLast.unlock();
}
