#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"

//TODO:
// save values for reconstruction
// make a historgram
// sort the historgram into a list
// choose only colors from the historgram by index
// make a look up matrix with all the distances of colours.
// better sort

class ColorLook
{
public:
    static unsigned int getIndex(unsigned char r, unsigned char g, unsigned char b)
    {
        return (r >> 3 << (5 * 2)) | (g >> 3 << 5) | b;
    }

    static ofColor getColour(unsigned int index)
    {
        int r = index >> (5 * 2) << 3;
        index -= r << (5 * 2);
        int g = index >> 5 << 3;
        index -= g << 5;
        int b = index << 3;
        return ofColor(r, g, b);
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

    vector< unsigned short > palette;

private:
    vector< vector< unsigned char > > colourLookup;
};

ColorLook mCL;

struct pred
{
    bool operator()(unsigned short const & a, unsigned short const & b) const
    {
        static int iblack = ColorLook::getIndex(255,255,255);
        return mCL.getDelta(a, iblack) < mCL.getDelta(b, iblack);
    }
};

GAProblem::GAProblem()
{
	mPopSize = 100;
	mNGen = 100;
}

void GAProblem::setup()
{
    mCL.buildColourLookup();        
    sort(mCL.palette.begin(), mCL.palette.end(), pred());

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

    mCompareWidth = 400;
    mCompareHeight = 400;
    mRepeat = 10;
    mImgCompare.resize(mCompareWidth, mCompareHeight);
    mCL.buildPalette(mImgCompare);
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
					diff += mCL.getDelta(img1.getColor(w, h), img2.getColor(w, h));
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
}

void GAProblem::go()
{
    static float lastFit = 0;

    if (mUseDna) {
		if (!mGALib.started) {
			mGALib.setup(mRanges, mRepeat, mPopSize, mNGen);
		}
        float result = mGALib.run(1);
		float fit = fitnessTest(mGALib.mOut);
		mWorkingPixels.clear();
		createPixels(mWorkingPixels, mGALib.mOut, mLayers.back());
		if (mGALib.done()) {
			if (fit > lastFit) {
				pushValues(mGALib.mOut, mWorkingPixels);
				lastFit = fit;
			}
			mGALib.setup(mRanges, mRepeat);
		}
    }
    else {
        static vector<float> workingValues;
        
        fillRandom(workingValues);
        float fit = fitnessTest(workingValues);
		mWorkingPixels.clear();
		createPixels(mWorkingPixels, workingValues, mLayers.back());
		/*
        if (fit > lastFit) {
            pushValues(workingValues, mWorkingPixels);
            lastFit = fit;
        }
		*/
    }

    if (bFlattenAndSave) {
        static int mLevels = 0;
        while (mLayers.size() > 1) {
            string s = "output/outimg" + ofToString(++mLevels, 2, 5, '0');
            mLayers.front().saveImage(s + ".jpg");
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

//////////////////////////////////////////////////////////////////////////

void FboProblem::setup()
{
    GAProblem::setup();

    mFbo.allocate(mImgOrig.getWidth(), mImgOrig.getHeight());
    mFbo.begin();
    ofClear(255,255,255, 0);
    mFbo.end();
}

void FboProblem::setRanges()
{
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b
    mRanges.push_back(RangeInfo(4, 0.f, 300.f)); // x
    mRanges.push_back(RangeInfo(4, 0.f, 300.f)); // y
    mRanges.push_back(RangeInfo(4, 20, 50)); // z
    mRanges.push_back(RangeInfo(4, 20, 50)); // w
}

void FboProblem::drawValues( const vector<float>& values )
{
    ofFill();
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
        float g = values[i++];
        float b = values[i++];

        float x = values[i++];
        float y = values[i++];

        float z = values[i++];
        float w = values[i++];

        ofFloatColor col(r,g,b);
        ofSetColor(col);
        ofRect(x,y,z,w);
        //        ofCircle(x,y,z);
        // ofBox(x,y,z,w);
    }
}

void FboProblem::createPixels( ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage )
{
    assert (values.size() == mRanges.size() * mRepeat);
    assert(mFbo.isAllocated());
    mFbo.begin();
    ofFill();
    ofSetColor(255);
    baseImage.draw(0,0);
    drawValues(values);
    mFbo.end();
    mFbo.readToPixels(pixels);
}

//////////////////////////////////////////////////////////////////////////

void BrushProblem::setRanges()
{
    mRanges.clear();
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b
    mRanges.push_back(RangeInfo(4, 0.f, mImgOrig.getWidth())); // x
    mRanges.push_back(RangeInfo(4, 0.f, mImgOrig.getHeight())); // y
    mRanges.push_back(RangeInfo(4, 20, 50)); // size
    mRanges.push_back(RangeInfo(4, 0, TWO_PI)); // ang
}

void BrushProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& imgBase)
{
    assert (values.size() == mRanges.size() * mRepeat);
    pixResult.setFromPixels(imgBase.getPixels(), imgBase.getWidth(), imgBase.getHeight(), imgBase.getPixelsRef().getNumChannels());
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
        float g = values[i++];
        float b = values[i++];
        float x = values[i++];
        float y = values[i++];
        float sz = values[i++];
        float ang = values[i++];
        mDraw.renderBrush(pixResult, ofVec2f(x, y), mBrush[1], sz, ang, ofFloatColor(r, g, b));
    }
}

//////////////////////////////////////////////////////////////////////////

void StrokeProblem::setRanges()
{
    mRanges.clear();
    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // r
    //    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // g
    //    mRanges.push_back(RangeInfo(4, 0.f, 1.f)); // b

    mRanges.push_back(RangeInfo(4, -PI, PI)); // ang
    mRanges.push_back(RangeInfo(4, 0.f, mImgOrig.getWidth())); // x
    mRanges.push_back(RangeInfo(4, 0.f, mImgOrig.getHeight())); // y

    mRanges.push_back(RangeInfo(4, -1.f, 1.f)); // dx
    mRanges.push_back(RangeInfo(4, -1.f, 1.f)); // dy
    mRanges.push_back(RangeInfo(4, 0, 1.f)); // size
}

float StrokeProblem::gDeltaSize = 5;
float StrokeProblem::gMinSize = 5;
float StrokeProblem::gMaxSize = 5;
float StrokeProblem::gRotation = 1;

void StrokeProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& imgBase)
{
    assert (values.size() == mRanges.size() * mRepeat);
    pixResult.setFromPixels(imgBase.getPixels(), imgBase.getWidth(), imgBase.getHeight(), imgBase.getPixelsRef().getNumChannels());
    int i = 0;
    for (int is = 0; is < mRepeat; ++is)
    {
        float r = values[i++];
//        float g = values[i++];
//        float b = values[i++];

        float sang = values[i++];
        float x = values[i++];
        float y = values[i++];

        float dx = values[i++] * gDeltaSize;
        float dy = values[i++] * gDeltaSize;

        float sz = ofLerp(gMinSize, gMaxSize, values[i++]);

        ofFloatColor col = mCL.getPalette(r);

        mDraw.stroke(pixResult, mBrush[1], col, sz, sang * gRotation, ofVec2f(x, y), ofVec2f(x + (dx * sz), y + (dy * sz)));
    }
}

