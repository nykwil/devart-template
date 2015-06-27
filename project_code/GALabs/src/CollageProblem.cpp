#include "TestProblem.h"
#include <assert.h>
#include "ColorConvert.h"
#include "ofxSimpleGuiToo.h"

float gMinSize = 0.0005f;
float gMaxSize = 0.98f;
int gConsider = 250;
float gSpacing = 0.5f;
float gSmoothing = 0.03f;
float gMinThresh = 90;
float gMaxThresh = 250.0f;
float gThreshThresh = 50.0f;
float gThreshSteps = 4.f;
float gMaxScale = 600.f;
float MAX_WIDTH_MULTIPLIER = 100.f;

float gDebugThresh = 500;
float gDebugImage = 0;
bool gDebugRefresh = false;

ofFloatColor mLineColor = ofColor(0, 0, 0, 128);

int mBlendMode = 3;

float mLineWidth = 1.5f;

bool mExportFiles = false;
bool mUseFatline = false;

CollageProblem::CollageProblem() : GAProblem() {
	mCompMethod = 7;
	mUseDna = true;
	bFlattenAndSave = true;
	mRepeat = 1;
	mPopSize = 20;
	mNGen = 20;

	gui.addPage("Collage");
	gui.addSlider("Consider", gConsider, 0.0f, 500.0f);
	gui.addSlider("MinSize", gMinSize, 0.0f, 0.01f);
	gui.addSlider("MaxSize", gMaxSize, 0.0f, 1.0f);
	gui.addSlider("Spacing", gSpacing, 0.0f, 10.0f);
	gui.addSlider("Smoothing", gSmoothing, 0.0f, 1.0f);

	gui.addSlider("MinThresh", gMinThresh, 0.0f, 1000.0f);
	gui.addSlider("MaxThresh", gMaxThresh, 0.0f, 1000.0f);
	gui.addSlider("ThreshSteps", gThreshSteps, 0.0f, 10.0f);
	gui.addSlider("MaxScale", gMaxScale, 100.0f, 2000.0f);

	gui.addColorPicker("LineColor", mLineColor);
	gui.addSlider("LinelWidth", mLineWidth, 0.f, 10.f);
	gui.addToggle("UseFatline", mUseFatline);
	gui.addSlider("BlendMode", mBlendMode, 0, 5);

	gui.addToggle("ExportFiles", mExportFiles);

	gui.addTitle("-- Debug --");
	gui.addSlider("DebugThresh", gDebugThresh, 0.0f, 1.0f);
	gui.addSlider("DebugImage", gDebugImage, 0.0f, 0.99999f);
	gui.addButton("DebugRefresh", gDebugRefresh);
}

void CollageProblem::setup() {
	GAProblem::setup();

	ofDirectory dir(mRootDir + "/source/");
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

		bool onEdge = rect.getLeft() < 2 || 
			image.getWidth() - rect.getRight() < 2 ||
			rect.getTop() < 2 ||
			image.getHeight() - rect.getBottom() < 2;

		if (mExportFiles || !onEdge) {
			ofPolyline line;
			line.addVertices(contourFinder.blobs[i].pts);
			line.setClosed(true);
			if (gSpacing > 0) { 			
				line = line.getResampledBySpacing(gSpacing);
				line = line.getSmoothed((gSmoothing * gSmoothing) * line.size(), 1.f);
			}
 			line.simplify();

			ofVboMesh mesh;
			tess.tessellateToMesh(line, OF_POLY_WINDING_ODD, mesh, true);

			vector<ofVec3f>& verts = mesh.getVertices();
			for (int iv = 0; iv < verts.size(); ++iv) {
				mesh.addTexCoord(ofVec2f(verts[iv].x, verts[iv].y));
			}
			mesh.disableColors();
			mesh.disableNormals();

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
			mesh.draw();
			image.unbind();

			if (mLineWidth > 0) {
				ofEnableBlendMode((ofBlendMode)mBlendMode);
				ofSetColor(mLineColor);
				ofSetLineWidth(mLineWidth);
				ofEnableSmoothing();
				ofEnableAntiAliasing();
				line.draw();
				if (mUseFatline) {
					ofSetLineWidth(mLineWidth * 2);
					line.draw();
					ofSetLineWidth(mLineWidth * 3);
					line.draw();
				}
			}
			ofPopMatrix();

			fbo.end();

			pixResult.clear();
			fbo.readToPixels(pixResult);
			workingImage.setFromPixels(pixResult);

			static int index = 0;
			if (mExportFiles) {
				if (onEdge) {
					ofSaveImage(workingImage.getPixels(), "debug/bad_" + name + ofToString(index++) + " .png", OF_IMAGE_QUALITY_BEST);
				}
				else {
					ofSaveImage(workingImage.getPixels(), "debug/good_" + name + ofToString(index++) + " .png", OF_IMAGE_QUALITY_BEST);
				}
			}
			if (!onEdge) {
				BlobInfo* info = new BlobInfo();
				info->line = line;
				info->centroid = contourFinder.blobs[i].centroid;
				info->mesh = mesh;
				info->texture = workingImage;
				blobs.push_back(info);
			}
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

	ofFbo fbo;
	fbo.allocate(setts);
	fbo.begin();
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
			tex.draw(-tex.getWidth() / 2, -tex.getHeight() / 2);
			ofPopMatrix();
		}
	}

	// end draw
	fbo.end();
	fbo.readToPixels(pixResult);
}

void CollageProblem::debugDraw() {
	ImageCache& imgCache = mImages[gDebugImage * mImages.size()];

	static float lastTresh = 0;
	if (gDebugRefresh || gDebugThresh != lastTresh) {
		imgCache.deleteBlobs();
		imgCache.addBlobs(ofMap(gDebugThresh, 0.f, 1.f, gMinThresh, gMaxThresh));
		lastTresh = gDebugThresh;
	}

	float scale = 1;
	float x = 100;
	float y = 100;
	float deg = 0;
	float blob = 0;

	int xs = (int)sqrt(imgCache.blobs.size());

	for (int i = 0; i < imgCache.blobs.size(); ++i) {
		ofImage& tex = imgCache.blobs[i]->texture;

		//		scale = scale * width / imgCache.image.getWidth();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255, 255, 255);
		ofPushMatrix();
		ofTranslate(x, y);
		ofRotateZ(deg);
		ofScale(scale, scale, scale);
		ofTranslate(-tex.getWidth() / 2, -tex.getHeight() / 2);
		tex.draw(0, 0);
		ofPopMatrix();

		x += tex.getWidth();
		if (x > 1000) {
			x = 0;
			y += 200;
		}
	}
}
