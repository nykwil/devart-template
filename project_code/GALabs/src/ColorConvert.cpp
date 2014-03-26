#include "ColorConvert.h"
#include "core.hpp"

float refX = 31271.f / 32902.f;
float refY = 1.f;
float refZ = 35827.f / 32902.f;

struct ColorMatrix
{
    float rScale;
    float gScale;
    float bScale;
};

ColorMatrix yuvmatrix = { 0.299f, 0.587f, 0.114f };

/*
//    'bt601':
// name: 'ITU-R BT.601 (DVD, JPEG, Youtube)',
float rScale = 0.299;
float gScale = 0.587;
float bScale = 0.114;

// 'bt709':
// name: 'ITU-R BT.709 (HDTV)',
float rScale = 0.2125;
float gScale = 0.7154;
float bScale = 0.0721;

// 'smpte240m':
// name: 'SMPTE 240M (very old HDTV)',
float rScale = 0.212;
float gScale = 0.701;
float bScale = 0.087;

// 'fcc':
// name: 'FCC',
float rScale = 0.3;
float gScale = 0.59;
float bScale = 0.11;
*/

inline float clamp(float x, float mn, float mx)
{
    return min(max(x, mn), mx);
};

float clamphue(float hue)
{
	hue = fmod(hue, (float)PI * 2.f);

	if(hue < 0)
	{
		hue += PI * 2.f;
	}

	return hue;
};

float colmin(float r, float g, float b)
{
    return min(r, min(g, b));
}

float colmax(float r, float g, float b)
{
    return max(r, max(g, b));
}

static float maxDelta = 105;

float ColorCompare::getMaxDelta()
{
    return maxDelta;
}

// CIE Delta E 1976
// JND: ~2.3
float ColorCompare::deltaE1976(const ColorLab& lab1, const ColorLab& lab2)
{
	float delta_L = lab1.L - lab2.L;
	float delta_a = lab1.a - lab2.a;
	float delta_b = lab1.b - lab2.b;

    float delta = sqrt(delta_L * delta_L + delta_a * delta_a + delta_b * delta_b);
    maxDelta = max(maxDelta, delta);
	return delta;
}

// CIE Delta E 1994
float ColorCompare::deltaE1994(const ColorLab& lab1, const ColorLab& lab2, int type)
{
	float C1 = sqrt(lab1.a * lab1.a + lab1.b * lab1.b);
	float C2 = sqrt(lab2.a * lab2.a + lab2.b * lab2.b);

	float delta_L = lab1.L - lab2.L;
	float delta_C = C1 - C2;
	float delta_a = lab1.a - lab2.a;
	float delta_b = lab1.b - lab2.b;
	float delta_H = sqrt(delta_a * delta_a + delta_b * delta_b - delta_C * delta_C);

	if (type == 0)
	{
		delta_C /= C1 * 0.045f + 1.f;
		delta_H /= C1 * 0.015f + 1.f;
	}
	else 
	{
		delta_L *= 0.5f;
		delta_C /= C1 * 0.048f + 1.f;
		delta_H /= C1 * 0.014f + 1.f;
	}

	return sqrt(delta_L * delta_L + delta_C * delta_C + delta_H * delta_H);
}

// CIE Delta E 2000
// Note: maximum is about 158 for colors in the sRGB gamut.
float ColorCompare::deltaE2000(const ColorLChab& lch1, const ColorLChab& lch2)
{
	float avg_L = (lch1.L + lch2.L) * 0.5f;
	float delta_L = lch2.L - lch1.L;

	float avg_C = (lch1.C + lch2.C) * 0.5f;
	float delta_C = lch1.C - lch2.C;

	float avg_H = (lch1.h + lch2.h) * 0.5f;

	if(fabs(lch1.h - lch2.h) > PI)
	{
		avg_H += PI;
	}

	float delta_H = lch2.h - lch1.h;

	if(fabs(delta_H) > PI)
	{
		if(lch2.h <= lch1.h) 
            delta_H += PI * 2.f;
		else delta_H -= PI * 2.f;
	}

	delta_H = sqrt(lch1.C * lch2.C) * sin(delta_H) * 2.f;

	float T = 1
		- 0.17f * cos(avg_H - PI / 6.f)
		+ 0.24f * cos(avg_H * 2.f)
		+ 0.32f * cos(avg_H * 3.f + PI / 30.f)
		- 0.20f * cos(avg_H * 4.f - PI * 7.f / 20.f);

	float SL = avg_L - 50.f;
	SL *= SL;
	SL = SL * 0.015f / sqrt(SL + 20.f) + 1.f;

	float SC = avg_C * 0.045f + 1.f;

	float SH = avg_C * T * 0.015f + 1.f;

	float delta_Theta = avg_H / 25.f - PI * 11.f / 180.f;
	delta_Theta = exp(delta_Theta * -delta_Theta) * (PI / 6.f);

	float RT = pow(avg_C, 7.f);
	RT = sqrt(RT / (RT + 6103515625.f)) * sin(delta_Theta) * -2.f; // 6103515625 = 25^7

	delta_L /= SL;
	delta_C /= SC;
	delta_H /= SH;

	return sqrt(delta_L * delta_L + delta_C * delta_C + delta_H * delta_H + RT * delta_C * delta_H);
}

