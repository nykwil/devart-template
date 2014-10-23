#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"

float gMinSize =  500;
float gMaxSize = 1000000;
int gConsider = 250;
float gTolerance = 0.1f;
float gMinThresh = 0;
float gMaxThresh = 250.0f;
float gThreshThresh = 50.0f;

bool gDebugOn = false;
float gDebugThresh = 500;
float gDebugImage = 0;

CollageProblem::CollageProblem() : GAProblem()
{
	mCompMethod = 7;
	mUseDna = true;
	bFlattenAndSave = true;
	mRepeat = 1;
	mPopSize = 20;
	mNGen = 20;

	gui.addTitle("-- Collage --");
	gui.addSlider("Consider", gConsider, 0.0f, 500.0f);
	gui.addSlider("MinSize", gMinSize, 0.0f, 1000.0f);
	gui.addSlider("MaxSize", gMaxSize, 0.0f, 1000000.0f);
	gui.addSlider("Tolerance", gTolerance, 0.0f, 10.0f);

	gui.addSlider("MinThresh", gMinThresh, 0.0f, 1000.0f);
	gui.addSlider("MaxThresh", gMaxThresh, 0.0f, 1000.0f);
	gui.addSlider("ThreshThresh", gThreshThresh, 0.0f, 100.0f);

	gui.addTitle("-- Debug --");
	gui.addToggle("DebugOn", gDebugOn);
	gui.addSlider("DebugThresh", gDebugThresh, 0.0f, 1.0f);
	gui.addSlider("DebugImage", gDebugImage, 0.0f, 0.99999f);
}

void CollageProblem::setup()
{
	GAProblem::setup();

	ofDirectory dir(rootDir + "source/");
	dir.allowExt("jpg");
	int nd = dir.listDir();

	mImages.resize(nd);
	for (int i = 0; i < nd; ++i) {
		mImages[i].loadImage(dir.getPath(i), width * 1.5f);
	}

	ofFbo::Settings sett;
	sett.width = width;
	sett.height = height;
	mFbo.allocate(sett);
	mFbo.begin();
	ofClear(255,255,255, 0);
	mFbo.end();
}

void ImageCache::createBlobCvGray(ofxCvGrayscaleImage& cvImg) {
	ofTessellator tess;
	ofxCvContourFinder contourFinder;

	contourFinder.findContours(cvImg, gMinSize, cvImg.getWidth() * cvImg.getHeight() * 0.8f, gConsider, false, true);

	for(int i = 0; i < contourFinder.blobs.size(); i++) {
		ofRectangle& rect = contourFinder.blobs[i].boundingRect;

// 		if (abs(rect.x) > 1 && 
// 			abs(rect.y) > 1 &&
// 			abs(rect.x + rect.width) < cvImg.getWidth() &&
// 			abs(rect.y + rect.height) < cvImg.getHeight() 
// 			) {

 		if (abs(rect.width - cvImg.getWidth()) > 5 && 
 			abs(rect.height - cvImg.getHeight()) > 5) {

			BlobInfo* info = new BlobInfo();
			info->centroid = contourFinder.blobs[i].centroid;
			info->line.addVertices(contourFinder.blobs[i].pts);
			info->line.setClosed(true);
			info->line.simplify(gTolerance);

			tess.tessellateToMesh(info->line, OF_POLY_WINDING_ODD, info->mesh, true);

			vector<ofVec3f>& verts = info->mesh.getVertices();
			for (int i = 0; i < verts.size(); ++i) {
				info->mesh.addTexCoord(ofVec2f(verts[i].x, verts[i].y));
			}
			info->mesh.disableColors();
			info->mesh.disableNormals();
//			blobs.push_back(info);

			ofFbo fbo;
			ofPixels pixResult;
			ofImage workingImage;
			ofFbo::Settings sett;
			sett.width = rect.width;
			sett.height = rect.height;
			fbo.allocate(sett);

			fbo.begin();
			ofClear(255,255,255, 0);
			ofEnableAlphaBlending();
			ofSetColor(255, 255, 255);
			image.bind();
			ofPushMatrix();
			ofTranslate(-rect.x, -rect.y);
			info->mesh.draw();
			ofPopMatrix();
			image.unbind();
			fbo.end();

			pixResult.clear();
			fbo.readToPixels(pixResult);
			workingImage.setFromPixels(pixResult);

			if (rect.getLeft() < 2 || 
				image.width - rect.getRight() < 2 ||
				rect.getTop() < 2 ||
				image.height - rect.getBottom() < 2)
			{
				static int index = 0;
				ofSaveImage(workingImage.getPixels(), "test/" + name + "_bad" + ofToString(index++) + " .png", OF_IMAGE_QUALITY_BEST);
			}
			else 
			{
				static int index = 0;
				ofSaveImage(workingImage.getPixels(), "test/" + name + "_good" + ofToString(index++) + " .png", OF_IMAGE_QUALITY_BEST);
				textures.push_back(workingImage);
			}
		}
	}

	//@TODO sort
}

