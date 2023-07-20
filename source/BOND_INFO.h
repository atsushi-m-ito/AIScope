#ifndef BOND_INFO_H
#define BOND_INFO_H

#pragma once

#include "vec3.h"

struct VECTOR_IJ{
	int i;
	int j;
	vec3f v;
};

struct BOND_INFO{
	
	int count;
	size_t sz_bvector;
	VECTOR_IJ* bvector;	//ボンドを作る原子ペアのindex//
	size_t sz_coords;
	int* coords;		//原子ごとの配位数//
	
	BOND_INFO():
		count(0),
		sz_bvector(0),
		bvector(NULL),
		sz_coords(0),
		coords(NULL)
	{

	};

	virtual ~BOND_INFO(){

		if(bvector){ delete [] bvector;}
		if(coords){ delete [] coords;}
	};
};




#endif //!BOND_INFO_H