float ColorCompare::HsbDiff(const ofColor& c1, const ofColor& c2)
{
    ofVec3f h1;
    ofVec3f h2;

    c1.getHsb(h1.x, h1.y, h1.z);
    c2.getHsb(h2.x, h2.y, h2.z);

    static float hw = 0.33f;
    static float sw = 0.33f;
    static float bw = 0.33f;

    return 1.f - (ofAngleDifferenceDegrees(h1.x / 255.f * 360.f, h2.x / 255.f * 360.f) / 180.f * hw + 
        fabs(h1.y - h2.y) / 255.f * sw + 
        fabs(h1.z - h2.z) / 255.f * bw);
}

ColorRGB::ColorRGB( float R, float G, float B, bool clamped )
{
    this->clamped = clamped || R < 0 || R > 1 || G < 0 || G > 1 || B < 0 || B > 1;
    this->R = clamp(R, 0, 1);
    this->G = clamp(G, 0, 1);
    this->B = clamp(B, 0, 1);
}

ColorLinearRGB ColorRGB::toLinearRGB()
{
    return ColorLinearRGB(toLinearRGBc(this->R), toLinearRGBc(this->G), toLinearRGBc(this->B), this->clamped);
}

ColorHSV ColorRGB::toHSV()
{
    float mn = colmin(this->R, this->G, this->B);
    float mx = colmax(this->R, this->G, this->B);
    float delta = mx - mn;
    float H = 0;
    float S = 0;
    if(fabs(delta) != 0)
    {
        float S = delta / mx;

        if(mx == this->R) 
            H = (this->G - this->B) / delta;
        else if(mx == this->G) 
            H = (this->B - this->R) / delta + 2.f;
        else 
            H = (this->R - this->G) / delta + 4.f;
    }

    return ColorHSV(H * 60.f, S, mx, this->clamped);
}

ColorHSL ColorRGB::toHSL()
{
    float mn = colmin(this->R, this->G, this->B);
    float mx = colmax(this->R, this->G, this->B);
    float delta = mx - mn;

    float L = (mx + mn) * 0.5f;

    float S = 0;
    float H = 0;

    if(fabs(delta) != 0)
    {
        if(L < 0.5f) 
            S = delta / (mx + mn);
        else 
            S = delta / (2 - mx - mn);

        if(mx == this->R) 
            H = (this->G - this->B) / delta;
        else if(mx == this->G)
            H = (this->B - this->R) / delta + 2.f;
        else
            H = (this->R - this->G) / delta + 4.f;
    }

    return ColorHSL(H * 60, S, L, this->clamped);
}

ColorYUV ColorRGB::toYUV()
{
    auto mat = yuvmatrix;

    float y = this->R * mat.rScale + this->G * mat.gScale + this->B * mat.bScale;
    float u = (this->B - y) / (1.f - mat.bScale) * 0.5f + 0.5f;
    float v = (this->R - y) / (1.f - mat.rScale) * 0.5f + 0.5f;

    return ColorYUV(y, u, v, this->clamped);
}

ColorYIQ ColorRGB::toYIQ()
{
    return ColorYIQ(
        this->R *  0.299f                + this->G *  0.587f               + this->B *  0.114f,
        this->R *  0.5f                  + this->G * -0.23038159508364756f + this->B * -0.26961840491635244f  + 0.5f,
        this->R * -0.202349432337541121f + this->G *  0.5f                 + this->B * -0.297650567662458879f + 0.5f,
        this->clamped);
}

