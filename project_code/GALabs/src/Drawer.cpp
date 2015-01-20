#include "Drawer.h"
#include <assert.h>

float Drawer::UNITS_PER_POINT = 4.f;
float Drawer::CURVENESS = 1.2f;

ofVec2f ofLerp(const ofVec2f& a, const ofVec2f& b, float amt) {
    return ofVec2f(ofLerp(a.x, b.x, amt), ofLerp(a.y, b.y, amt));
}

vector<float> gIntensityCurve;

void buildIntensityCurve() {
    gIntensityCurve.push_back(0);
    gIntensityCurve.push_back(0.2);
    gIntensityCurve.push_back(0.5);
    gIntensityCurve.push_back(0.7);
    gIntensityCurve.push_back(1);
    gIntensityCurve.push_back(0.9);
    gIntensityCurve.push_back(0.7);
    gIntensityCurve.push_back(0.6);
    gIntensityCurve.push_back(0.5);
    gIntensityCurve.push_back(0.4);
    gIntensityCurve.push_back(0.3);
    gIntensityCurve.push_back(0.2);
    gIntensityCurve.push_back(0.1);
    gIntensityCurve.push_back(0);
}

float getIntensity(float amt) {
    float f = amt * (gIntensityCurve.size() - 1);

    int i = (int)f;
    float delta = f - (float)i;

    return ofLerp(gIntensityCurve[i], gIntensityCurve[i + 1], delta);
}

Drawer::Drawer() {
    buildIntensityCurve();
}

ofColor Drawer::rotatedPixel(ofImage& brush, ofVec2f pos, float size, float angle) {
    assert(brush.getWidth() == brush.getHeight());

    pos /= size;

    pos.x -= 0.5f;
    pos.y -= 0.5f;

    // when pos.x == size

    float cs = cosf(angle);
    float sn = sinf(angle);

    ofVec2f pr(pos.x * cs - pos.y * sn, pos.x * sn + pos.y * cs);

    pr.x += 0.5f;
    pr.y += 0.5f;
    pr *= brush.getWidth();

    if (0 <= pr.x && pr.x < brush.getWidth() && 0 <= pr.y && pr.y < brush.getHeight())
        return brush.getColor(pr.x, pr.y);
    else
        return ofColor(0,0,0,0);
}

void Drawer::renderBrush(ofPixels& pixDest, const ofVec2f& pos, ofImage& imgBrush, float angle, float size, const ofFloatColor& colMult)
{
    float hs = size / 2.f;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int px = pos.x - hs + x;
            int py = pos.y - hs + y;
            if (0 <= px && px < pixDest.getWidth() && 0 <= py && py < pixDest.getHeight())
            {
                ofColor colImage = pixDest.getColor(px, py);
                ofColor colBrush = rotatedPixel(imgBrush, ofVec2f(x, y), size, angle);
                ofColor colNew;
                if (colBrush.a != 0)
                {
                    float cbA = colBrush.a / 255.f;

                    colNew.r = ofLerp(colImage.r, colBrush.r * colMult.r, cbA);
                    colNew.g = ofLerp(colImage.g, colBrush.g * colMult.g, cbA);
                    colNew.b = ofLerp(colImage.b, colBrush.b * colMult.b, cbA);
                    colNew.a = 255; // colImage.a;
                }
                else {
                    colNew = colImage;
                }

                pixDest.setColor(px, py, colNew);
            }
        }
    }
}

void Drawer::stroke(ofPixels& pixDest, ofImage& imgBrush, const ofFloatColor& colMult, float brushSize, float ang, const ofVec2f& startPos, const ofVec2f& endPos) {
    float dist = startPos.distance(endPos) * CURVENESS;

    ofPolyline line;

    line.addVertex(startPos);
    ofVec2f dir = (endPos - startPos).getNormalized();
    ofPoint p = dir.getRotated(ang) * dist;

    line.bezierTo(p.x, p.y, 0, p.x, p.y, 0, endPos.x, endPos.y, 0);

    stroke(pixDest, imgBrush, colMult, brushSize, line);
}

void Drawer::stroke(ofPixels& pixDest, ofImage& imgBrush, const ofFloatColor& colMult, float brushSize, const ofPolyline& iline) {
//    line.getSmoothed(0,0);
    ofPolyline line = iline.getResampledBySpacing((int)UNITS_PER_POINT);

    for (int i = 0; i < (int)line.size() - 1; ++i) {
        ofVec2f pos = line[i];
        ofVec2f np = line[i + 1];

        ofVec2f dir = (np - pos);

        float ang = atan2f(dir.y, dir.x);
        float size = getIntensity((float)i / (float)line.size()) * brushSize;

        renderBrush(pixDest, pos, imgBrush, ang, size, colMult);
    }
}
