#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "finders.h"
#include "nullspace_finders.h"

/* Returns the minum radius enclosing 4 features.
 *
 * feat parameters are the positions of each feature. (in blocks) These
 * parameters do not need to be in any particular order.
 *
 * ax, ay, az are the dimensions of the box around each feature that must
 * fit inside the radius
 *
 * max is the maximum radius to start searching from.
 *
 * Based off of getEnclosingRadius from cubiomes.
 */
float getMinRadius4(Pos p0, Pos p1, Pos p2, Pos p3,
	int ax, int ay, int az, int max) {
    // find outer rectangle
    int *xMin;
    xMin = p0.x < p1.x ? &(p0.x) : &(p1.x);
    xMin = *xMin < p2.x ? xMin : &(p2.x);
    xMin = *xMin < p3.x ? xMin : &(p3.x);

    int *zMin;
    zMin = p0.z < p1.z ? &(p0.z) : &(p1.z);
    zMin = *zMin < p2.z ? zMin : &(p2.z);
    zMin = *zMin < p3.z ? zMin : &(p3.z);

    int *xMax, *x2Max;
    x2Max = p0.x > p1.x ? &(p1.x) : &(p0.x);
    xMax = p0.x > p1.x ? &(p0.x) : &(p1.x);
    x2Max = *xMax > p2.x ? (*x2Max > p2.x ? x2Max : &(p2.x)) : xMax;
    xMax = *xMax > p2.x ? xMax : &(p2.x);
    x2Max = *xMax > p3.x ? (*x2Max > p3.x ? x2Max : &(p3.x)) : xMax;
    xMax = *xMax > p3.x ? xMax : &(p3.x);
    *xMax += ax;
    *x2Max += ax;
    
    int *zMax, *z2Max;
    z2Max = p0.z > p1.z ? &(p1.z) : &(p0.z);
    zMax = p0.z > p1.z ? &(p0.z) : &(p1.z);
    z2Max = *zMax > p2.z ? (*z2Max > p2.z ? z2Max : &(p2.z)) : zMax;
    zMax = *zMax > p2.z ? zMax : &(p2.z);
    z2Max = *zMax > p3.z ? (*z2Max > p3.z ? z2Max : &(p3.z)) : zMax;
    zMax = *zMax > p3.z ? zMax : &(p3.z);
    *zMax += az;
    *z2Max += az;

    // find inner rectangle
    int xCenterMin = *xMax - max;
    int xCenterMax = *xMin + max;
    int zCenterMin = *zMax - max;
    int zCenterMax = *zMin + max;

    int sqrad = 0x7fffffff;

    // search for ideal center
    int x, z;
    for(x = xCenterMin; x <= xCenterMax; ++x) {
	for(z = zCenterMin; z <= zCenterMax; ++z) {
	    int sq = 0;
	    int s;
	    s = (x-p0.x)*(x-p0.x) + (z-p0.z)*(z-p0.z); if(s > sq) sq = s;
	    s = (x-p1.x)*(x-p1.x) + (z-p1.z)*(z-p1.z); if(s > sq) sq = s;
	    s = (x-p2.x)*(x-p2.x) + (z-p2.z)*(z-p2.z); if(s > sq) sq = s;
	    s = (x-p3.x)*(x-p3.x) + (z-p3.z)*(z-p3.z); if(s > sq) sq = s;
	    if(sq < sqrad) {
		sqrad = sq;
	    }
	}
    }

    return sqrad < 0x7fffffff ? sqrtf(sqrad + ay*ay/4.0f) : 0xffff;
}

/* Returns the minum radius enclosing 3 features.
 *
 * feat parameters are the positions of each feature. (in blocks) These
 * parameters do not need to be in any particular order.
 *
 * ax, ay, az are the dimensions of the box around each feature that must
 * fit inside the radius
 *
 * max is the maximum radius to start searching from.
 *
 * Based off of getEnclosingRadius from cubiomes.
 */
