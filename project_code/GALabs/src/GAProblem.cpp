#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ColorLook.h"
#include <ppl.h>

int ITERS_PER_UPDATE = 10;

float myMap01(float value, float outputMin, float outputMax) {
	return (value * (outputMax - outputMin) + outputMin);
}

float myMap01(float value, float outputMin, float outputMax, bool clamp) {

	float outVal = (value * (outputMax - outputMin) + outputMin);

	if (clamp) {
		if (outputMax < outputMin) {
			if (outVal < outputMax)
				outVal = outputMax;
			else if (outVal > outputMin)
				outVal = outputMin;
		}
		else {
			if (outVal > outputMax)
				outVal = outputMax;
			else if (outVal < outputMin)
				outVal = outputMin;
		}
	}
	return outVal;
}

GAProblem::GAProblem() {
	mRootDir = ofToDataPath("");
	mCompMethod = 5;
	mUseDna = true;
	bFlattenAndSave = true;

	mPopSize = 10;
	mNGen = 10;

	mCompareWidth = 100;
	mRepeat = 1;
	mWorkingWidth = 600;
	mLevels = 0;

	gui.addSlider("CompMethod", mCompMethod, 0, 9);
	gui.addToggle("UseDna", mUseDna);
	gui.addSlider("Times", mTimes, 0, 50);
	gui.addSlider("Repeat", mRepeat, 0, 100);
	gui.addSlider("PopSize", mPopSize, 0, 200);
	gui.addSlider("NGen", mNGen, 0, 200);
	gui.addToggle("FlattenAndSave", bFlattenAndSave);
}

void GAProblem::setup() {

	{
		ofDirectory dir(mRootDir + "/target/");
		dir.allowExt("jpg");
		int nd = dir.listDir();
		mImgOrig.loadImage(dir.getPath(rand() % nd));
	}

	mWorkingHeight = mWorkingWidth / (mImgOrig.getWidth() / mImgOrig.getHeight());
	mImgOrig.resize(mWorkingWidth, mWorkingHeight);

	mFinalWidth = mWorkingWidth * 4;
	mFinalHeight = mWorkingHeight * 4;

	mImgCompare = mImgOrig;

	mCompareHeight = mCompareWidth / (mImgOrig.getWidth() / mImgOrig.getHeight());
	mImgCompare.resize(mCompareWidth, mCompareHeight);

	ofImage img;
	ofFile file(mRootDir + "/palette.jpg");
	if (file.exists()) {
		img.loadImage(mRootDir + "/palette.jpg");
		ColorLook::instance().buildPalette(img);
	}
	else {
		ColorLook::instance().buildPalette(mImgCompare);
	}

	{
		ofDirectory dir(mRootDir + "/output/");
		dir.allowExt("png");
		mLevels = dir.listDir();
		if (mLevels > 0) {
			img.loadImage(mRootDir + "/output/outimg" + ofToString(mLevels, 2, 5, '0') + ".png");
		}
		else {
			ofFile file(mRootDir + "/empty.jpg");
			if (file.exists()) {
				img.loadImage(mRootDir + "/empty.jpg");
			}
			else {
				img = mImgOrig;
				img.resize(1,1);
			}
		}
	}

	mLastWorking = img;
	mLastWorking.resize(mWorkingWidth, mWorkingHeight); // @TODO

	mLastFinal = img;
	mLastFinal.resize(mFinalWidth, mFinalHeight); // @TODO

	mFinalLayers.push_back(mLastFinal);
	mFinalValues.push_back(vector<float>());

	mGALib.setFitness(this, &GAProblem::fitnessTest);

	setRanges();
}

float GAProblem::fitnessTest(const vector<float>& values) {
	createPixels(mWorkingPixels, values, mLastWorking, mWorkingWidth, mWorkingHeight);

	ofImage compImage;

	compImage.setFromPixels(mWorkingPixels);
	compImage.resize(mCompareWidth, mCompareHeight);

	assert(mImgCompare.getWidth() == compImage.getWidth() && mImgCompare.getHeight() == compImage.getHeight());

	return compareImg(compImage, mCompMethod);
}

void GAProblem::fillRandom(vector<float>& values) {
	values.resize(mRanges.size() * mRepeat);
	for (int i = 0; i < values.size(); ++i) {
		values[i] = ofRandom(mRanges[i % mRanges.size()].mMin, mRanges[i % mRanges.size()].mMax);
	}
}

enum CompareTypes {
	CT_0,
	CT_1,
	CT_2,
	CT_3,
	CT_HSB,
	CT_DELTA,
	CT_LOOKUP,
	CT_BRIGHT,
	CT_CANNY
};

