
#include "LinkedCellList.h"

#include <stdio.h>
#include <string.h>

#include "renew.h"
#include "atomarray.h"

#define USE_I3D_CELL

LCL::LCL(double cellwidth) : 
	CellPartition(cellwidth),
	m_cell_bufsz(0),
	m_bufsz_atom(0),
	m_inext(NULL),
	m_icell(NULL),
	m_first_in_cell(NULL),
	m_i3d_cell(NULL)
{	
}


LCL::~LCL(){


	delete [] m_inext;
	delete [] m_icell;
	delete [] m_first_in_cell;
	delete [] m_i3d_cell;


}

void LCL::mCheckParticleBuffer(int maxcount){
	
	if(maxcount <= m_bufsz_atom){ return;}

	const int count_org = m_bufsz_atom;
	m_bufsz_atom = maxcount;

	renew_preserve<int>(&m_inext, count_org, m_bufsz_atom);
	renew_preserve<int>(&m_icell, count_org, m_bufsz_atom);
	renew_preserve<int>(&m_i3d_cell, count_org*3, m_bufsz_atom*3);
	
}

void LCL::mCheckCellBuffer() {

	if ((m_cell_bufsz < m_cell_count) ||
		(m_cell_bufsz > m_cell_count * 2)) {	//buffer再確保.

		m_cell_bufsz = m_cell_count;	//バッファサイズに余裕をもたす.

		delete[] m_first_in_cell;
		m_first_in_cell = new int[m_cell_bufsz];		//master threadだけでリロケート.

	}

}

//



void LCL::Create(int count, vec3d* __restrict r, const int* __restrict knd){
	
	Clear();

	Calculate(0, count, r, knd);

	
}

void LCL::Create(int istart, int count, vec3d* __restrict r, const int* __restrict knd){
	
	Clear();

	Calculate(istart, count, r, knd);
		
}



inline void LCL::mFolding(double& x, double box_width, int& cell_posision, int num_cell) {
	if (cell_posision < 0) {
		x += box_width;
		cell_posision += num_cell;
	} else if (cell_posision >= num_cell) {
		x -= box_width;
		cell_posision -= num_cell;
	}
}


void LCL::Calculate(int istart, int count, vec3d* __restrict r, const int* __restrict knd){


	const int iend = istart + count;

	mCheckParticleBuffer(iend);

	int* __restrict re_icell = m_icell;
		
#ifdef USE_I3D_CELL
	Split3D(istart, count, r, m_i3d_cell);

	{//forlding//

		#pragma omp parallel for
		for (int i = istart; i < iend; ++i) {
			mFolding(r[i].x, m_box.x, m_i3d_cell[i * 3], m_cell_x );
			mFolding(r[i].y, m_box.y, m_i3d_cell[i * 3 + 1], m_cell_y);
			mFolding(r[i].z, m_box.z, m_i3d_cell[i * 3 + 2], m_cell_z);

			
		}
	}


	{
		for (int i = iend - 1; i >= istart; i--) {

			//if (knd[i] < -1) continue;
			const int icell = m_i3d_cell[i * 3] + m_cell_x * (m_i3d_cell[i * 3 + 1] + m_cell_y * m_i3d_cell[i * 3 + 2]);
			re_icell[i] = icell;		


			m_inext[i] = m_first_in_cell[icell];
			m_first_in_cell[icell] = i;

		}

	}

#else

	Split(istart, count, r, m_icell);

	{
		for (int i = iend - 1; i >= istart; i--) {

			//if (knd[i] < -1) continue;
			const int icell = re_icell[i];


			m_inext[i] = m_first_in_cell[icell];
			m_first_in_cell[icell] = i;

		}

	}
#endif

	
}


void LCL::I1DtoI3D(int cell_id, int* i3d) {
	i3d[0] = cell_id % m_cell_x;
	i3d[1] = (cell_id / m_cell_x) % m_cell_y;
	i3d[2] = cell_id / (m_cell_x * m_cell_y);
}