ColorLinearRGB::ColorLinearRGB( float R, float G, float B, bool clamped )
{
    this->clamped = clamped || R < 0 || R > 1 || G < 0 || G > 1 || B < 0 || B > 1;
    this->R = clamp(R, 0, 1);
    this->G = clamp(G, 0, 1);
    this->B = clamp(B, 0, 1);
}

ColorRGB ColorLinearRGB::toRGB()
{
    return ColorRGB(toRGBc(this->R), toRGBc(this->G), toRGBc(this->B), this->clamped);
}

ColorXYZ ColorLinearRGB::toXYZ()
{
    return ColorXYZ(
        this->R * (5067776.f / 12288897.f) + this->G * (4394405.f / 12288897.f) + this->B * (4435075.f / 24577794.f),
        this->R * (871024.f / 4096299.f)   + this->G * (8788810.f / 12288897.f) + this->B * (887015.f / 12288897.f),
        this->R * (79184.f / 4096299.f)    + this->G * (4394405.f / 36866691.f) + this->B * (70074185.f / 73733382.f),
        this->clamped);
}

ColorXYZ::ColorXYZ( float X, float Y, float Z, bool clamped )
{
    this->clamped = clamped || X < 0 || X > refX || Y < 0 || Y > refY || Z < 0 || Z > refZ;

    this->X = clamp(X, 0, refX);
    this->Y = clamp(Y, 0, refY);
    this->Z = clamp(Z, 0, refZ);
}

ColorLinearRGB ColorXYZ::toLinearRGB()
{
    return ColorLinearRGB(
        this->X * (641589.f / 197960.f)      + this->Y * (-608687.f / 395920.f)    + this->Z * (-49353.f / 98980.f),
        this->X * (-42591639.f / 43944050.f) + this->Y * (82435961.f / 43944050.f) + this->Z * (1826061.f / 43944050.f),
        this->X * (49353.f / 887015.f)       + this->Y * (-180961.f / 887015.f)    + this->Z * (49353.f / 46685.f),
        this->clamped);
}

ColorxyY ColorXYZ::toxyY()
{
    float div = this->X + this->Y + this->Z;

    if(fabs(div) == 0)
    {
        div = 1.f;
    }

    return ColorxyY(this->X / div, this->Y / div, this->Y, this->clamped);
}

float ColorXYZ::toLabc( float c )
{
    if (c > 216.f / 24389.f) 
        return cv::cubeRoot(c);
    return c * (841.f / 108.f) + (4.f / 49.f);
}

ColorLab ColorXYZ::toLab()
{
    float X = toLabc(this->X / refX);
    float Y = toLabc(this->Y / refY);
    float Z = toLabc(this->Z / refZ);

    return ColorLab(116.f * Y - 16.f, 500.f * (X - Y), 200.f * (Y - Z), this->clamped);
}

ColorLuv ColorXYZ::toLuv()
{
    float rdiv = refX + refY * 15.f + refZ * 3.f;
    float ur = refX * 4.f / rdiv;
    float vr = refY * 9.f / rdiv;

    float div = this->X + this->Y * 15.f + this->Z * 3.f;

    if(fabs(div) == 0)
    {
        div = 1.f;
    }

    float u = this->X * 4.f / div;
    float v = this->Y * 9.f / div;
    float yr = this->Y / refY;
    float L;

    if(yr > 216.f / 24389.f) {
        L = cv::cubeRoot(yr) * 116.f - 16.f;
    }
    else
    {
        L = yr * (24389.f / 27.f);
    }

    return ColorLuv(L, L * 13.f * (u - ur), L * 13.f * (v - vr), this->clamped);
}

ColorLChab::ColorLChab(float L, float C, float h, bool clamped)
{
    this->clamped = clamped || L < 0 || L > 100 || C < 0 || C > 4.64238345442629658e2f;

    this->L = clamp(L, 0, 100.f);
    this->C = clamp(C, 0, 4.64238345442629658e2f); // 2500*sqrt(1/29)
    this->h = clamphue(h);
}

ColorLab ColorLChab::toLab()
{
    float h = clamphue(this->h);
    return ColorLab(this->L, cos(h) * this->C, sin(h) * this->C, this->clamped);
}

ColorLShuv::ColorLShuv( float L, float S, float h, bool clamped )
{
    this->clamped = clamped || L < 0 || L > 100.f; // TODO: what is S min/max?

    this->L = clamp(L, 0.f, 100.f);
    this->S = S;
    this->h = clamphue(h);
}