float GAProblem::compareImg(ofImage& imgNew, int method) {
	assert(mImgCompare.getWidth() == imgNew.getWidth() && mImgCompare.getHeight() == imgNew.getHeight());

	if (method < 4) {
		static ofxCvColorImage cvi1;
		static ofxCvColorImage cvi2;
		cvi1.setFromPixels(mImgCompare.getPixels(), mImgCompare.getWidth(), mImgCompare.getHeight());
		cvi2.setFromPixels(imgNew.getPixels(), imgNew.getWidth(), imgNew.getHeight());

		cv::Mat src_base, hsv_base;
		cv::Mat src_test, hsv_test;
		cv::Mat hsv_half_down;

		src_base = cv::cvarrToMat(cvi1.getCvImage());
		src_test = cv::cvarrToMat(cvi2.getCvImage());

		/// Convert to HSV
		cvtColor(src_base, hsv_base, CV_BGR2HSV);
		cvtColor(src_test, hsv_test, CV_BGR2HSV);

		hsv_half_down = hsv_base(cv::Range(hsv_base.rows/2, hsv_base.rows - 1), cv::Range(0, hsv_base.cols - 1));

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
		calcHist(&hsv_base, 1, channels, cv::Mat(), hist_base, 2, histSize, ranges, true, false);
		normalize(hist_base, hist_base, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

		calcHist(&hsv_test, 1, channels, cv::Mat(), hist_test, 2, histSize, ranges, true, false);
		normalize(hist_test, hist_test, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

		double base_test = compareHist(hist_base, hist_test, method);
		return base_test;
	}
	else if (method == 4) {
		return compHsb(imgNew);
	}
	else if (method == 5) {
		return compColorDelta(imgNew);
	}
	if (method == 6) {
		return compLookup(imgNew);
	}
	else if (method == 7) {
		return compBright(imgNew);
	}
	else if (method == 8) {
		return compCanny(imgNew);
	}
	else {
		return 9000;
	}
}

void GAProblem::threadedFunction() {
	while (isThreadRunning()) {
		go();
	}
}

void GAProblem::getCompImg(ofImage& img) {
	mutexComp.lock();
	img = _mImgComp;
	mutexComp.unlock();
}

void GAProblem::getWorkImg(ofImage& img) {
	mutexWork.lock();
	img = _mImgWork;
	mutexWork.unlock();
}

void GAProblem::getFinalImg(ofImage& img) {
	mutexFinal.lock();
	img = _mImgFinal;
	mutexFinal.unlock();
}

void saveFloat(const string& filename, vector<float> values) {
	ofFile file(filename, ofFile::Mode::WriteOnly);
	for (int i = 0; i < values.size(); ++i) {
		string s = ofToString(values[i]);
		if (i != values.size() - 1) {
			s += ", ";
		}
		file.write(s.c_str(), s.length());
	}
}

void GAProblem::go() {
	static float mLastFit = 0;
	static float mStartImageTimer = 0;

	if (mUseDna && !gui.isOn()) {
		if (!mGALib.started) {
			mStartImageTimer = ofGetElapsedTimef();
			mGALib.setup(mRanges, mRepeat, mPopSize, mNGen);
		}
		float startt = ofGetElapsedTimef();
		float result = mGALib.run(mTimes);
		cout << ("est time: " + ofToString((ofGetElapsedTimef() - startt) * mNGen)).c_str() << "\t";
		if (mGALib.done()) {
			float fit = fitnessTest(mGALib.mOut);
			updateWorkImage();
			cout << ("\nactual time: " + ofToString((ofGetElapsedTimef() - mStartImageTimer) * mNGen)).c_str() << "\n";

			if (fit > mLastFit) {
				createPixels(mFinalPixels, mGALib.mOut, mLastFinal, mFinalWidth, mFinalHeight);
				pushValuesFinal(mGALib.mOut, mFinalPixels);
				mLastFit = fit;
			}
			mGALib.started = false;
		}
	}
	else {
		vector<float> values;

		fillRandom(values);
		float fit = fitnessTest(values);

		createPixels(mFinalPixels, values, mLastFinal, mFinalWidth, mFinalHeight);
		createPixels(mWorkingPixels, values, mLastWorking, mWorkingWidth, mWorkingHeight);
		if (fit > mLastFit && !gui.isOn()) {
			pushValuesFinal(values, mFinalPixels);
			mLastFit = fit;
		}

		mutexWork.lock();
		_mImgWork.setFromPixels(mWorkingPixels);
		mutexWork.unlock();
	}

	if (bFlattenAndSave) {
		while (mFinalLayers.size() > 1) {
			string s = mRootDir + "/output/outimg" + ofToString(++mLevels, 2, 5, '0');
			ofSaveImage(mFinalLayers.front(), s + ".png", OF_IMAGE_QUALITY_BEST);
			mFinalLayers.pop_front();

			saveFloat(s + ".txt", mFinalValues.front());
			mFinalValues.pop_front();
		}
	}
}

// not working pixels final pixels
void GAProblem::pushValuesFinal(const vector<float>& values, const ofPixelsRef pixels) {
	mFinalValues.push_back(values);

	mLastFinal.setFromPixels(pixels);
	mFinalLayers.push_back(pixels);

	mLastWorking = mLastFinal;
	mLastWorking.resize(mWorkingWidth, mWorkingHeight); // @TODO

	mutexFinal.lock();
	_mImgFinal.setFromPixels(pixels);
	mutexFinal.unlock();
}

void GAProblem::updateWorkImage()
{
	static int iter = 0;
	if (iter % ITERS_PER_UPDATE == 0 && mutexWork.tryLock())
	{
		_mImgWork.setFromPixels(mWorkingPixels);
		mutexWork.unlock();
	}
	++iter;
}

ofxCvGrayscaleImage cvCannyImgComp;

using namespace concurrency;

void GAProblem::createCanny(ofImage &imgNew, ofxCvGrayscaleImage &canny)
{
	ofxCvGrayscaleImage cvImgGray;
	imgNew.setImageType(OF_IMAGE_GRAYSCALE);
	cvImgGray.allocate(imgNew.getWidth(), imgNew.getHeight());
	cvImgGray.setFromPixels(imgNew.getPixelsRef());

	canny.allocate(cvImgGray.getWidth(), cvImgGray.getHeight());  
	cvCanny(cvImgGray.getCvImage(), canny.getCvImage(), 50.0, 120.0);
}

float GAProblem::compCanny(ofImage& imgNew) {
	if (cvCannyImgComp.getWidth() == 0) {
		createCanny(mImgCompare, cvCannyImgComp);
		updateCompImage(cvCannyImgComp);
	}

	ofxCvGrayscaleImage canny;  
	createCanny(imgNew, canny);
	updateCompImage(canny);

	auto newpix = canny.getPixels();
	auto oldpix = cvCannyImgComp.getPixels();

	combinable<float> sum; 
	parallel_for(int(0), (int)(canny.getWidth() * canny.getHeight()), [&](int i)
	{
		sum.local() += 255 - abs(oldpix[i] - newpix[i]);
	});
	return sum.combine(plus<float>());
}

float GAProblem::compColorDelta(ofImage &imgNew)
{
	updateCompImage(imgNew);

	combinable<float> sum;
	parallel_for(int(0), (int)(imgNew.getWidth() * imgNew.getHeight()), [&](int i)
	{
		ofFloatColor c1 = mImgCompare.getColor(i);
		ofFloatColor c2 = imgNew.getColor(i);

		float delta = ColorCompare::deltaE1976(
			ColorRGB(c1.r, c1.g, c1.b, false).toLinearRGB().toXYZ().toLab(),
			ColorRGB(c2.r, c2.g, c2.b, false).toLinearRGB().toXYZ().toLab()
			);
		sum.local() += ColorLook::instance().getMaxDelta() - delta;
	});
	return sum.combine(plus<float>());
}

float GAProblem::compLookup(ofImage &imgNew)
{
	updateCompImage(imgNew);

	combinable<float> sum;
	parallel_for(int(0), (int)(imgNew.getWidth() * imgNew.getHeight()), [&](int i)
	{
		sum.local() += ColorLook::instance().getDelta(mImgCompare.getColor(i), imgNew.getColor(i));
	});
	return sum.combine(plus<float>());
}

float GAProblem::compBright(ofImage &imgNew)
{
	updateCompImage(imgNew);

	combinable<float> sum;
	parallel_for(int(0), (int)(imgNew.getWidth() * imgNew.getHeight()), [&](int i)
	{
		sum.local() += ColorCompare::BrightDiff(mImgCompare.getColor(i), imgNew.getColor(i));
	});
	return sum.combine(plus<float>());
}

float GAProblem::compHsb(ofImage &imgNew)
{
	updateCompImage(imgNew);

	combinable<float> sum;
	parallel_for(int(0), (int)(imgNew.getWidth() * imgNew.getHeight()), [&](int i)
	{
		sum.local() += ColorCompare::HsbDiff(mImgCompare.getColor(i), imgNew.getColor(i));
	});
	return sum.combine(plus<float>());
}

