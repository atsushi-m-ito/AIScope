

#include "ResetBond.h"
#include "renew.h"

#include "LinkedCellList.h"



extern int g_periodic;



void glips_ResetBond(ATOMS_DATA *dat, BOND_INFO* bond, double cutoff){
	//ボンドを再計算


	mat33f boxaxis = dat->boxaxis;
	vec3f boxorg = dat->boxorg;
		

	if (g_periodic == 0){
		
		//非周期境界のボックスの定義//
		vec3f rmin = {0.0f, 0.0f, 0.0f};
		vec3f rmax = {0.0f, 0.0f, 0.0f};
		for(int i = 0; i < dat->pcnt; i++){
			if( rmin.x  > dat->r[i].x  ){ rmin.x = dat->r[i].x;}
			if( rmin.y  > dat->r[i].y ){ rmin.y = dat->r[i].y;}
			if( rmin.z  > dat->r[i].z ){ rmin.z = dat->r[i].z;}

			if( rmax.x  < dat->r[i].x ){ rmax.x = dat->r[i].x;}
			if( rmax.y  < dat->r[i].y ){ rmax.y = dat->r[i].y;}
			if( rmax.z  < dat->r[i].z ){ rmax.z = dat->r[i].z;}
		}

		boxorg.Set(rmin.x, rmin.y, rmin.z);
		boxaxis.Set((rmax.x - rmin.x), 0.0f, 0.0f, 0.0f, (rmax.y - rmin.y),0.0f, 0.0f, 0.0f, (rmax.z - rmin.z));

	}



	vec3d* r_d = new vec3d[dat->pcnt];
	
	for(int i=0;i<dat->pcnt;i++){
		r_d[i] = dat->r[i] - boxorg;
	}

	LCL pLCL( cutoff );
	
	double nonortho_term = fabs(boxaxis.m12) + fabs(boxaxis.m13)
					+ fabs(boxaxis.m21) + fabs(boxaxis.m23) 
					+ fabs(boxaxis.m31) + fabs(boxaxis.m32); 
	if (nonortho_term > 0.1e-6) {

		pLCL.SetBoxAxis(mat33d(boxaxis.m11, boxaxis.m12, boxaxis.m13,
			boxaxis.m21, boxaxis.m22, boxaxis.m23,
			boxaxis.m31, boxaxis.m32, boxaxis.m33));
	} else {
		pLCL.SetBoxsize(boxaxis.m11, boxaxis.m22, boxaxis.m33);
	}
	
	pLCL.Create(dat->pcnt, r_d, NULL);
	
	
	
	int bond_buf_sz = dat->pcnt * 4;

	if(bond->sz_bvector < bond_buf_sz){
		bond->sz_bvector  = bond_buf_sz;		
		renew<VECTOR_IJ>(&(bond->bvector),bond->sz_bvector);
	}

	if (bond->sz_coords < dat->pcnt) {
		bond->sz_coords = dat->pcnt;
		renew<int>(&(bond->coords), bond->sz_coords);
	}

	const int bvector_size = 200;
	BVECTOR bvector[bvector_size];
	const double CUTOFF2 = cutoff*cutoff;
	int num_bonds = 0;
	for (int i = 0, i_end = dat->pcnt; i < i_end; ++i) {
		int half_point;
		int num = pLCL.GetNeighborList(i, r_d, CUTOFF2, bvector, &half_point, bvector_size);
		
		if (bond->sz_bvector <= num_bonds + num) {
			bond->sz_bvector *= 2;
			renew_preserve<VECTOR_IJ>(&(bond->bvector), num_bonds, bond->sz_bvector);
		}

		//ボンドの追加//
		for (int k = 0; k < half_point; ++k) {		

			bond->bvector[num_bonds + k].i = i;
			bond->bvector[num_bonds + k].j = bvector[k].pairIndex;
			bond->bvector[num_bonds + k].v = -bvector[k].rij;			
		}
		num_bonds += half_point;
		
		//配位数//
		bond->coords[i] = num;
	}
	
	bond->count = num_bonds;
		

	if(g_periodic){
		for(int i=0;i<dat->pcnt;i++){
			dat->r[i] = (vec3f)(r_d[i]) + boxorg;
		}
	}

	delete [] r_d;

}
