#include "CellPartition.h"
#include <stdio.h>


CellPartition::CellPartition(double cellwidth) :
	m_cell_count(0),
	m_cell_x(0),
	m_cell_y(0),
	m_cell_z(0),
	m_necessary_cellwidth(cellwidth),	//セルの初期値
	m_is_orthogonal(1)		//直交座標ならtrue
{


}

CellPartition::~CellPartition()
{
}


void CellPartition::CalcNumCells(double boxsize_x, double boxsize_y, double boxsize_z, double necessary_cellwidth, int* num_cell_x, int* num_cell_y, int* num_cell_z) {

	int lx = (int)floor(boxsize_x / necessary_cellwidth);
	int ly = (int)floor(boxsize_y / necessary_cellwidth);
	int lz = (int)floor(boxsize_z / necessary_cellwidth);
	if (lx <= 0) { lx = 1; }
	if (ly <= 0) { ly = 1; }
	if (lz <= 0) { lz = 1; }

	*num_cell_x = lx;
	*num_cell_y = ly;
	*num_cell_z = lz;

}

void CellPartition::SetBoxsize(double boxsize_x, double boxsize_y, double boxsize_z) {

	SetBoxsizeWithMargin(boxsize_x, boxsize_y, boxsize_z, 0.0, 0, 0, 0, NULL);
}


void CellPartition::SetBoxsizeWithMargin(double boxsize_x, double boxsize_y, double boxsize_z, double margin, int is_margin_x, int is_margin_y, int is_margin_z, int* margin_xyz) {

	m_is_orthogonal = 1;	//直交座標ならtrue

#if 1
	int lx, ly, lz;
	CalcNumCells(boxsize_x, boxsize_y, boxsize_z, m_necessary_cellwidth, &lx, &ly, &lz);
#else
	int lx = (int)floor(boxsize_x / m_necessary_cellwidth);
	int ly = (int)floor(boxsize_y / m_necessary_cellwidth);
	int lz = (int)floor(boxsize_z / m_necessary_cellwidth);
	if (lx <= 0) { lx = 1; }
	if (ly <= 0) { ly = 1; }
	if (lz <= 0) { lz = 1; }
#endif

	//のりしろを取る場合の処理//
	int margin_x = 0;
	if (is_margin_x) {
		double width = boxsize_x / (double)lx;
		margin_x = (int)ceil(margin / width);
		lx += margin_x * 2;
		boxsize_x = width * (double)lx;
	}

	int margin_y = 0;
	if (is_margin_y) {
		double width = boxsize_y / (double)ly;
		margin_y = (int)ceil(margin / width);
		ly += margin_y * 2;
		boxsize_y = width * (double)ly;
	}

	int margin_z = 0;
	if (is_margin_z) {
		double width = boxsize_z / (double)lz;
		margin_z = (int)ceil(margin / width);
		lz += margin_z * 2;
		boxsize_z = width * (double)lz;
	}

	if (margin_xyz) {
		margin_xyz[0] = margin_x;
		margin_xyz[1] = margin_y;
		margin_xyz[2] = margin_z;
	}


	//情報のセット//
	m_box.Set(boxsize_x, boxsize_y, boxsize_z);
	m_boxi.Set(1.0 / boxsize_x, 1.0 / boxsize_y, 1.0 / boxsize_z);
	m_Am.Set(m_box.x, 0.0, 0.0, 0.0, m_box.y, 0.0, 0.0, 0.0, m_box.z);
	ResetCell(lx, ly, lz);
	RefreshPeriodicVector();

}