int LCL::I3DtoI1D(const int* i3d) {
	return i3d[0]  + (i3d[1] + i3d[2] * m_cell_y) * m_cell_x;
	
}


void LCL::Insert(int atom_index_begin, int atom_count, const int* cell_indexs){

	mCheckParticleBuffer(atom_index_begin + atom_count);
	
	for (int i = 0; i < atom_count; i++) {
		const int atom_index = atom_index_begin + i;
		const int cell_id = cell_indexs[i];
		m_icell[atom_index] = cell_id;
		m_inext[atom_index] = m_first_in_cell[cell_id];
		m_first_in_cell[cell_id] = atom_index;
#ifdef USE_I3D_CELL
		I1DtoI3D(cell_id, &(m_i3d_cell[atom_index*3]));
#endif
	}

}



void LCL::GetCellInfo(LCL_INFO* info, vec3d *periodic_vec) const {
	
	
//return infomation//
	GetCellInfo(info);
	memcpy(periodic_vec, m_periodic_vec, sizeof(vec3d)*27);

}


void LCL::GetCellInfo(LCL_INFO* info) const {
	
	
//return infomation//
	info->cell_x = m_cell_x;
	info->cell_y = m_cell_y;
	info->cell_z = m_cell_z;
	info->icell = m_icell;
	info->first_in_cell = m_first_in_cell;
    info->inext = m_inext;	

}

	




void LCL::Clear(){
	mCheckCellBuffer();
	// set zero head of chain array
	//#pragma omp master
	{
		memset(m_first_in_cell, -1, m_cell_count * sizeof(int));
	}
}



/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/

int LCL::GetNeighborList(int i, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, int* half_point, const int bvector_size) {
	// local variable ------------------------------;

	return mGetNeighborList(r[i], &(m_i3d_cell[i * 3]), i, r, CUTOFF_SQR, bvector, half_point, bvector_size);
}


int LCL::GetNeighborList(const vec3d& center, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, const int bvector_size) {
	// local variable ------------------------------;
	int i3d_cell[3];
	Split3D(0, 1, &center, i3d_cell);
	vec3d r_i(center);
	mFolding(r_i.x, m_box.x, i3d_cell[0], m_cell_x);
	mFolding(r_i.y, m_box.y, i3d_cell[1], m_cell_y);
	mFolding(r_i.z, m_box.z, i3d_cell[2], m_cell_z);

	int dummy;
	return mGetNeighborList(r_i, i3d_cell, -1, r, CUTOFF_SQR, bvector, &dummy, bvector_size);
}

