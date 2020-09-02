#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "finders.h"
#include "nullspace_finders.h"

/* Searches a given range in a given seed for notable structure clusters.
 *
 * If center_triples is enabled, will translate seeds such that triple
 * feature clusters are at (0,0) and call itself on the new seeds with
 * center_triples disabled.
 */
// TODO: Figure out how to add triple_centered seeds in the output.
void findClustersInRange(int64_t seed, int range, bool center_triples,
	ClusterCount *ccnt) {
    const int spos_dim = (2 * range) + 1;
    Pos *spos_feat = malloc(sizeof(Pos) * spos_dim * spos_dim);
    Pos *spos_mon = malloc(sizeof(Pos) * spos_dim * spos_dim);

    // find all structure chunk positions
    int spos_idx = 0;
    for(int x = -range; x <= range; ++x) {
	for(int z = -range; z <= range; ++z, ++spos_idx) {
	    spos_feat[spos_idx] = getFeatureChunkInRegion(
		FEATURE_CONFIG, seed, x, z
	    );
	    spos_mon[spos_idx] = getLargeStructureChunkInRegion(
		MONUMENT_CONFIG, seed, x, z
	    );
	}
    }

    // search for structure clusters
    spos_idx = 0;
    for(int x = -range; x < range; ++x, ++spos_idx) {
	for(int x = -range; z < range; ++z, ++spos_idx) {
	    int feat_cluster_size = clusterSize(
		&spos_feat[spos_idx],
		&spos_feat[spos_idx + spos_dim],
		&spos_feat[spos_idx + 1],
		&spos_feat[spos_idx + spos_dim + 1],
		7+1, 7+43+1, 9+1, 3
	    );
	    if(feat_cluster_size == 4) {
		++(ccnt->qfcnt);
	    } else if(feat_cluster_size == 3) {
		// search again with triple feature at (0,0)
		if(center_triples && x != 0 && z != 0) {
		    int64_t tseed = moveStructure(seed, -x, -z);
		    findClustersInRange(tseed, range, false, ccnt);
		} else {
		    ++(ccnt->tfcnt);
		}
	    } else if(feat_cluster_size == 2) {
		++(ccnt->dfcnt);
	    }

	    int mon_cluster_size = clusterSize(
		&spos_mon[spos_idx],
		&spos_mon[spos_idx + spos_dim],
		&spos_mon[spos_idx + 1],
		&spos_mon[spos_idx + spos_dim + 1],
		58+1, /*TODO: replace with gaurdian farm height*/ 0, 58+1
	    );
	    if(mon_cluster_size == 4) {
		++(ccnt->qmcnt);
	    } else if(mon_cluster_size == 3) {
		++(ccnt->tmcnt);
	    } else if(mon_cluster_size == 2) {
		++(ccnt->dmcnt);
	    }
	}
    }

    free(spos_feat);
    free(spos_mon);
}

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
