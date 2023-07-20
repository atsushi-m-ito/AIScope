#pragma once

#include "vec3.h"


struct BVECTOR {
	int pairIndex;	// index j which is pair of i //
	vec3d rij;		// normalized relative vector //
	double dr;
};


class INeighborFinder {

public:
	virtual ~INeighborFinder() {};

	virtual int GetNeighborList(int atom_index, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, int* half_point, const int bvector_size) = 0;
};