void StrokeProblem::setup()
{
	GAProblem::setup();
//	mBrush[0].loadImage(ofToDataPath("brushtest.png"));
//	mBrush[1].loadImage(ofToDataPath("softbrush2.png"));
	//    mBrush[1].loadImage(ofToDataPath("circlebrush.png"));
}

int gSmoothingSize = 1;
float gSmoothingShape = 0;
float gMinSize =  500;
float gMaxSize = 40000;
int gConsider = 250;
float gTolerance = 1.0f;
float gMinThresh = 0;
float gMaxThresh = 250.0f;
float gThreshThresh = 50.0f;

bool gDebugOn = false;
float gDebugThresh = 500;
float gDebugImage = 0;

void CollageProblem::setup()
{
	GAProblem::setup();

	mCompMethod = 5;
	mUseDna = true;
	bFlattenAndSave = true;
	width = mImgOrig.getWidth();
	height = mImgOrig.getHeight();

	gui.addSlider("CompMethod", mCompMethod, 0, 9);
	gui.addToggle("UseDna", mUseDna);
	gui.addToggle("FlattenAndSave", bFlattenAndSave);
	gui.addSlider("consider", gConsider, 0.0f, 500.0f);
	gui.addSlider("minsize", gMinSize, 0.0f, 1000.0f);
	gui.addSlider("maxsize", gMaxSize, 0.0f, 40000.0f);
	gui.addSlider("smoothingSize", gSmoothingSize, 0, 500);
	gui.addSlider("smoothingShape", gSmoothingShape, 0.0f, 1.0f);
	gui.addSlider("tolerance", gTolerance, 0.0f, 10.0f);

	gui.addSlider("MinThresh", gMinThresh, 0.0f, 1000.0f);
	gui.addSlider("MaxThresh", gMaxThresh, 0.0f, 1000.0f);
	gui.addSlider("gThreshThresh", gThreshThresh, 0.0f, 100.0f);

	gui.addToggle("gDebugOn", gDebugOn);
	gui.addSlider("gDebugThresh", gDebugThresh, 0.0f, 1.0f);
	gui.addSlider("gDebugImage", gDebugImage, 0.0f, 0.99999f);

	mImages.resize(5);
	for (int i = 0; i < mImages.size(); ++i) {
		mImages[i].loadImage("source/img (" + ofToString(i + 1) + ").jpg", width, height);
	}

	mFbo.allocate(width, height);
	mFbo.begin();
	ofClear(255,255,255, 0);
	mFbo.end();
}

