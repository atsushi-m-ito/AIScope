#ifndef __fad_writer_h__
#define __fad_writer_h__

#ifdef USE_MPI
#include "mpi.h"
#endif

#include <stdlib.h>
#include <stdio.h>
//#include <math.h>

//#include "vec3.h"
//#include "mat33.h"
//#include "mod.h"
//#include "rebojf.h"
//#include "debugprint.h"



class FAD_Writer
{
private:
	FILE* m_fp;
	float* m_buffer4;
	int m_buffer4Sz;
	
public:


	FAD_Writer();
	virtual ~FAD_Writer();

	void Open(const char* fileName);
	void Close();

	void Write(int count, int* knd, int* id, double* r, double* p, double* boxaxisorg);
	void Write(int count, int* knd, int* id, float* r, float* p, float* boxaxisorg);

};



#endif    // __fad_writer_h__