float getMinRadius3(Pos p0, Pos p1, Pos p2,
	int ax, int ay, int az, int max) {
    // find outer rectangle
    int *xMin;
    xMin = p0.x < p1.x ? &(p0.x) : &(p1.x);
    xMin = *xMin < p2.x ? xMin : &(p2.x);

    int *zMin;
    zMin = p0.z < p1.z ? &(p0.z) : &(p1.z);
    zMin = *zMin < p2.z ? zMin : &(p2.z);

    int *xMid, *xMax;
    xMid = p0.x > p1.x ? &(p1.x) : &(p0.x);
    xMax = p0.x > p1.x ? &(p0.x) : &(p1.x);
    xMid = *xMax > p2.x ? (*xMid > p2.x ? xMid : &(p2.x)) : xMax;
    xMax = *xMax > p2.x ? xMax : &(p2.x);
    *xMax += ax;
    if(2 * (*xMid) > *xMax + *xMin) *xMid += ax;

    int *zMid, *zMax;
    zMid = p0.z > p1.z ? &(p1.z) : &(p0.z);
    zMax = p0.z > p1.z ? &(p0.z) : &(p1.z);
    zMid = *zMax > p2.z ? (*zMid > p2.z ? zMid : &(p2.z)) : zMax;
    zMax = *zMax > p2.z ? zMax : &(p2.z);
    if(2 * (*zMid) > *zMax + *zMin) *zMid += az;

    // find inner rectangle
    int xCenterMin = *xMax - max;
    int xCenterMax = *xMin + max;
    int zCenterMin = *zMax - max;
    int zCenterMax = *zMin + max;

    int sqrad = 0x7fffffff;

    // search for ideal center
    int x, z;
    for(x = xCenterMin; x <= xCenterMax; ++x) {
	for(z = zCenterMin; z <= zCenterMax; ++z) {
	    int sq = 0;
	    int s;
	    s = (x-p0.x)*(x-p0.x) + (z-p0.z)*(z-p0.z); if(s > sq) sq = s;
	    s = (x-p1.x)*(x-p1.x) + (z-p1.z)*(z-p1.z); if(s > sq) sq = s;
	    s = (x-p2.x)*(x-p2.x) + (z-p2.z)*(z-p2.z); if(s > sq) sq = s;
	    if(sq < sqrad) {
		sqrad = sq;
	    }
	}
    }

    return sqrad < 0x7fffffff ? sqrtf(sqrad + ay*ay/4.0f) : 0xffff;
}

/* Returns the minum radius enclosing 2 features.
 *
 * feat parameters are the positions of each feature. (in blocks) These
 * parameters do not need to be in any particular order.
 *
 * ax, ay, az are the dimensions of the box around each feature that must
 * fit inside the radius
 *
 * Based off of getEnclosingRadius from cubiomes.
 */
float getMinRadius2(Pos p0, Pos p1, int ax, int ay, int az) {
    int dx = abs(p1.x - p0.x) + ax;
    int dz = abs(p1.z - p0.z) + az;

    int sqd = dx*dx + ay*ay + dz*dz;
    return sqrtf(sqd) / 2.0f;
}

/* Returns the size of the feature cluster within 4 regions, given the
 * chunk position of the feature within each region.
 *
 * feat parameters represent the block coordinates of a feature. The two
 * bits at the end of each parameter name represent the relative x
 * coordinate (right bit) and z coordinate (left bit) of that region.
 *
 * ax, ay, az are the dimensions of the feature.
 *
 * min is the minimum cluster size to search for.
 *
 * Double features across regions (01,11) or regions (10,11) are not
 * counted by this function, as this function is intended to be used to
 * check a large rectangle of regions, and those double features will
 * be caught when checking another overlapping set of 4 regions.
 */
int getClusterSize(Pos feat00, Pos feat01, Pos feat10, Pos feat11,
	int ax, int ay, int az, int min) {
    float r;
    int diag = 0;

    // check if diagonal features are in range
    if(getMinRadius2(feat00, feat11, ax, ay, az) <= 128) diag |= 1;
    if(getMinRadius2(feat01, feat10, ax, ay, az) <= 128) diag |= 2;

    // check for quad features
    if(min <= 4 && diag == 3) {
	r = getMinRadius4(feat00, feat01, feat10, feat11,
		ax, ay, az, 128);
	if(r <= 128) return 4;
    }

    // check for triple features
    if(min <= 3) {
	// check including first diagonal
	if(diag & 1) {
	    r = getMinRadius3(feat00, feat01, feat11,
		    ax, ay, az, 128);
	    if(r <= 128) return 3;

	    r = getMinRadius3(feat00, feat10, feat11,
		    ax, ay, az, 128);
	    if(r <= 128) return 3;
	}
	// check including second diagonal
	if(diag & 2) {
	    r = getMinRadius3(feat00, feat01, feat10,
		    ax, ay, az, 128);
	    if(r <= 128) return 3;

	    r = getMinRadius3(feat01, feat10, feat11,
		    ax, ay, az, 128);
	    if(r <= 128) return 3;
	}
    }

    // check for double features
    if(min <= 2) {
	// reuse diagonal checks
	if(diag) return 2;

	if(getMinRadius2(feat00, feat01, ax, ay, az) <= 128) return 2;
	if(getMinRadius2(feat00, feat10, ax, ay, az) <= 128) return 2;
    }

    return 1;
}
