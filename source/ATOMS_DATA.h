#ifndef ATOMS_DATA_H
#define ATOMS_DATA_H

#include "vec3.h"
#include "mat33.h"
#include "BOND_INFO.h"


class ATOMS_DATA {
public:
	int frame=0;	//読み込んだ順に0からインクリメント
	int no = 0;
	int pcnt = 0;
	int istep = 0;
	float realtm = 0.0f;
	mat33d boxaxis;
	vec3d boxorg;
	float tempave;
	float tempvar;
	int* knd = nullptr;
	int* id = nullptr;
	vec3f* r = nullptr;
	vec3f* p = nullptr;
	char* info = nullptr;		//主に文字列などに使用

	BOND_INFO bond;

	ATOMS_DATA() = default;

	virtual ~ATOMS_DATA(){
		delete [] knd;
		delete [] id;
		delete [] r;
		delete [] p;
		delete [] info;
	};

};


#endif    // !ATOMS_DATA