void ImageCache::createBlobs(ofxCvColorImage& cvImgColor, float threshold) {
	for (int i = 0; i < blobs.size(); ++i) {
		delete blobs[i];
	}
	blobs.clear();

	ofxCvGrayscaleImage cvImgGrayscale;
	cvImgGrayscale.allocate(cvImgColor.width, cvImgColor.height);

	cvImgGrayscale.setFromColorImage(cvImgColor);
	cvImgGrayscale.threshold(threshold, false);
	createBlobCvGray(cvImgGrayscale);

	cvImgGrayscale.setFromColorImage(cvImgColor);
	cvImgGrayscale.invert();
	cvImgGrayscale.threshold(threshold, false);
	createBlobCvGray(cvImgGrayscale);
}

void ImageCache::loadImage(const string& filename, float maxWidth)
{
	name = filename;
	image.loadImage(filename);

	if (image.getWidth() > maxWidth) {
		float diff = image.getWidth() / image.getHeight();
		image.resize(maxWidth, maxWidth / diff);
	}

	ofxCvColorImage cvImgColor;
	cvImgColor.allocate(image.getWidth(), image.getHeight());
	cvImgColor.setFromPixels(image.getPixelsRef());
	for (float f = 0; f < 10.f; f += 1.0f) {
		createBlobs(cvImgColor, ofMap(f, 0, 10, gMinThresh, gMaxThresh));
	}
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

	mRanges[RT_X] = RangeInfo(4, 0, 1.f); // x
	mRanges[RT_Y] = RangeInfo(4, 0, 1.f); // y
	mRanges[RT_DEG] = RangeInfo(4, 0.f, 360.0f); // y
	mRanges[RT_SCALE] = RangeInfo(4, 0.3f, 1.0f); // y
	mRanges[RT_IMAGE] = RangeInfo(4, 0.f, 0.99999f); // y
	mRanges[RT_TRESH] = RangeInfo(4, 0.f, 1.0f); // y
	mRanges[RT_BLOB] = RangeInfo(4, 0.f, 0.99999f); // image
}

void CollageProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& baseImage)
{
	if (mImages.size() == 0)
		return;

	float width = baseImage.width;
	float height = baseImage.height;

	mFbo.begin();
	// start draw
	ofFill();

// 	if (gDebugOn) {
// 		ImageCache& image = mImages[gDebugImage * mImages.size()];
// 		float thresh = ofMap(gDebugThresh, 0, 1, gMinThresh, gMaxThresh);
// 		if (gThreshThresh < fabs(image.getThreshold() - thresh)) {
// 			image.createBlobs(thresh);
// 		}
// 		ofPushMatrix();
// 		float scale = width / image.image.getWidth();
// 		ofScale(scale, scale, scale);
// 		image.cvImgGrayscale.draw(0,0);
// 		for (int ib = 0; ib < image.blobs.size(); ++ib) {
// 			ofSetColor(255);
// 			image.blobs[ib]->line.draw();
// 			image.image.bind();
// 			image.blobs[ib]->mesh.draw(OF_MESH_FILL);
// 			image.image.unbind();
// 		}
// 		ofPopMatrix();
// 	}
//	else {
	{
		ofSetColor(255);
		baseImage.draw(0, 0);
		for (int ir = 0; ir < mRepeat; ++ir) {
			float x = values[ir * RT_MAX + RT_X] * width;
			float y = values [ir * RT_MAX + RT_Y] * height;
			float deg = values[ir * RT_MAX + RT_DEG];
			float scale = values[ir * RT_MAX + RT_SCALE] * (width / 600);
			int iimg = (int)(values[ir * RT_MAX + RT_IMAGE] * (float)mImages.size());
			float thresh = ofMap(values[ir * RT_MAX + RT_TRESH], 0, 1, gMinThresh, gMaxThresh);
			assert(0.3f <= scale && scale <= 2.0f);

			ImageCache& image = mImages[iimg];

			if (image.textures.size() > 0 ) {
				int ib = values[RT_BLOB] * image.textures.size();
				ofImage& img = image.textures[ib];
				scale = scale * width / image.image.getWidth();
				ofPushMatrix();
				ofTranslate(x, y);
				ofRotateZ(deg);
				ofScale(scale, scale, scale);
				img.draw(-img.width / 2, -img.height / 2);
				ofPopMatrix();
			}
		}
	}

	// end draw
	mFbo.end();
	mFbo.readToPixels(pixResult);
}

void CollageProblem::createPixelsFinal(ofPixelsRef pixels, const vector<float>& values, ofImage& baseImage)
{
	createPixels(pixels, values, baseImage);
}
