#pragma once


#include <stdio.h>
#include "targetver.h"
#include <windows.h>


struct MATERIAL_INFO{
	UINT indexCount;
	UINT indexOffset;
	float diffuse_color[4];
    WCHAR* texture_name;
	UINT textureid;
	
};



struct MATERIAL_COMPOSIT {
	int begin_index;		//複数のマテリアルをグリーピングした際の範囲
	int end_index;
	int rank;
};


inline bool operator<(const MATERIAL_COMPOSIT& left, const MATERIAL_COMPOSIT& right)
{
	if (left.rank == right.rank) {
		return left.begin_index < right.begin_index;
	} else {
		return left.rank < right.rank;
	}
}