////応力制御などによる非直交の辺によるsim.boxの設定
void CellPartition::CalcNumCells(const vec3d& a, const vec3d& b, const vec3d& c, double necessary_cellwidth, int* num_cell_x, int* num_cell_y, int* num_cell_z, double* vertical_abc) {

	//体積割る面積をして中心から各面への垂線の高さ*2(vertical_a,b,c)を求める//
	vec3d v = Cross(b, c);
	const double volume = a * v;
	const double surface_b_c = Abs(v);
	v = Cross(c, a);
	const double surface_c_a = Abs(v);
	v = Cross(a, b);
	const double surface_a_b = Abs(v);

	const double vertical_a = volume / surface_b_c;
	const double vertical_b = volume / surface_c_a;
	const double vertical_c = volume / surface_a_b;

	//local ---------	
	int lx = (int)floor(vertical_a / necessary_cellwidth);
	int ly = (int)floor(vertical_b / necessary_cellwidth);
	int lz = (int)floor(vertical_c / necessary_cellwidth);
	if (lx <= 0) { lx = 1; }
	if (ly <= 0) { ly = 1; }
	if (lz <= 0) { lz = 1; }


	*num_cell_x = lx;
	*num_cell_y = ly;
	*num_cell_z = lz;

	if (vertical_abc) {
		vertical_abc[0] = vertical_a;
		vertical_abc[1] = vertical_b;
		vertical_abc[2] = vertical_c;
	}

}


////応力制御などによる非直交の辺によるsim.boxの設定
void CellPartition::SetBoxAxisWithMargin(const vec3d& a, const vec3d& b, const vec3d& c, double margin, int is_margin_x, int is_margin_y, int is_margin_z, int* margin_xyz) {

	m_is_orthogonal = 0;	//直交座標ならtrue

	m_Am.a = a;
	m_Am.b = b;
	m_Am.c = c;

#if 1
	int lx, ly, lz;
	double vertical_abc[3];
	CalcNumCells(a, b, c, m_necessary_cellwidth, &lx, &ly, &lz, vertical_abc);

	const double vertical_a = vertical_abc[0];
	const double vertical_b = vertical_abc[1];
	const double vertical_c = vertical_abc[2];

#else
	//体積割る面積をして中心から各面への垂線の高さ*2(vertical_a,b,c)を求める//
	vec3d v = Cross(b, c);
	const double volume = a * v;
	const double surface_b_c = Abs(v);
	v = Cross(c, a);
	const double surface_c_a = Abs(v);
	v = Cross(a, b);
	const double surface_a_b = Abs(v);

	const double vertical_a = volume / surface_b_c;
	const double vertical_b = volume / surface_c_a;
	const double vertical_c = volume / surface_a_b;

	//local ---------	
	int lx = (int)floor(vertical_a / m_necessary_cellwidth);
	int ly = (int)floor(vertical_b / m_necessary_cellwidth);
	int lz = (int)floor(vertical_c / m_necessary_cellwidth);
	if (lx <= 0) { lx = 1; }
	if (ly <= 0) { ly = 1; }
	if (lz <= 0) { lz = 1; }
#endif

	//のりしろを取る場合の処理//
	int margin_x = 0;
	if (is_margin_x) {
		double width = vertical_a / (double)lx;
		margin_x = (int)ceil(margin / width);

		m_Am.a *= ((double)(lx + margin_x * 2) / (double)lx);
		lx += margin_x * 2;
	}

	int margin_y = 0;
	if (is_margin_y) {
		double width = vertical_b / (double)ly;
		margin_y = (int)ceil(margin / width);

		m_Am.b *= ((double)(ly + margin_y * 2) / (double)ly);
		ly += margin_y * 2;

	}

	int margin_z = 0;
	if (is_margin_z) {
		double width = vertical_c / (double)lz;
		margin_z = (int)ceil(margin / width);

		m_Am.c *= ((double)(lz + margin_z * 2) / (double)lz);
		lz += margin_z * 2;

	}

	if (margin_xyz) {
		margin_xyz[0] = margin_x;
		margin_xyz[1] = margin_y;
		margin_xyz[2] = margin_z;
	}



	//情報のセット//
	m_box.Set(m_Am.a.x, m_Am.b.y, m_Am.c.z);
	m_boxi.Set(1.0 / m_box.x, 1.0 / m_box.y, 1.0 / m_box.z);
	ResetCell(lx, ly, lz);
	RefreshPeriodicVector();

}


