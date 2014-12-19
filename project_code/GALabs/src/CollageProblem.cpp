#include "GAProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"
#include "ofxFatLine.h"
#include "ofNode.h"

float gMinSize =  0.1;
float gMaxSize = 0.8;
int gConsider = 250;
float gTolerance = 0.1f;
float gSpacing = 1.f;
float gSmoothing = 1.f;
float gMinThresh = 0;
float gMaxThresh = 250.0f;
float gThreshThresh = 50.0f;
float gThreshSteps = 4.f;
float gMaxScale = 600.f;
float MAX_WIDTH_MULTIPLIER = 100.f;

float gDebugThresh = 500;
float gDebugImage = 0;

ofFloatColor lineColor = ofColor::white;

int capType = 0;
int jointType = 0;
int blendMode = 1;

float globalWidth = 1.f;
bool bFeather = false;

float feathering = 1.f;
bool bTriangulation = false;
bool bFeatherAtCore = false;
bool bFeatherAtCap = false;

bool exportFiles = false;
bool useFatline = true;

ofxFatLine fatline; 
ofPolyline line;

CollageProblem::CollageProblem() : GAProblem() {
	mCompMethod = 7;
	mUseDna = true;
	bFlattenAndSave = true;
	mRepeat = 1;
	mPopSize = 20;
	mNGen = 20;

	gui.addTitle("-- Collage --");
	gui.addSlider("Consider", gConsider, 0.0f, 500.0f);
	gui.addSlider("MinSize", gMinSize, 0.0f, 0.01f);
	gui.addSlider("MaxSize", gMaxSize, 0.0f, 1.0f);
	gui.addSlider("Tolerance", gTolerance, 0.0f, 1.0f);
	gui.addSlider("Spacing", gSpacing, 0.0f, 100.0f);
	gui.addSlider("Smoothing", gSmoothing, 0.0f, 1.0f);

	gui.addSlider("MinThresh", gMinThresh, 0.0f, 1000.0f);
	gui.addSlider("MaxThresh", gMaxThresh, 0.0f, 1000.0f);
	gui.addSlider("ThreshSteps", gThreshSteps, 0.0f, 10.0f);
	gui.addSlider("MaxScale", gMaxScale, 100.0f, 2000.0f);

	gui.addSlider("capType",capType,  0, 3);
	gui.addSlider("jointType", jointType, 0, 2);
	gui.addSlider("blendMode", blendMode, 0, 5);
	gui.addColorPicker("lineColor", lineColor);

	gui.addSlider("globalWidth", globalWidth, 0.f, 10.f);
	gui.addToggle("bFeather", bFeather);

	gui.addSlider("feathering", feathering, 0.f, 10.f);
	gui.addToggle("bTriangulation", bTriangulation);
	gui.addToggle("bFeatherAtCore", bFeatherAtCore);
	gui.addToggle("bFeatherAtCap", bFeatherAtCap);
	gui.addToggle("exportFiles", exportFiles);
	gui.addToggle("useFatline", useFatline);

	gui.addTitle("-- Debug --");
	gui.addSlider("DebugThresh", gDebugThresh, 0.0f, 1.0f);
	gui.addSlider("DebugImage", gDebugImage, 0.0f, 0.99999f);
}

void CollageProblem::setup() {
	GAProblem::setup();

	ofDirectory dir(rootDir + "source/");
	dir.allowExt("jpg");
	int nd = dir.listDir();

	mImages.resize(nd);
	for (int i = 0; i < nd; ++i) {
		mImages[i].loadImage(dir.getPath(i), mWorkingWidth * MAX_WIDTH_MULTIPLIER);
	}
}

bool blobSort(BlobInfo* b1, BlobInfo* b2) 
{ 
	return (b1->texture.getWidth() * b1->texture.getHeight() < b2->texture.getWidth() * b2->texture.getHeight()); 
}