ColorLChuv ColorLShuv::toLChuv()
{
    return ColorLChuv(this->L, this->S * this->L, this->h, this->clamped);
}

ColorLChuv::ColorLChuv( float L, float C, float h, bool clamped )
{
    this->clamped = clamped || L < 0 || L > 100.f || C < 0 || C > 7.40066582332174237e2f;

    this->L = clamp(L, 0.f, 100.f);
    this->C = clamp(C, 0.f, 7.40066582332174237e2f); // 240/316141*sqrt(950343809713)
    this->h = clamphue(h);
}

ColorLShuv ColorLChuv::toLShuv()
{
    return ColorLShuv(this->L, this->C / this->L, this->h, this->clamped);
}

ColorLuv ColorLChuv::toLuv()
{
    float h = clamphue(this->h);
    return ColorLuv(this->L, cos(h) * this->C, sin(h) * this->C, this->clamped);
}

ColorLuv::ColorLuv( float L, float u, float v, bool clamped )
{
    this->clamped = clamped || L < 0 || L > 100 || u < -81304600.f / 316141.f || v > 54113280.f / 316141.f; // TODO: what is u max, and v min??

    this->L = clamp(L, 0, 100.f);
    this->u = (u < -81304600.f / 316141.f) ? -81304600.f / 316141.f : u;
    this->v = (v > 54113280.f / 316141.f) ? 54113280.f / 316141.f : v;
}

ColorLChuv ColorLuv::toLChuv()
{
    return ColorLChuv(
        this->L,
        sqrt(this->u * this->u + this->v * this->v),
        atan2(this->v, this->u),
        this->clamped);
}

ColorXYZ ColorLuv::toXYZ()
{
    float rdiv = refX + refY * 15.f + refZ * 3.f;
    float ur = refX * 4.f / rdiv;
    float vr = refY * 9.f / rdiv;
    float Y;

    if(L > 8.f)
    {
        Y = (this->L + 16.f) / 116.f;
        Y = Y * Y * Y;
    }
    else
    {
        Y = this->L * (27.f / 24389.f);
    }

    float a = (this->L * 52.f / (this->u + this->L * 13.f * ur) - 1.f) / 3.f;
    float b = -5.f * Y;
    float d = (this->L * 39.f / (this->v + this->L * 13.f * vr) - 5.f) * Y;

    float X = (d - b) / (a + 1.f / 3.f);
    float Z = X * a + b;

    return ColorXYZ(X, Y, Z, this->clamped);
}

ColorLab::ColorLab( float L, float a, float b, bool clamped )
{
    this->clamped = clamped || L < 0.f || L > 100.f || a < -12500.f / 29.f || a > 12500.f / 29.f || b < -5000.f / 29.f || b > 5000.f / 29.f;

    this->L = clamp(L, 0.f, 100.f);
    this->a = clamp(a, -12500.f / 29.f, 12500.f / 29.f);
    this->b = clamp(b, -5000.f / 29.f, 5000.f / 29.f);
}

float ColorLab::toXYZc( float c )
{
    float c3 = c * c * c;

    if(c3 > 216.f / 24389.f) 
        return c3;

    return c * (108.f / 841.f) - (432.f / 24389.f);
}

ColorXYZ ColorLab::toXYZ()
{
    float Y = (this->L + 16.f) / 116.f;

    return ColorXYZ(
        toXYZc(Y + this->a / 500.f) * refX,
        toXYZc(Y) * refY,
        toXYZc(Y - this->b / 200.f) * refZ,
        this->clamped);
}

ColorLChab ColorLab::toLChab()
{
    return ColorLChab(
        this->L,
        sqrt(this->a * this->a + this->b * this->b),
        atan2(this->b, this->a),
        this->clamped);
}

ColorxyY::ColorxyY( float x, float y, float Y, bool clamped )
{
    this->clamped = clamped || x < 0 || x > 1 || y < 0 || y > 1 || Y < 0 || Y > 1;

    this->x = clamp(x, 0.f, 1.f);
    this->y = clamp(y, 0.f, 1.f);
    this->Y = clamp(Y, 0.f, 1.f);
}

