#include "nullspace_finders.h"

/* Returns the size of the feature cluster within 4 regions, given the
 * chunk position of the feature within each region.
 *
 * feat parameters represent the chunk coordinates of a feature relative
 * to the region that it is in. The two bits at the end of each parameter
 * name represent the relative x coordinate (right bit) and z coordinate
 * (left bit) of that region.
 *
 * ax, ay, az are the dimensions of the feature.
 *
 * min is the minimum cluster size to search for.
 * 
 * Double witch huts across regions (01,11) or regions (10,11) are not
 * counted by this function, as this function is intended to be used to
 * check a large rectangle of regions, and those double witch huts will
 * be caught when checking another overlapping set of 4 regions.
 */
int clusterSize(Pos *feat00, Pos *feat01, Pos *feat10, Pos *feat11,
	int ax, int ay, int az, int min) {
    int dx1, dz1, dx2, dz2;
    int diag = 0;

    // check if diagonal features are in range
    // different bit set for each diagonal
    dx1 = (feat11->x + 32) - feat00->x;
    dz1 = (feat11->z + 32) - feat00->z;
    if(dx1*dx1 + dz1*dz1 <= 256) diag |= 1;

    dx2 = (feat01->x + 32) - feat10->x;
    dz2 = (feat10->z + 32) - feat01->z;
    if(dx2*dx2 + dz2*dz2 <= 255) diag |= 2;

    float sqrad;

    // check for quad features
    if(min < 4 && diag == 3) {
	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    ax, ay, az,
	    32, 128
	);
	if(sqrad < 128) {
	    return 4;
	}
    }

    // check for triple features
    // uses getEnclosingRadius for 4 features, but replaces the excluded
    // position with the midpoint of the two diagonal positions
    if(diag & 1) {
	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    feat00->x + (dx1 / 2), feat11->z - (dz1 / 2),
	    ax, ay, az,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}

	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    feat11->x - (dx1 / 2), feat00->z + (dz1 / 2),
	    feat10->x, feat10->z,
	    ax, ay, az,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}
    }
    if(diag & 2) {
	sqrad = getEnclosingRadius(
	    feat10->x + (dx2 / 2), feat01->z + (dz2 / 2),
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    ax, ay, az,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}

	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat01->x - (dx2 / 2), feat10->z - (dz2 / 2),
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    ax, ay, az,
	    32, 128
	);
	if(sqrad < 128) {
	    return 3;
	}
    }

    return 1;
}
