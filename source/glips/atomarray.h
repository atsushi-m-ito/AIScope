#ifndef atomarray_h
#define atomarray_h


#include "vec3.h"
#include "mat33.h"


class AtomArray{
private:
	int m_buffersize;
	int m_maxID;
public:
	int count;
	int* knd;
	int* id;
	double* mass;
	vec3d* r;
	vec3d* p;
	vec3d* f;
	double* pot;

	AtomArray();
	AtomArray(int buffersize);
	virtual ~AtomArray();

	void ForceClear();
	void CopyAtom(int indexDst, int indexSrc);
	void SwapAtom(int indexDst, int indexSrc);
	void CheckByIndex(int index);
	int SortCheck();
	//int ClearCheck();
	int EjectCheckedElements();
	int GetIndex(int ID);
	int GetIndex(int ID, int count_org);
	int GetRealID(int index);
	
	void Add(int knd, double mass, vec3d* r0, vec3d* p0);//将来的に廃止//
	
	void Add(int knd_i, double mass_d, const vec3d& r0, const vec3d& p0, const vec3d& f0);
	void Add(int knd_i, double mass_d, const vec3d& r0, const vec3d& p0);
	void Add(int knd_i, double mass_d, const vec3d& r0);

	int GetMaxID();
	void IncrementMaxID();

	int GetTotalAtomCount(){ return count;};
	double GetTotalKineticEnergy();
	double GetTotalKineticEnergy(vec3d* K_v);
	double GetTotalPotentialEnergy();
	vec3d GetTotalForce();

	

	int GetBufferSize(){
		return m_buffersize;
	};

};

//void CheckAtoms(AtomArray* atoms, int chkcount, int* list);


#endif    // !atomarray_h
