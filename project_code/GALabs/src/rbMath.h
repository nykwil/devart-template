#pragma once
#include "ofMain.h"

// Copyright 2001 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.


// Assume that classes are already given for the objects:
//    Point and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test  equality
//            != to test  inequality
//            Point   = Point ± Vector
//            Vector =  Point - Point
//            Vector =  Scalar * Vector    (scalar product)
//            Vector =  Vector * Vector    (3D cross product)
//    Line and Ray and Segment with defining  points {Point P0, P1;}
//        (a Line is infinite, Rays and  Segments start at P0)
//        (a Ray extends beyond P1, but a  Segment ends at P1)
//    Plane with a point and a normal {Point V0; Vector  n;}
//===================================================================


#define SMALL_NUM   0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y)
#define perp(u,v)  ((u).x * (v).y - (u).y * (v).x)  // perp product  (2D)

struct Segment
{
	ofVec2f P0;
	ofVec2f P1;
};

// inSegment(): determine if a point is inside a segment
//    Input:  a point P, and a collinear segment S
//    Return: 1 = P is inside S
//            0 = P is  not inside S
int inSegment( ofVec2f P, Segment S)
{
	if (S.P0.x != S.P1.x) {    // S is not  vertical
		if (S.P0.x <= P.x && P.x <= S.P1.x)
			return 1;
		if (S.P0.x >= P.x && P.x >= S.P1.x)
			return 1;
	}
	else {    // S is vertical, so test y  coordinate
		if (S.P0.y <= P.y && P.y <= S.P1.y)
			return 1;
		if (S.P0.y >= P.y && P.y >= S.P1.y)
			return 1;
	}
	return 0;
}
//===================================================================

// intersect2D_2Segments(): find the 2D intersection of 2 finite segments
//    Input:  two finite segments S1 and S2
//    Output: *I0 = intersect point (when it exists)
//            *I1 =  endpoint of intersect segment [I0,I1] (when it exists)
//    Return: 0=disjoint (no intersect)
//            1=intersect  in unique point I0
//            2=overlap  in segment from I0 to I1
int intersect2D_2Segments( Segment S1, Segment S2, ofVec2f& I0, ofVec2f& I1 )
{
	ofVec2f    u = S1.P1 - S1.P0;
	ofVec2f    v = S2.P1 - S2.P0;
	ofVec2f    w = S1.P0 - S2.P0;
	float     D = perp(u,v);

	// test if  they are parallel (includes either being a point)
	if (fabs(D) < SMALL_NUM) {           // S1 and S2 are parallel
		if (perp(u,w) != 0 || perp(v,w) != 0)  {
			return 0;                    // they are NOT collinear
		}
		// they are collinear or degenerate
		// check if they are degenerate  points
		float du = dot(u,u);
		float dv = dot(v,v);
		if (du==0 && dv==0) {            // both segments are points
			if (S1.P0 !=  S2.P0)         // they are distinct  points
				return 0;
			I0 = S1.P0;                 // they are the same point
			return 1;
		}
		if (du==0) {                     // S1 is a single point
			if  (inSegment(S1.P0, S2) == 0)  // but is not in S2
				return 0;
			I0 = S1.P0;
			return 1;
		}
		if (dv==0) {                     // S2 a single point
			if  (inSegment(S2.P0, S1) == 0)  // but is not in S1
				return 0;
			I0 = S2.P0;
			return 1;
		}
		// they are collinear segments - get  overlap (or not)
		float t0, t1;                    // endpoints of S1 in eqn for S2
		ofVec2f w2 = S1.P1 - S2.P0;
		if (v.x != 0) {
			t0 = w.x / v.x;
			t1 = w2.x / v.x;
		}
		else {
			t0 = w.y / v.y;
			t1 = w2.y / v.y;
		}
		if (t0 > t1) {                   // must have t0 smaller than t1
			float t=t0; t0=t1; t1=t;    // swap if not
		}
		if (t0 > 1 || t1 < 0) {
			return 0;      // NO overlap
		}
		t0 = t0<0? 0 : t0;               // clip to min 0
		t1 = t1>1? 1 : t1;               // clip to max 1
		if (t0 == t1) {                  // intersect is a point
			I0 = S2.P0 +  t0 * v;
			return 1;
		}

		// they overlap in a valid subsegment
		I0 = S2.P0 + t0 * v;
		I1 = S2.P0 + t1 * v;
		return 2;
	}

	// the segments are skew and may intersect in a point
	// get the intersect parameter for S1
	float     sI = perp(v,w) / D;
	if (sI < 0 || sI > 1)                // no intersect with S1
		return 0;

	// get the intersect parameter for S2
	float     tI = perp(u,w) / D;
	if (tI < 0 || tI > 1)                // no intersect with S2
		return 0;

	I0 = S1.P0 + sI * u;                // compute S1 intersect point
	return 1;
}
//===================================================================