int LCL::mGetNeighborList(const vec3d& r_i, const int* i3d_cell_i, int ignore_id, const vec3d* r, double CUTOFF_SQR, BVECTOR* bvector, int* half_point, const int bvector_size) {
		// local variable ------------------------------;

	int num_bonds = 0;
	int twobody_endpoint = 0;

	//LCL_INFO info;
	//vec3d periodic_vec[27];
	//pLCL->GetCellInfo(&info, periodic_vec);
	const vec3d* periodic_vec = m_periodic_vec;

	const int cell_wx = m_cell_x;
	const int cell_wy = m_cell_y;
	const int cell_wz = m_cell_z;
	const int* first_in_cell = m_first_in_cell;
	const int* inext = m_inext;


	const int cell_wxwy = cell_wx * cell_wy;
	const int cell_wxwywz = cell_wxwy * cell_wz;


	{
		const int icell_i = I3DtoI1D(i3d_cell_i);

#ifdef USE_I3D_CELL
		const int& icell_z = i3d_cell_i[2];
#else
		const int icell_z = ic / cell_wxwy;
#endif
		int nz[3] = { 0, 0, 0 };
		int tz[3] = { -cell_wxwy, 0, cell_wxwy };

		if (icell_z == 0) {
			nz[0] = 18;	//2 * (3 * 3)
			tz[0] += cell_wxwywz;
		}
		if (icell_z == cell_wz - 1) {
			nz[2] = 9;	//1 * (3 * 3)
			tz[2] -= cell_wxwywz;
		}

#ifdef USE_I3D_CELL
		const int& icell_y = i3d_cell_i[ 1];
#else
		const int icell_y = (ic / cell_wx) % cell_wy;
#endif
		int ny[3] = { 0, 0, 0 };
		int ty[3] = { -cell_wx, 0, cell_wx };
		if (icell_y == 0) {
			ny[0] = 6;	//2 * 3
			ty[0] += cell_wxwy;
		}
		if (icell_y == cell_wy - 1) {
			ny[2] = 3;	//1 * 3
			ty[2] -= cell_wxwy;
		}

#ifdef USE_I3D_CELL
		const int& icell_x = i3d_cell_i[0];
#else
		const int icell_x = ic % cell_wx;
#endif
		int nx[3] = { 0, 0, 0 };
		int tx[3] = { -1, 0, 1 };
		if (icell_x == 0) {
			nx[0] = 2;
			tx[0] = (cell_wx - 1);
		}
		if (icell_x == cell_wx - 1) {
			nx[2] = 1;
			tx[2] = (1 - cell_wx);
		}

		

		// loop for adjoin cell and same cell//
		//int num_t = 0;
		for (int iz = 0; iz < 3; iz++) {
			for (int iy = 0; iy < 3; iy++) {
				for (int ix = 0; ix < 3; ix++) {

					const int nxyz = nx[ix] + ny[iy] + nz[iz];
					const int target_cell = icell_i + tx[ix] + ty[iy] + tz[iz];


					//if(nxyz){//異なるセル//(単にperiodic境界を跨がない判定になってしまっている)
					//if(target_cell != ic){//異なるセル//(unit cellの際にすべてが同一セルになってしまう)
					if (((ix & (iy & iz)) & 0x1) == 0) {//(unit cellでも動く)


						const vec3d rb_i = r_i - *(periodic_vec + nxyz);
						for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]) {
							const vec3d dd = rb_i - r[j];

							const double r2 = dd * dd;

							// judge the relative distance //
							if (r2 <= CUTOFF_SQR) {
								if (bvector_size <= num_bonds) {
									return -1;
								}
								
								// add to bonding array //
								BVECTOR& bv_i = bvector[num_bonds];

								bv_i.pairIndex = j;
								bv_i.rij = dd;
								bv_i.dr = sqrt(r2);

								// increment of index //
								num_bonds++;

							}
						}


					} else {//同じセル//
						const vec3d rb_i = r_i;
						for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]) {

							if (ignore_id == j) {
								//丁度ここまでで13/27セルと同じセル中の前半部分が終了//
								twobody_endpoint = num_bonds;	//end point for two-body force of i-th particle's bond//
																//worker.Intermidiate();

							} else {

								const vec3d dd = rb_i - r[j];
								const double r2 = dd * dd;

								// judge the relative distance //
								if (r2 <= CUTOFF_SQR) {
									if (bvector_size <= num_bonds) {
										return -1;
									}

									// add to bonding array //
									BVECTOR& bv_i = bvector[num_bonds];

									bv_i.pairIndex = j;
									bv_i.rij = dd;
									bv_i.dr = sqrt(r2);

									// increment of index //
									num_bonds++;

								}
							}
						}
					}


				}
			}
		}//iz//

		*half_point = twobody_endpoint;


	}



	return num_bonds;


}


/*
LinkedListの指定のcell領域の中に入っている粒子のindexを取得
*/
int LCL::GetIndexInCells(int cell_id, int* indexes)
{
	int num = 0;
	for (int j = m_first_in_cell[cell_id]; j != -1; j = m_inext[j]) {
		indexes[num] = j;
		num++;
	}
	
	return num;
}

