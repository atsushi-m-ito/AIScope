////////////////////////////////////////////////////////
//
//  Interface class of potential //
//
/////////////////////////////////////////////////////////



#ifndef __IPotential_h__
#define __IPotential_h__


#include "vec3.h"
#include "mat33.h"
#include "LinkedCellList.h"
#include "atomarray.h"
//#include "debugprint.h"



class IPotential
	//: public CellDiv 
{

public:

	virtual ~IPotential(){};
	
	virtual void SetMaxNumParticles(int maxcount) = 0;

	virtual void SetParticlePointers(AtomArray* atoms) = 0;
	virtual void SetLinkedCellList(LCL* pLCL) = 0;
	virtual void SetBoxsize(double x, double y, double z) = 0;
//	virtual void SetBoxAxis(const vec3d& a, const vec3d& b, const vec3d& c) = 0;
	virtual void SetBoxAxis(const mat33d& Am) = 0;
	

	virtual void Force(int count, int noloop_count) = 0;
	
	virtual void GetFr(mat33d* a_Fr_m) = 0;
	
	virtual double GetCutoffLength() = 0;

	/*
	the reach (range) of the forces by the i-th atom and the i-j bond.
	this is used as the width of margin in DDM.
	*/
	virtual double GetForceRange() = 0;
	
	virtual void GetParams(double* params) = 0;
	virtual void SetParams(const double* params) = 0;
	virtual int GetNumParams() = 0;
	
	virtual void GetInfo(int mode, void* info) = 0;
	
};

enum TYPE_POTENTIAL{
	TYPE_POT_NULL,		//NULL//
	TYPE_POT_BOP,		//template of potential//
	TYPE_POT_LJ,		//LJ potential//
	TYPE_POT_REBO,		//REBO potential//
//	TYPE_POT_REBOv2,	//
	TYPE_POT_REBOv3,	//REBO potential for low cost openmp parallelization//
	TYPE_POT_REO,		//new carbon potential//
	TYPE_POT_REOv2,		//new carbon potential//
	TYPE_POT_TERSOFF,	//TERSOFF potential//
	TYPE_POT_AFS,		//AFS potential//
	TYPE_POT_AFSI,	//AFSI potential//
	TYPE_POT_AFSIv7,	//AFSI potential for low cost openmp parallelization//
	TYPE_POT_THOMASFERMI,	//Thomas-Fermi potential//
	TYPE_POT_NBONDS		//to count number of coordination//
};

IPotential* CreatePotential(TYPE_POTENTIAL type_pot, int maxcount);


#endif	//!__IPotential_h__