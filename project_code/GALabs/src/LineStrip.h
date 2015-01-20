#pragma once
class LineStrip
{
public:
	ofPolyline mOrigLine;
	vector<float> mWeight;
	ofTessellator mTess;
	ofPolyline mOutLine;
	ofPolyline mLine;
	ofVboMesh mMesh;
	bool bUpdate;
	ofColor mFillColor;
	ofColor mLineColor;
	float mDefaultWidth;
	float mAngStep;
	float mSmoothingSize;
	float mSmoothingShape;
	float mSpacing;
	float mOutSpacing;
	float mOutSmoothingSize;

	LineStrip() {
		mAngStep = 1.f;
		mDefaultWidth = 50.f;
		mFillColor = ofColor(0, 100, 0);
		mLineColor = ofColor(255, 255, 0);
		bDrawMesh = true;
		bDrawWire = false;
		bDrawLine = true;
		bDrawNormal = true;
		bDrawTangeant = true;
		bDrawPoints = true;
		bDrawOutline = true;
		bDrawLinePoints = true;
		bDrawOrigLine = true;
		mSmoothingSize = 0;
		mSmoothingShape = 0;
		mSpacing = 0;
		mOutSmoothingSize = 0;
		mOutSpacing = 0;
	}

	void clear() {
		mOrigLine.clear();
		bUpdate = true;
	}

	void addVertex(const ofPoint& p) {
		mOrigLine.addVertex(p);
		bUpdate = true;
	}

	ofVec2f getVec(float angle) {
		return ofVec2f(sinf(angle), cosf(angle));
	}

	float getAng(ofVec2f vec) {
		return atan2(vec.x, vec.y);
	}

	void update() {
		//bUpdate = false;
		mOutLine.clear();
		mMesh.clear();

		if (mOrigLine.size() > 0) {
			mLine = mOrigLine;
			if (mSpacing > 0.01f) {
				mLine = mLine.getResampledBySpacing(mSpacing);
			}
			mLine = mLine.getSmoothed(mSmoothingSize * mLine.size(), mSmoothingShape);

			int i = 0;
			float ang1 = getAng(-tangAtIndex(mLine, i));
			float ang2 = PI + ang1 - 0.01f;
			float diff = PI - 0.01f;
			buildCurve(i, weightAtIndex(mLine, i), diff, ang1, ang2, 0.5f);

			for (i = 1; i < mLine.size() - 1; ++i) {
				// buildSide(i, 1);
				buildSimple(i, 1);
			}

			i = mLine.size() - 1;
			ang1 = getAng(tangAtIndex(mLine, i));
			ang2 = PI + ang1 - 0.01f;
			diff = PI - 0.01f;
			buildCurve(i, weightAtIndex(mLine, i), diff, ang1, ang2, 0.5f);

			for (i = mLine.size() - 2; i >= 1; --i) {
				// buildSide(i, -1);
				buildSimple(i, -1);
			}
			i = 0;
			mOutLine.addVertex(mLine[i] + -tangAtIndex(mLine, i) * weightAtIndex(mLine, i));
		}

		mOutLine.close();
		if (mOutSpacing > 0.01f) {
			mOutLine = mOutLine.getResampledBySpacing(mOutSpacing);
		}
		mOutLine = mOutLine.getSmoothed(mOutSmoothingSize * mOutLine.size(), mSmoothingShape);

		mOutLine.simplify();
		mTess.tessellateToMesh(mOutLine, OF_POLY_WINDING_NONZERO, mMesh);
	}

	void buildSide(int i, float sign) {
		float w = weightAtIndex(mLine, i);
		ofVec2f v1 = tangAtIndex(mLine, i - 1) * sign;
		ofVec2f v2 = tangAtIndex(mLine, i) * sign;
		float ang1 = getAng(v1);
		float ang2 = getAng(v2);
		float diff = ofAngleDifferenceRadians(ang2, ang1);
		if (diff > 0) {
			buildCurve(i, w, diff, ang1, ang2, mAngStep);
		}
		else {
			Segment S1;
			Segment S2;
			S1.P0 = mLine[i - 1] + v1 * weightAtIndex(mLine, i - 1);
			S1.P1 = mLine[i] + v1 * weightAtIndex(mLine, i);

			S2.P0 = mLine[i] + v2 * weightAtIndex(mLine, i);
			S2.P1 = mLine[i + 1] + v2 * weightAtIndex(mLine, i + 1);

			ofVec2f I0;
			ofVec2f I1;
			int ret = intersect2D_2Segments(S1, S2, I0, I1);
			if (ret > 0) {
				mOutLine.addVertex(ofVec3f(I0));
			}
			else {
				mOutLine.addVertex(mLine[i] + mLine.getNormalAtIndex(i) * sign * weightAtIndex(mLine, i));
			}
		}
	}

