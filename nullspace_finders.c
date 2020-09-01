#include <stdlib.h>
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
 * Double features across regions (01,11) or regions (10,11) are not
 * counted by this function, as this function is intended to be used to
 * check a large rectangle of regions, and those double features will
 * be caught when checking another overlapping set of 4 regions.
 */
int clusterSize(Pos *feat00, Pos *feat01, Pos *feat10, Pos *feat11,
	int ax, int ay, int az, int min) {
    int dx1, dz1, dx2, dz2;
    int diag = 0;

    // check if diagonal features are in range
    // different bit set for each diagonal
    dx1 = ((feat11->x + 32 - feat00->x)<<4) + ax;
    dz1 = ((feat11->z + 32 - feat00->z)<<4) + az;
    if(dx1*dx1 + ay*ay + dz1*dz1 <= 65536) diag |= 1;

    dx2 = ((feat01->x + 32 - feat10->x)<<4) + ax;
    dz2 = ((feat10->z + 32 - feat01->z)<<4) + az;
    if(dx2*dx2 + ay*ay + dz2*dz2 <= 65536) diag |= 2;

    float sqrad;

    // check for quad features
    if(min <= 4 && diag == 3) {
	sqrad = getEnclosingRadius(
	    feat00->x, feat00->z,
	    feat11->x, feat11->z,
	    feat01->x, feat01->z,
	    feat10->x, feat10->z,
	    ax, ay, az,
	    32, 128
	);
	if(sqrad < 128) return 4;
    }

    // check for triple features
    // uses getEnclosingRadius for 4 features, but replaces the excluded
    // position with the midpoint of the two diagonal positions
    if(min <= 3) {
	if(diag & 1) {
	    sqrad = getEnclosingRadius(
		feat00->x, feat00->z,
		feat11->x, feat11->z,
		feat01->x, feat01->z,
		feat00->x + (dx1 / 2), feat11->z - (dz1 / 2),
		ax, ay, az,
		32, 128
	    );
	    if(sqrad < 128) return 3;

	    sqrad = getEnclosingRadius(
		feat00->x, feat00->z,
		feat11->x, feat11->z,
		feat11->x - (dx1 / 2), feat00->z + (dz1 / 2),
		feat10->x, feat10->z,
		ax, ay, az,
		32, 128
	    );
	    if(sqrad < 128) return 3;
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
	    if(sqrad < 128) return 3;

	    sqrad = getEnclosingRadius(
		feat00->x, feat00->z,
		feat01->x - (dx2 / 2), feat10->z - (dz2 / 2),
		feat01->x, feat01->z,
		feat10->x, feat10->z,
		ax, ay, az,
		32, 128
	    );
	    if(sqrad < 128) return 3;
	}
    }

    // check for double features
    // check diagonal regions
    if(min <= 2) {
	if(diag) return 2;

	dx1 = ((feat01->x + 32 - feat00->x)<<4) + ax;
	dz1 = abs((feat01->z + 32 - feat00->z)<<4) + az;
	if(dx1*dx1 + ay*ay + dz1*dz1 <= 65536) return 2;

	dx2 = abs((feat10->x + 32 - feat00->x)<<4) + ax;
	dz2 = ((feat10->z + 32 - feat00->z)<<4) + az;
	if(dx2*dx2 + ay*ay + dz2*dz2 <= 65536) return 2;
    }

    return 1;
}
