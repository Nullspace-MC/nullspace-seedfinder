#ifndef NULLSPACE_FINDERS_H_
#define NULLSPACE_FINDERS_H_

#include <stdint.h>
#include "finders.h"

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
	int ax, int ay, int az, int max);

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
	int ax, int ay, int az, int max);

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
float getMinRadius2(Pos p0, Pos p1, int ax, int ay, int az);

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
	int ax, int ay, int az, int min);

#endif
