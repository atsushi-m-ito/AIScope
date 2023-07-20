#pragma once
#include <stdlib.h>
#include "vec3.h"
#include "mat33.h"

class CellPartition
{
protected:
	
	int m_cell_count;	//全cell数(m_cell_x * m_cell_y * m_cell_z)
	int m_cell_x;		//x方向のcell数
	int m_cell_y;		//y方向のcell数
	int m_cell_z;		//z方向のcell数
	int m_is_orthogonal;	//直交座標なら1

	double m_necessary_cellwidth;

	vec3d m_box;
	vec3d m_boxi;
	mat33d m_Am;		//sim.boxの軸テンソル(a, b, c).

	vec3d m_periodic_vec[27];

	int ResetCell(int a_mcx, int a_mcy, int a_mcz);
	void RefreshPeriodicVector();

public:
	CellPartition(double cellwidth);
	virtual ~CellPartition();

	/*
	ボックスサイズを元にセル数を求める
	*/
	void CalcNumCells(double boxsize_x, double boxsize_y, double boxsize_z, double necessary_cellwidth, int* num_cell_x, int* num_cell_y, int* num_cell_z);
	void CalcNumCells(const vec3d& a, const vec3d& b, const vec3d& c, double necessary_cellwidth, int* num_cell_x, int* num_cell_y, int* num_cell_z, double* vertical_abc);

	//void SetMaxNumParticles(int maxcount) {};//何もしない.将来廃止する//

	//void SetBoxsizeCell(double boxsize_x, double boxsize_y, double boxsize_z, int cellcount_x, int cellcount_y, int cellcount_z);
	//void EstimateCellCount(double boxsize_x, double boxsize_y, double boxsize_z, int* cellx, int* celly, int* cellz);
	void SetBoxsize(double x, double y, double z);
	void SetBoxsizeWithMargin(double boxsize_x, double boxsize_y, double boxsize_z, double margin, int is_margin_x, int is_margin_y, int is_margin_z, int* margin_xyz);
	void SetBoxAxis(const vec3d& a, const vec3d& b, const vec3d& c);
	void SetBoxAxis(const mat33d& Am);
	void SetBoxAxisWithMargin(const vec3d& a, const vec3d& b, const vec3d& c, double margin, int is_margin_x, int is_margin_y, int is_margin_z, int* margin_xyz);
	//mat33d GetBoxAxis();

	void Split(int istart, int count, vec3d* __restrict r, int* __restrict icell);
	void Split3D(int istart, int count, const vec3d* __restrict r, int* __restrict icell);

	void GetCellWidth(vec3d *width);
	void GetCellCount(int* cell_x, int* cell_y, int* cell_z);


};

