#ifndef NULLSPACE_FINDERS_H_
#define NULLSPACE_FINDERS_H_

#include "finders.h"

/* Returns the size of the feature cluster within 4 regions, given the
 * chunk position of the feature within each region.
 *
 * Each parameter represents the chunk coordinates of a feature relative
 * to the region that it is in. The two bits at the end of each parameter
 * name represent the relative x coordinate (right bit) and z coordinate
 * (left bit) of that region.
 * 
 * Double witch huts across regions (01,11) or regions (10,11) are not
 * counted by this function, as this function is intended to be used to
 * check a large rectangle of regions, and those double witch huts will
 * be caught when checking another overlapping set of 4 regions.
 */
int clusterSize(Pos *feat00, Pos *feat01, Pos *feat10, Pos *feat11
	int ax, int ay, int az, int min);

#endif