void ImageCache::createBlobCvGray(ofxCvGrayscaleImage& cvImg) {
	ofTessellator tess;
	ofxCvContourFinder contourFinder;

	contourFinder.findContours(cvImg, cvImg.getWidth() * cvImg.getHeight() * gMinSize, cvImg.getWidth() * cvImg.getHeight() * gMaxSize, gConsider, false, true);

	for(int i = 0; i < contourFinder.blobs.size(); i++) {
		ofRectangle& rect = contourFinder.blobs[i].boundingRect;

		if (rect.getLeft() < 2 || 
			image.width - rect.getRight() < 2 ||
			rect.getTop() < 2 ||
			image.height - rect.getBottom() < 2) {

		}
		else {
			BlobInfo* info = new BlobInfo();
			info->centroid = contourFinder.blobs[i].centroid;
			info->line.addVertices(contourFinder.blobs[i].pts);
			info->line.setClosed(true);
			info->line.simplify(gTolerance);
			info->line = info->line.getSmoothed(gSpacing, gSpacing);

//			info->line.getResampledBySpacing(gSpacing);

			tess.tessellateToMesh(info->line, OF_POLY_WINDING_ODD, info->mesh, true);

			vector<ofVec3f>& verts = info->mesh.getVertices();
			for (int i = 0; i < verts.size(); ++i) {
				info->mesh.addTexCoord(ofVec2f(verts[i].x, verts[i].y));
			}
			info->mesh.disableColors();
			info->mesh.disableNormals();

			ofFbo fbo;
			ofPixels pixResult;
			ofImage workingImage;
			ofFbo::Settings sett;
			sett.width = rect.width;
			sett.height = rect.height;
			fbo.allocate(sett);

			fbo.begin();
			ofClear(255, 255, 255, 0);
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			ofSetColor(255, 255, 255);
			ofPushMatrix();
			ofTranslate(-rect.x, -rect.y);
			image.bind();
			info->mesh.draw();
			image.unbind();

			if (useFatline) {
				ofEnableBlendMode((ofBlendMode)blendMode);
				ofSetColor(lineColor);
				ofxFatLine fatline;
				fatline.setCapType((ofxFatLineCapType)jointType);
				fatline.setJointType((ofxFatLineJointType)capType);
				fatline.setGlobalWidth(globalWidth);
				fatline.setGlobalColor(lineColor);
				fatline.enableFeathering(bFeather);
				fatline.setFeather(0);
				fatline.enableTriangulation(bTriangulation);
				fatline.enableFeatherAtCore(bFeatherAtCore);
				fatline.enableFeatherAtCap(bFeatherAtCap);

				fatline.setFromPolyline(info->line);
				fatline.draw();
				fatline.setGlobalWidth(globalWidth * 2.f);
				fatline.draw();
				fatline.setGlobalWidth(globalWidth * 3.f);
				fatline.draw();
			}
			ofPopMatrix();

			fbo.end();

			pixResult.clear();
			fbo.readToPixels(pixResult);
			workingImage.setFromPixels(pixResult);

			static int index = 0;
			if (exportFiles) {
				ofSaveImage(workingImage.getPixels(), "test/good_" + name + ofToString(index++) + " .png", OF_IMAGE_QUALITY_BEST);
			}
			info->texture = workingImage;
			blobs.push_back(info);
		}

		printf("%d / %d\n", i + 1, contourFinder.blobs.size());
	}
	sort(blobs.begin(), blobs.end(), blobSort);
}

void ImageCache::deleteBlobs() {
	for (int i = 0; i < blobs.size(); ++i) {
		delete blobs[i];
	}
	blobs.clear();
}