	void buildSimple(int i, float sign) {
		mOutLine.addVertex(mLine[i] + mLine.getNormalAtIndex(i) * sign * weightAtIndex(mLine, i));
	}

	void buildCurve(int i, float w, float diff, float ang1, float ang2, float step) {
		mOutLine.addVertex(mLine[i] + getVec(ang1) * w);
		for (float f = ang1 - step; f > ang1 - diff; f -= step) {
			mOutLine.addVertex(mLine[i] + getVec(f) * w);
		}
		mOutLine.addVertex(mLine[i] + getVec(ang2) * w);
	}

	ofVec3f tangAtIndex(ofPolyline& line, int i) {
		if (i >= line.size() - 1) {
			ofVec3f tang = line.getRightVector().getCrossed(line[i] - line[i - 1]);
			return tang.normalized();
		}
		else {
			ofVec3f tang = line.getRightVector().getCrossed(line[i + 1] - line[i]);
			return tang.normalized();
		}
	}

	float weightAtIndex(ofPolyline& line, int i) {
		if (mWeight.size() == 0) {
			return mDefaultWidth;
		}
		else if (mWeight.size() == 1) {
			return mWeight[0];
		}

		float f = (float)i / (float)line.size() * ((float)mWeight.size() - 1.f);
		int ifv = static_cast<int>(f);
		float ffv = f - (float)ifv;
		if (ffv <= 0) {
			return mWeight[ifv];
		}
		else {
			return ofLerp(mWeight[ifv], mWeight[ifv + 1], ffv);
		}
	}

	bool bDrawMesh;
	bool bDrawWire;
	bool bDrawOrigLine;
	bool bDrawLine;
	bool bDrawLinePoints;
	bool bDrawNormal;
	bool bDrawTangeant;
	bool bDrawPoints;
	bool bDrawOutline;

	void draw() {
		if (bUpdate) {
			update();
		}

		if (bDrawMesh) {
			ofSetColor(mFillColor);
			mMesh.draw();
		}
		if (bDrawOutline) {
			ofSetColor(mLineColor);
			ofSetLineWidth(3.f);
			mOutLine.draw();
		}
		if (bDrawLine) {
			ofSetColor(mLineColor);
			mLine.draw();
		}
		if (bDrawWire) {
			ofSetColor(mLineColor);
			mMesh.drawWireframe();
		}
		if (bDrawPoints) {
			ofSetColor(mLineColor);
			for (int i = 0; i < mOutLine.size(); ++i) {
				ofDrawCircle(mOutLine[i], 4);
				ofDrawBitmapString(ofToString(i), mOutLine[i]);
			}
		}
		if (bDrawOrigLine) {
			ofSetColor(mLineColor);
			mOrigLine.draw();
		}
		if (bDrawLinePoints) {
			ofSetColor(mLineColor);
			for (int i = 0; i < mLine.size(); ++i) {
				ofDrawCircle(mLine[i], 4);
			}
		}
		if (bDrawNormal) {
			ofSetColor(mLineColor);
			for (int i = 0; i < mOrigLine.size(); ++i) {
				ofDrawLine(mOrigLine[i], mOrigLine[i] + mOrigLine.getNormalAtIndex(i) * mDefaultWidth);
				ofDrawLine(mOrigLine[i], mOrigLine[i] + mOrigLine.getNormalAtIndex(i) * -mDefaultWidth);
			}
		}
		if (bDrawTangeant) {
			ofSetColor(mLineColor);
			for (int i = 0; i < mOrigLine.size(); ++i) {
				ofDrawLine(mOrigLine[i], mOrigLine[i] + mOrigLine.getTangentAtIndex(i) * mDefaultWidth);
				ofDrawLine(mOrigLine[i], mOrigLine[i] + mOrigLine.getTangentAtIndex(i) * -mDefaultWidth);
			}
		}
	}
};