////応力制御などによる非直交の辺によるsim.boxの設定
void CellPartition::SetBoxAxis(const mat33d& Am) {
	SetBoxAxisWithMargin(Am.a, Am.b, Am.c, 0.0, 0, 0, 0, NULL);

}


void CellPartition::SetBoxAxis(const vec3d& a, const vec3d& b, const vec3d& c) {

	SetBoxAxisWithMargin(a, b, c, 0.0, 0, 0, 0, NULL);
}


int CellPartition::ResetCell(int a_mcx, int a_mcy, int a_mcz) {
	//ボックスサイズをチェックして, m_first_in_cellを再確保する.
	//確保しなかった場合は0を返す.
	//バッファは確保せずcellの個数だけ変更した場合は2を返す.


	//cell数が0は禁止
	if (a_mcx <= 0) { a_mcx = 1; }
	if (a_mcy <= 0) { a_mcy = 1; }
	if (a_mcz <= 0) { a_mcz = 1; }

	//セル情報を更新するかのチェック(trueなら更新しない)
	if (m_cell_x == a_mcx) {
		if (m_cell_y == a_mcy) {
			if (m_cell_z == a_mcz) {
				return 0;
			}
		}
	}


	m_cell_x = a_mcx;
	m_cell_y = a_mcy;
	m_cell_z = a_mcz;
	m_cell_count = m_cell_x * m_cell_y * m_cell_z;


	return m_cell_count;

}




void CellPartition::RefreshPeriodicVector() {

	//周期境界を跨いだ時に粒子間の相対ベクトルから差っ引くベクトルを準備//
	if (m_is_orthogonal) {	//直交系
		int index = 0;
		for (int iz = 0; iz < 3; iz++) {
			for (int iy = 0; iy < 3; iy++) {
				for (int ix = 0; ix < 3; ix++) {
					switch (ix) {
					case 0: m_periodic_vec[index].x = 0.0; break;
					case 1: m_periodic_vec[index].x = m_box.x; break;
					case 2: m_periodic_vec[index].x = -m_box.x; break;
					}
					switch (iy) {
					case 0: m_periodic_vec[index].y = 0.0; break;
					case 1: m_periodic_vec[index].y = m_box.y; break;
					case 2: m_periodic_vec[index].y = -m_box.y; break;
					}
					switch (iz) {
					case 0: m_periodic_vec[index].z = 0.0; break;
					case 1: m_periodic_vec[index].z = m_box.z; break;
					case 2: m_periodic_vec[index].z = -m_box.z; break;
					}
					index++;
				}
			}
		}
	} else {				//非直交系
		int index = 0;
		vec3d zero = { 0.0,0.0,0.0 };
		for (int iz = 0; iz < 3; iz++) {
			for (int iy = 0; iy < 3; iy++) {
				for (int ix = 0; ix < 3; ix++) {
					switch (ix) {
					case 0: m_periodic_vec[index] = zero; break;
					case 1: m_periodic_vec[index] = m_Am.a; break;
					case 2: m_periodic_vec[index] = -m_Am.a; break;
					}
					switch (iy) {
						//case 0: m_periodic_vec[index].y = 0.0; break;
					case 1: m_periodic_vec[index] += m_Am.b; break;
					case 2: m_periodic_vec[index] -= m_Am.b; break;
					}
					switch (iz) {
						//case 0: m_periodic_vec[index].z = 0.0; break;
					case 1: m_periodic_vec[index] += m_Am.c; break;
					case 2: m_periodic_vec[index] -= m_Am.c; break;
					}
					index++;
				}
			}
		}

	}


}