void ImageCache::addBlobs(float threshold) {

	ofxCvColorImage cvImgColor;
	cvImgColor.allocate(image.getWidth(), image.getHeight());
	cvImgColor.setFromPixels(image.getPixelsRef());

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

void ImageCache::loadImage(const string& filename, float maxWidth) {
	name = ofFilePath::getBaseName(filename);
	image.loadImage(filename);

	if (image.getWidth() > maxWidth) {
		float diff = image.getWidth() / image.getHeight();
		image.resize(maxWidth, maxWidth / diff);
	}

	deleteBlobs();
	for (float f = 0; f < 1.f; f += (1.f / gThreshSteps)) {
		addBlobs(ofMap(f, 0.f, 1.f, gMinThresh, gMaxThresh));
	}
}

enum RangeType {
	RT_X,
	RT_Y,
	RT_DEG,
	RT_SCALE,
	RT_IMAGE,
	RT_BLOB,
	RT_MAX
};

void CollageProblem::setRanges() {
	mRanges.resize(RT_MAX);

	mRanges[RT_X] = RangeInfo(4, 0, 1.f); // x
	mRanges[RT_Y] = RangeInfo(4, 0, 1.f); // y
	mRanges[RT_DEG] = RangeInfo(4, 0.f, 360.0f); // y
	mRanges[RT_SCALE] = RangeInfo(4, 0.3f, 1.0f); // y
	mRanges[RT_IMAGE] = RangeInfo(4, 0.f, 0.99999f); // y
	mRanges[RT_BLOB] = RangeInfo(4, 0.f, 0.99999f); // image
}

void CollageProblem::createPixels(ofPixelsRef pixResult, const vector<float>& values, ofImage& baseImage, int width, int height) {
	if (mImages.size() == 0)
		return;

	pixResult.clear();

	ofFbo::Settings setts;
	setts.width = width;
	setts.height = height;
	setts.internalformat = GL_RGB;

	ofFbo mFbo;
	mFbo.allocate(setts);
	mFbo.begin();
	ofClear(255,255,255,255);

	ofSetColor(255);
	ofBlendMode(OF_BLENDMODE_DISABLED);
	baseImage.draw(0, 0);

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	for (int ir = 0; ir < mRepeat; ++ir) {
		float x = values[ir * RT_MAX + RT_X] * width;
		float y = values [ir * RT_MAX + RT_Y] * height;
		float deg = values[ir * RT_MAX + RT_DEG];
		float scale = values[ir * RT_MAX + RT_SCALE] * (width / gMaxScale);
		int iimg = (int)(values[ir * RT_MAX + RT_IMAGE] * (float)mImages.size());

		ImageCache& imgCache = mImages[iimg];

		if (imgCache.blobs.size() > 0 ) {
			int ib = values[ir * RT_MAX + RT_BLOB] * imgCache.blobs.size();
			ofImage& tex = imgCache.blobs[ib]->texture;
			ofPushMatrix();
			ofTranslate(x, y);
			ofRotateZ(deg);
			ofScale(scale, scale, scale);
			tex.draw(-tex.width / 2, -tex.height / 2);
			ofPopMatrix();
		}
	}

	// end draw
	mFbo.end();
	mFbo.readToPixels(pixResult);
}

void CollageProblem::debugDraw() {
	ImageCache& imgCache = mImages[gDebugImage * mImages.size()];

	static float lastTresh = 0;
	if (gDebugThresh != lastTresh) {
		imgCache.deleteBlobs();
		imgCache.addBlobs(ofMap(gDebugThresh, 0.f, 1.f, gMinThresh, gMaxThresh));
		lastTresh = gDebugThresh;

		for (int i = 0; i < 10; ++i) {
			line.addVertex(ofVec3f(ofRandom(10, 200), ofRandom(10, 200), 0));
		}
	}

	ofEnableBlendMode((ofBlendMode)blendMode);
	ofSetColor(lineColor);

	fatline.clear();
	fatline.setCapType((ofxFatLineCapType)jointType);
	fatline.setJointType((ofxFatLineJointType)capType);
	fatline.setGlobalWidth(globalWidth);
	fatline.setGlobalColor(lineColor);
	fatline.enableFeathering(bFeather);

	fatline.setFeather(feathering);
	fatline.enableTriangulation(bTriangulation);
	fatline.enableFeatherAtCore(bFeatherAtCore);
	fatline.enableFeatherAtCap(bFeatherAtCap);

	fatline.setFromPolyline(line);
	fatline.draw();

	float scale = 1;
	float x = 100;
	float y = 100;
	float deg = 0;
	float blob = 0;

	int xs = (int)sqrt(imgCache.blobs.size());

	for (int i = 0; i < imgCache.blobs.size(); ++i) {
		ofImage& tex = imgCache.blobs[i]->texture;

		x += tex.getWidth();
		if (x > 1000) {
			x = 0;
			y += 200;
		}

		//		scale = scale * width / imgCache.image.getWidth();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255, 255, 255);
		ofPushMatrix();
		ofTranslate(x, y);
		ofRotateZ(deg);
		ofScale(scale, scale, scale);
		ofTranslate(-tex.width / 2, -tex.height / 2);
		tex.draw(0, 0);

		ofEnableBlendMode((ofBlendMode)blendMode);
		ofSetColor(lineColor);
		ofPopMatrix();
	}
}
