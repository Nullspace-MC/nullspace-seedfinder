#ifndef NULLSPACE_FINDERS_H_
#define NULLSPACE_FINDERS_H_

#include <stdbool.h>
#include <stdint.h>
#include "finders.h"

/* Returns the size of the feature cluster within 4 regions, given the
 * chunk position of the feature within each region.
 *
 * feat parameters represent the chunk coordinates of a feature relative
 * to the region that it is in. The two bits at the end of each parameter
 * name represent the relative x coordinate (right bit) and z coordinate
 * (left bit) of that region.
 *
 * ax, ay, az are the dimensions of the feature. (Increase ay if you need
 * more height for a dropshaft.)
 *
 * min is the minimum cluster size to search for.
 * 
 * Double features across regions (01,11) or regions (10,11) are not
 * counted by this function, as this function is intended to be used to
 * check a large rectangle of regions, and those double features will
 * be caught when checking another overlapping set of 4 regions.
 */
int clusterSize(Pos *feat00, Pos *feat01, Pos *feat10, Pos *feat11,
	int ax, int ay, int az, int min);

#endif