void CellPartition::Split(int istart, int count, vec3d* __restrict r, int* __restrict icell) {

	const int iend = istart + count;

	if (m_is_orthogonal) {
		//直交系の場合////////////////////////////////////////////////////////


		const double boxix = m_boxi.x;
		const double boxiy = m_boxi.y;
		const double boxiz = m_boxi.z;


		const int cell_x = m_cell_x;
		const int cell_y = m_cell_y;
		//const int cell_z = m_cell_z;
		const double d_cell_x = (double)m_cell_x;
		const double d_cell_y = (double)m_cell_y;
		const double d_cell_z = (double)m_cell_z;


#pragma omp parallel for
		for (int i = istart; i < iend; i++) {

			double cx, cy, cz;

			//double xi = r[i].x * m_boxi.x;
			double xi = r[i].x * boxix;
			//xi -= floor(xi + 1.0) - 1.0;
			xi = modf(xi + 1.0, &cx);
			//xi = modf(xi+1.0, NULL);
			//		m_cycle_x[i] += (int)cx;
			//		m_cycle_x[i] --;
			r[i].x = xi * m_box.x;

			//double yi = r[i].y * m_boxi.y;
			double yi = r[i].y * boxiy;
			//yi -= floor(yi + 1.0) - 1.0;
			yi = modf(yi + 1.0, &cy);
			//		m_cycle_y[i] += (int)cy;
			//		m_cycle_y[i] --;
			r[i].y = yi * m_box.y;

			//double zi = r[i].z * m_boxi.z;
			double zi = r[i].z * boxiz;
			//zi -= floor(zi + 1.0) - 1.0;
			zi = modf(zi + 1.0, &cz);
			//		m_cycle_z[i] += (int)cz;
			//		m_cycle_z[i] --;
			r[i].z = zi * m_box.z;

			//	m_icell[i] = (int)floor(xi * (double)(m_cell_x)) + ( (int)floor(yi * (double)(m_cell_y)) + (int)floor(zi * (double)(m_cell_z))*m_cell_y )*m_cell_x;
			icell[i] = ((int)floor(xi * d_cell_x)) + ((int)floor(yi * d_cell_y) + ((int)floor(zi * d_cell_z))*cell_y)*cell_x;

#if 0
			if (re_icell[i] < 0) {
				printf("error: x=%lg, y=%lg, z=%lg\n", r[i].x, r[i].y, r[i].z);
				printf("error: xi=%lg, yi=%lg, zi=%lg\n", xi, yi, zi);
				printf("error: i_xi=%d, i_yi=%d, i_zi=%d\n", (int)floor(xi * d_cell_x), (int)floor(yi * d_cell_y), (int)floor(zi * d_cell_z));
			}
#endif

		}

		/*
		const double boxx = m_box.x;
		const double boxy = m_box.y;
		const double boxz = m_box.z;

		const double boxix = m_boxi.x;
		const double boxiy = m_boxi.y;
		const double boxiz = m_boxi.z;

		const int cell_x = m_cell_x;
		const int cell_y = m_cell_y;
		//const int cell_z = m_cell_z;
		const double d_cell_x = (double)m_cell_x;
		const double d_cell_y = (double)m_cell_y;
		const double d_cell_z = (double)m_cell_z;

		const int cell_xm1 = m_cell_x - 1;
		const int cell_ym1 = m_cell_y - 1;
		const int cell_zm1 = m_cell_z - 1;

		#pragma omp parallel for
		for(int i = istart; i < iend ; i++){

		double cx,cy,cz;


		double xi = r[i].x * boxix;
		xi = modf(xi, &cx);
		int idx;
		if(xi < 0.0){
		idx = (int)(xi * d_cell_x) + cell_xm1;
		xi += 1.0;
		}else{
		idx = (int)(xi * d_cell_x);
		}
		r[i].x = xi * boxx;

		double yi = r[i].y * boxiy;
		yi = modf(yi, &cy);
		int idy;
		if(yi < 0.0){
		idy = (int)(yi * d_cell_y) + cell_ym1;
		yi += 1.0;
		}else{
		idy = (int)(yi * d_cell_y);
		}
		r[i].y = yi * boxy;

		double zi = r[i].z * boxiz;
		zi = modf(zi, &cz);
		int idz;
		if(zi < 0.0){
		idz = (int)(zi * d_cell_z) + cell_zm1;
		zi += 1.0;
		}else{
		idz = (int)(zi * d_cell_z);
		}
		r[i].z = zi * boxz;

		re_icell[i] = idx + ( idy + idz*cell_y )*cell_x;

		}
		*/
		/*

		const double boxx = m_box.x;
		const double boxy = m_box.y;
		const double boxz = m_box.z;

		const double boxix = m_boxi.x;
		const double boxiy = m_boxi.y;
		const double boxiz = m_boxi.z;

		const int cell_x = m_cell_x;
		const int cell_y = m_cell_y;
		const double d_cell_x = ((double)m_cell_x)/boxx;
		const double d_cell_y = ((double)m_cell_y)/boxy;
		const double d_cell_z = ((double)m_cell_z)/boxz;

		#pragma omp parallel for
		for(int i = istart; i < iend ; i++){


		//double xi = fmod(r[i].x, boxx);
		//r[i].x = xi;
		double xi = r[i].x;
		if (xi < 0.0){ xi += boxx;}
		else if (xi >= boxx){ xi -= boxx;}
		r[i].x = xi;



		double yi = r[i].y;
		if (yi < 0.0){ yi += boxy;}
		else if (yi >= boxy){ yi -= boxy;}
		r[i].y = yi;

		double zi = r[i].z;
		if (zi < 0.0){ zi += boxz;}
		else if (zi >= boxz){ zi -= boxz;}
		r[i].z = zi;

		//	m_icell[i] = (int)floor(xi * (double)(m_cell_x)) + ( (int)floor(yi * (double)(m_cell_y)) + (int)floor(zi * (double)(m_cell_z))*m_cell_y )*m_cell_x;
		//	re_icell[i] = (int)floor(xi * d_cell_x) + ( (int)floor(yi * d_cell_y) + (int)floor(zi * d_cell_z)*cell_y )*cell_x;
		re_icell[i] = (int)(xi * d_cell_x) + ( (int)(yi * d_cell_y) + (int)(zi * d_cell_z)*cell_y )*cell_x;

		//if(re_icell[i] < 0 || re_icell[i] >= m_cell_count){
		//	re_icell[i] = re_icell[i];
		//}


		}
		*/
	} else {
		//非直交系の場合////////////////////////////////////////////////////////

		/*
		const mat33d Am = m_Am;
		const mat33d iAm = Inverse(m_Am);

		const int cell_x = m_cell_x;
		const int cell_y = m_cell_y;
		const double d_cell_x = (double)m_cell_x;
		const double d_cell_y = (double)m_cell_y;
		const double d_cell_z = (double)m_cell_z;

		#pragma omp parallel for
		for(int i = istart; i < iend ; i++){

		//			if (knd[i] < -1) continue;

		double cx,cy,cz;

		vec3d r_prim = iAm * r[i];

		r_prim.x = modf(r_prim.x+1.0, &cx);
		//r_prim.x -= floor(r_prim.x + 1.0) - 1.0;
		//		m_cycle_x[i] += (int)cx;
		//		m_cycle_x[i] --;

		r_prim.y = modf(r_prim.y+1.0, &cy);
		//r_prim.y -= floor(r_prim.y + 1.0) - 1.0;
		//		m_cycle_y[i] += (int)cy;
		//		m_cycle_y[i] --;

		r_prim.z = modf(r_prim.z+1.0, &cz);
		//r_prim.z -= floor(r_prim.z + 1.0) - 1.0;
		//		m_cycle_z[i] += (int)cz;
		//		m_cycle_z[i] --;

		r[i] = Am * r_prim;

		//m_icell[i] = (int)(r_prim.x * (double)(m_cell_x)) + ( (int)(r_prim.y * (double)(m_cell_y)) + (int)(r_prim.z * (double)(m_cell_z))*m_cell_y )*m_cell_x;
		re_icell[i] = (int)(r_prim.x * d_cell_x) + ( (int)(r_prim.y * d_cell_y) + (int)(r_prim.z * d_cell_z)*cell_y )*cell_x;

		}
		*/


		const mat33d Am = m_Am;
		const mat33d iAm = Inverse(m_Am);

		const int cell_x = m_cell_x;
		const int cell_y = m_cell_y;
		const double d_cell_x = (double)m_cell_x;
		const double d_cell_y = (double)m_cell_y;
		const double d_cell_z = (double)m_cell_z;

		const int cell_xm1 = m_cell_x - 1;
		const int cell_ym1 = m_cell_y - 1;
		const int cell_zm1 = m_cell_z - 1;

#pragma omp parallel for
		for (int i = istart; i < iend; i++) {

			//			if (knd[i] < -1) continue;

			double cx, cy, cz;

			vec3d r_prim = iAm * r[i];
			r_prim.x = modf(r_prim.x, &cx);
			int idx;
			if (r_prim.x < 0.0) {
				idx = (int)(r_prim.x * d_cell_x) + cell_xm1;
				r_prim.x += 1.0;
			} else {
				idx = (int)(r_prim.x * d_cell_x);
			}

			r_prim.y = modf(r_prim.y, &cy);
			int idy;
			if (r_prim.y < 0.0) {
				idy = (int)(r_prim.y * d_cell_y) + cell_ym1;
				r_prim.y += 1.0;
			} else {
				idy = (int)(r_prim.y * d_cell_y);
			}

			r_prim.z = modf(r_prim.z, &cz);
			int idz;
			if (r_prim.z < 0.0) {
				idz = (int)(r_prim.z * d_cell_z) + cell_zm1;
				r_prim.z += 1.0;
			} else {
				idz = (int)(r_prim.z * d_cell_z);
			}

			r[i] = Am * r_prim;
			icell[i] = idx + (idy + idz*cell_y)*cell_x;

		}

	}	//m_is_orthogonal//

}


