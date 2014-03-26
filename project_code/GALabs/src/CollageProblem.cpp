#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"

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

	mCompMethod = 7;
	mUseDna = true;
	bFlattenAndSave = true;
	width = mImgOrig.getWidth();
	height = mImgOrig.getHeight();
	mRepeat = 1;
	mPopSize = 200;
	mNGen = 200;

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

	mImages.resize(8);
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

	mRanges[RT_X] = RangeInfo(4, 10.f, mImgOrig.getWidth() - 10.f); // x
	mRanges[RT_Y] = RangeInfo(4, 10.f, mImgOrig.getHeight() - 10.f); // y
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
		ofSetColor(255);
		baseImage.draw(0, 0);
		for (int i = 0; i < mRepeat; ++i) {
			float x = values[i * RT_MAX + RT_X];
			float y = values [i * RT_MAX + RT_Y];
			float deg = values[i * RT_MAX + RT_DEG];
			float scale = values[i * RT_MAX + RT_SCALE];
			int iimg = (int)(values[i * RT_MAX + RT_IMAGE] * (float)mImages.size());
			float thresh = ofMap(values[i * RT_MAX + RT_TRESH], 0, 1, gMinThresh, gMaxThresh);
			assert(0.5f <= scale && scale <= 2.0f);

			ImageCache& image = mImages[iimg];

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