ColorXYZ ColorxyY::toXYZ()
{
    if(abs(this->y) != 0)
    {
        float mul = this->Y / this->y;
        return ColorXYZ(this->x * mul, this->Y, (1 - this->x - this->y) * mul, this->clamped);
    }
    else
    {
        return ColorXYZ(0.f, 0.f, 0.f, this->clamped);
    }
}

ColorHSV::ColorHSV( float H, float S, float V, bool clamped )
{
    this->clamped = clamped || S < 0 || S > 1 || V < 0 || V > 1;
    this->H = clamphue(H);
    this->S = clamp(S, 0, 1);
    this->V = clamp(V, 0, 1);
}

ColorRGB ColorHSV::toRGB()
{
    if(this->S <= 0)
    {
        return ColorRGB(this->V, this->V, this->V, this->clamped);
    }

    float H = clamphue(this->H) / 60.f;
    float C = this->V * this->S;
    float m = this->V - C;
    float X = C * (1.f - fabs(fmod(H, 2.f) - 1.f)) + m;

    C += m;

    if(H >= 5.f) 
        return ColorRGB(C, m, X, this->clamped);
    if(H >= 4.f) 
        return ColorRGB(X, m, C, this->clamped);
    if(H >= 3.f) 
        return ColorRGB(m, X, C, this->clamped);
    if(H >= 2.f) 
        return ColorRGB(m, C, X, this->clamped);
    if(H >= 1.f) 
        return ColorRGB(X, C, m, this->clamped);

    return ColorRGB(C, X, m, this->clamped);
}

ColorHSL::ColorHSL( float H, float S, float L, bool clamped )
{
    this->clamped = clamped || S < 0.f || S > 1.f || L < 0.f || L > 1.f;
    this->H = clamphue(H);
    this->S = clamp(S, 0.f, 1.f);
    this->L = clamp(L, 0.f, 1.f);
}

ColorRGB ColorHSL::toRGB()
{
    if(this->S <= 0.f)
    {
        return ColorRGB(this->S, this->S, this->S, this->clamped);
    }

    float H = clamphue(this->H) / 60.f;
    float C = (1 - abs(this->L * 2.f - 1.f)) * this->S;
    float m = this->L - C * 0.5f;
    float X = C * (1.f - fabs(fmod(H, 2.f) - 1.f)) + m;

    C += m;

    if(H >= 5.f) 
        return ColorRGB(C, m, X, this->clamped);
    if(H >= 4.f) 
        return ColorRGB(X, m, C, this->clamped);
    if(H >= 3.f) 
        return ColorRGB(m, X, C, this->clamped);
    if(H >= 2.f) 
        return ColorRGB(m, C, X, this->clamped);
    if(H >= 1.f) 
        return ColorRGB(X, C, m, this->clamped);

    return ColorRGB(C, X, m, this->clamped);
}

ColorYUV::ColorYUV( float Y, float U, float V, bool clamped )
{
    this->clamped = clamped || Y < 0.f || Y > 1.f || U < 0.f || V > 1.f || V < 0.f || V > 1.f;
    this->Y = clamp(Y, 0.f, 1.f);
    this->U = clamp(U, 0.f, 1.f);
    this->V = clamp(V, 0.f, 1.f);
}

ColorRGB ColorYUV::toRGB()
{
    auto mat = yuvmatrix;

    float u = (this->U - 0.5f) / 0.5f * (1.f - mat.bScale);
    float v = (this->V - 0.5f) / 0.5f * (1.f - mat.rScale);

    float r = v + this->Y;
    float b = u + this->Y;
    float g = (this->Y - r * mat.rScale - b * mat.bScale) / mat.gScale;

    return ColorRGB(r, g, b, this->clamped);
}

ColorYIQ::ColorYIQ( float Y, float I, float Q, bool clamped )
{
    this->clamped = clamped || Y < 0.f || Y > 1.f || I < 0.f || I > 1.f || Q < 0.f || Q > 1.f;
    this->Y = clamp(Y, 0.f, 1.f);
    this->I = clamp(I, 0.f, 1.f);
    this->Q = clamp(Q, 0.f, 1.f);
}

ColorRGB ColorYIQ::toRGB()
{
    float i = this->I - 0.5f;
    float q = this->Q - 0.5f;

    return ColorRGB(
        this->Y + i * 1.13933588212202582f - q * 0.649035964281386078f,
        this->Y - i * 0.32416610079155499f + q * 0.676636193255190191f,
        this->Y - i * 1.31908708412142932f - q * 1.78178677298826495f,
        this->clamped);
}

