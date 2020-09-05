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
    // find search bounding box
    int xMin01, xMin23, xMax;
    xMin01 = p0.x < p1.x ? p0.x : p1.x;
    xMin23 = p2.x < p3.x ? p2.x : p3.x;
    xMax = (xMin01 < xMin23 ? xMin01 : xMin23) + max;
    
    int zMin01, zMin23, zMax;
    zMin01 = p0.z < p1.z ? p0.z : p1.z;
    zMin23 = p2.z < p3.z ? p2.z : p3.z;
    zMax = (zMin01 < zMin23 ? zMin01 : zMin23) + max;

    int xMax01, xMax23, xMin;
    xMax01 = p0.x > p1.x ? p0.x : p1.x;
    xMax23 = p2.x > p3.x ? p2.x : p3.x;
    xMin = (xMax01 > xMax23 ? xMax01 : xMax23) + ax - max;
    
    int zMax01, zMax23, zMin;
    zMax01 = p0.z > p1.z ? p0.z : p1.z;
    zMax23 = p2.z > p3.z ? p2.z : p3.z;
    zMin = (zMax01 > zMax23 ? zMax01 : zMax23) + az - max;

    int sqrad = 0x7fffffff;

    // search for ideal center
    int x, z;
    for(x = xMin; x <= xMax; ++x) {
	for(z = zMin; z <= zMax; ++z) {
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
    // find search bounding box
    int xMin01, xMax;
    xMin01 = p0.x < p1.x ? p0.x : p1.x;
    xMax = (xMin01 < p2.x ? xMin01 : p2.x) + max;

    int zMin01, zMax;
    zMin01 = p0.z < p1.z ? p0.z : p1.z;
    zMax = (zMin01 < p2.z ? zMin01 : p2.z) + max;

    int xMax01, xMin;
    xMax01 = p0.x > p1.x ? p0.x : p1.x;
    xMin = (xMax01 > p2.x ? xMax01 : p2.x) + ax - max;

    int zMax01, zMin;
    zMax01 = p0.z > p1.z ? p0.z : p1.z;
    zMin = (zMax01 > p2.z ? zMax01 : p2.z) + az - max;

    int sqrad = 0x7fffffff;

    // search for ideal center
    int x, z;
    for(x = xMin; x <= xMax; ++x) {
	for(z = zMin; z <= zMax; ++z) {
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