void CellPartition::Split3D(int istart, int count, const vec3d* __restrict r, int* __restrict icell) {

	const int iend = istart + count;

	if (m_is_orthogonal) {
		//直交系の場合////////////////////////////////////////////////////////

		const double cell_inv_x = m_boxi.x * (double)m_cell_x;
		const double cell_inv_y = m_boxi.y * (double)m_cell_y;
		const double cell_inv_z = m_boxi.z * (double)m_cell_z;



#pragma omp parallel for
		for (int i = istart; i < iend; ++i) {

			icell[i * 3] = (int)floor(r[i].x * cell_inv_x); 
			icell[i * 3 + 1] = (int)floor(r[i].y * cell_inv_y);
			icell[i * 3 + 2] = (int)floor(r[i].z * cell_inv_z);
			
		}


	} else {
		//非直交系の場合////////////////////////////////////////////////////////
		

		mat33d Am_unit_cell = m_Am;
		Am_unit_cell.a /= (double)m_cell_x;
		Am_unit_cell.b /= (double)m_cell_y;
		Am_unit_cell.c /= (double)m_cell_z;
		const mat33d iAm = Inverse(Am_unit_cell);

#pragma omp parallel for
		for (int i = istart; i < iend; ++i) {
			
			vec3d r_prim = iAm * r[i];

			icell[i * 3] = (int)floor(r_prim.x);
			icell[i * 3 + 1] = (int)floor(r_prim.y);
			icell[i * 3 + 2] = (int)floor(r_prim.z);
			
		}

	}	//m_is_orthogonal//

}

void CellPartition::GetCellWidth(vec3d *width) {

	width->Set(m_box.x / (double)m_cell_x, m_box.y / (double)m_cell_y, m_box.z / (double)m_cell_z);

}

void CellPartition::GetCellCount(int* cell_x, int* cell_y, int* cell_z)
{
	*cell_x = m_cell_x;
	*cell_y = m_cell_y;
	*cell_z = m_cell_z;

}