void ImageCache::createBlobCvGray(ofxCvGrayscaleImage& cvImg) {

	contourFinder.findContours(cvImg, gMinSize, gMaxSize, gConsider, false, true);

	for(int i = 0; i < contourFinder.blobs.size(); i++) {
		BlobInfo* info = new BlobInfo();
		info->centroid = contourFinder.blobs[i].centroid;
		info->line.addVertices(contourFinder.blobs[i].pts);
		info->line.setClosed(true);
		info->line.simplify(gTolerance);
//		info->line.getSmoothed(gSmoothingSize, gSmoothingShape);

		tess.tessellateToMesh(info->line, OF_POLY_WINDING_ODD, info->mesh, true);

		vector<ofVec3f>& verts = info->mesh.getVertices();
		for (int i = 0; i < verts.size(); ++i) {
			info->mesh.addTexCoord(ofVec2f(verts[i].x, verts[i].y));
		}
		info->mesh.disableColors();
		info->mesh.disableNormals();
		blobs.push_back(info);
	}
}

void ImageCache::createBlobs(float threshold) {
	for (int i = 0; i < blobs.size(); ++i) {
		delete blobs[i];
	}
	blobs.clear();

	cvImgGrayscale.setFromColorImage(cvImgColor);
	cvImgGrayscale.threshold(threshold, true);
	createBlobCvGray(cvImgGrayscale);

	cvImgGrayscale.setFromColorImage(cvImgColor);
	cvImgGrayscale.threshold(threshold, false);
	createBlobCvGray(cvImgGrayscale);

	this->threshold = threshold;
}

void ImageCache::loadImage(string filename, float width, float height)
{
	image.loadImage(filename);
	float diff = image.getWidth() / image.getHeight();
	image.resize(width, height / diff);

	cvImgColor.setFromPixels(image.getPixelsRef());

	if (cvImgGrayscale.getWidth() == cvImgColor.getWidth() || cvImgGrayscale.getHeight() != cvImgColor.getHeight())
		cvImgGrayscale.allocate(cvImgColor.getWidth(), cvImgColor.getHeight());
}

enum RangeType {
	RT_X,
	RT_Y,
	RT_DEG,
	RT_SCALE,
	RT_IMAGE,
	RT_TRESH,
	RT_BLOB,
	RT_MAX
};

void CollageProblem::setRanges()
{
	mRanges.resize(RT_MAX);

	mRanges[RT_X] = RangeInfo(4, 0.f, mImgOrig.getWidth()); // x
	mRanges[RT_Y] = RangeInfo(4, 0.f, mImgOrig.getHeight()); // y
	mRanges[RT_DEG] = RangeInfo(4, 0.f, 360.0f); // y
	mRanges[RT_SCALE] = RangeInfo(4, 0.5f, 2.0f); // y
	mRanges[RT_IMAGE] = RangeInfo(4, 0.f, 0.99999f); // y
	mRanges[RT_TRESH] = RangeInfo(4, 0.f, 1.0f); // y
	mRanges[RT_BLOB] = RangeInfo(4, 0.f, 0.99999f); // image
}

void CollageProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& baseImage)
{
	if (mImages.size() == 0)
		return;

	mFbo.begin();
	// start draw
	ofFill();

	if (gDebugOn) {
		ImageCache& image = mImages[gDebugImage * mImages.size()];
		float thresh = ofMap(gDebugThresh, 0, 1, gMinThresh, gMaxThresh);
		if (gThreshThresh < fabs(image.getThreshold() - thresh)) {
			image.createBlobs(thresh);
		}
		image.cvImgGrayscale.draw(0,0);
		for (int j = 0; j < image.blobs.size(); ++j) {
			ofSetColor(255);
			image.image.bind();
			image.blobs[j]->mesh.draw(OF_MESH_FILL);
			image.image.unbind();
		}
	}
	else {
		for (int i = 0; i < mRepeat; ++i) {
			float x = values[i * RT_MAX + RT_X];
			float y = values [i * RT_MAX + 1];
			float deg = values[i * RT_MAX + 2];
			float scale = values[i * RT_MAX + 3];
			int iimg = (int)(values[i * RT_MAX + RT_IMAGE] * (float)mImages.size());
			float thresh = ofMap(values[i * RT_MAX + RT_TRESH], 0, 1, gMinThresh, gMaxThresh);

			ImageCache& image = mImages[iimg];

			ofSetColor(255);
			baseImage.draw(0, 0);
			if (gThreshThresh < fabs(image.getThreshold() - thresh)) {
				image.createBlobs(thresh);
			}

			if (image.blobs.size() > 0 ) {

				int j = values[RT_BLOB] * image.blobs.size();

				ofTranslate(x, y);
				ofRotateZ(deg);
				ofScale(scale, scale, scale);
				ofTranslate(-image.blobs[j]->centroid.x, -image.blobs[j]->centroid.y);
				ofSetColor(255);
				image.image.bind();
				image.blobs[j]->mesh.draw(OF_MESH_FILL);
				image.image.unbind();
			}
		}
	}

	// end draw
	mFbo.end();
	mFbo.readToPixels(pixResult);
}