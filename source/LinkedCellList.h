#ifndef LinkedCellList_H
#define LinkedCellList_H

#pragma once

#ifdef _OPENMP
#include <omp.h>
#endif

#include "vec3.h"
#include "mat33.h"
#include "atom_sort.h"

#include "INeighborFinder.h"

#include "CellPartition.h"


struct LCL_INFO{
	int cell_x;	//x方向数のセル数//
	int cell_y;	//y方向数のセル数//
	int cell_z;	//z方向数のセル数//
	int* icell;	//粒子の入っているセル番号//
	int* inext;	//同一cell内での次の粒子へのリンク//
	int* first_in_cell;	//そのcellに入っている先頭の粒子番号//
	int* i3d_cell;	//粒子の入っている整数化したセル座標//
};

struct BVEC_REGION{
	int startpoint;		// start index of i-th atom's bond in BOP_BVECTOR array //
	int endpoint;		// end index of i-th atom's bond in BOP_BVECTOR array //
	int twobodyendpoint;
};


class LCL : public INeighborFinder, public CellPartition
{
private:
	LCL(const LCL&);
	LCL& operator=(const LCL&);

protected:
	//int m_max_count;	//取りえる最大粒子数

	int m_bufsz_atom;
	int* m_inext;		//同じcellに入っている次の粒子を返す(粒子ごとのデータ)//
	int* m_icell;		//粒子の入っているセルの番号(粒子ごとのデータ)//
	int* m_i3d_cell;		//粒子の入っているセルの番号(粒子ごとのデータ)//

		
	int m_cell_bufsz;	//cellごとの配列のバッファサイズ(再確保時にこれを超えたらrealloc).	
	int* m_first_in_cell;	//そのcellに入っている先頭の粒子番号(cellごとのデータ).//omp:shared//
	
	
	void mCheckParticleBuffer(int maxcount);
	void mCheckCellBuffer();

	void mFolding(double& x, double box_width, int& cell_posision, int num_cell);
	void I1DtoI3D(int i1d, int* i3d);
	int I3DtoI1D(const int* i3d);
public:
	
	LCL(double cellwidth);
	virtual ~LCL();	

	
	//void GetLinkedList(int count, vec3d* r, int* knd, LCL_INFO* info, vec3d *periodic_vec);
	void Create(int count, vec3d* __restrict r, const int* __restrict knd);
	void Create(int istart, int count, vec3d* __restrict r, const int* __restrict knd);
	void Insert(int atom_index_begin, int atom_count, const int* cell_indexs);

	void GetCellInfo(LCL_INFO* info, vec3d *periodic_vec) const;
	void GetCellInfo(LCL_INFO* info) const;
	
	
	void Clear();
	void Calculate(int istart, int count, vec3d* __restrict r, const int* __restrict knd);
	

	template <class TBVECTOR>
	int CreateBondList(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond);

    template <class TBVECTOR>
    int CreateBondList_in_parallel(int i_begin, int i_end, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond, int* bond_begin, int* bond_end);

    template <class TBVECTOR>
    int CreateBondList_TEST0(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond);

    template <class TBVECTOR>
    int CreateBondList_TEST1(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond, const int MAX_NUM_BOND);

    template <class TBVECTOR>
    int CreateBondList_TEST2(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond, const int MAX_NUM_BOND);


    



	/*
	INeighborFinder
	*/
	int GetNeighborList(int atom_index, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, int* half_point, const int bvector_size);
	int GetNeighborList(const vec3d& center, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, const int bvector_size);
	int mGetNeighborList(const vec3d& r_i, const int* i3d_cell_i, int ignore_id, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, int* half_point, const int bvector_size);

	int GetIndexInCells(int cell_id, int* indexes);
};

/////////////////////////////////////////////
//Create Bond List by using result from LCL//
/////////////////////////////////////////////


#include "LCL_CreateBondList.h"


#endif 
