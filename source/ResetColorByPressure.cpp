

#include "ResetColorByPressure.h"

#include "IPotential.h"


#ifndef BYTE
typedef unsigned char uint8_t;
#endif

extern int g_periodic;
extern double g_visual_max_pressure;

//スペクトル色を返す
void SpectrumColor(double value, unsigned char *r, unsigned char* g, unsigned char * b){
//	static BYTE tr[11] = {0, 12, 90,160,200,225,240,250,250,250,250};
//	static BYTE tg[11] = {0,  0,  0,  0,  0, 40, 90,140,190,240,250};
//	static BYTE tb[11] = {0,120,150,150,120, 10,  0,  0, 10,120,250};
	static unsigned char tr[11] = {0, 38, 76,  0,  0,  0,  0, 60,120,180,255};
	static unsigned char tg[11] = {0,  0,  0,  0,140,180,250,180,120, 60,  0};
	static unsigned char tb[11] = {128,106,211,230,120, 60,  0,  0,  0,  0,  0};

	if(value < 0.0){
		*r = *g = *b = 0;
	}

	double pos;
	double tt = modf(value * 10.0, &pos);
	int ipos = (int)(pos);
	if(ipos >= 10){
		*r = tr[10];
		*g = tg[10];
		*b = tb[10];
	}else if(ipos <0){
		*r = tr[0];
		*g = tg[0];
		*b = tb[0];
	}else{
		*r = tr[ipos] + (unsigned char)(tt * (tr[ipos+1]-tr[ipos]));
		*g = tg[ipos] + (unsigned char)(tt * (tg[ipos+1]-tg[ipos]));
		*b = tb[ipos] + (unsigned char)(tt * (tb[ipos+1]-tb[ipos]));
	}

}


/*
void glips_ResetColorByPressure(ATOMS_DATA *dat, BOND_INFO* bond){
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




	AtomArray* atoms = new AtomArray(dat->pcnt);
	vec3d* r_d = atoms ->r;
	int i;
	for(i=0;i<dat->pcnt;i++){
		atoms->Add(dat->knd[i], 1.0, dat->r[i] - boxorg);
	}

	IPotential* potential = CreatePotential(TYPE_POT_AFSI, 0);
	double cutoff = potential->GetCutoffLength();
	LCL* pLCL = new LCL( cutoff );

	potential->SetLinkedCellList(pLCL);
	potential->SetParticlePointers(atoms);	//粒子のメモリと関連付け.
	potential->SetMaxNumParticles(atoms->GetBufferSize());
	
	
	
	double nonortho_term = fabs(boxaxis.m12) + fabs(boxaxis.m13)
					+ fabs(boxaxis.m21) + fabs(boxaxis.m23) 
					+ fabs(boxaxis.m31) + fabs(boxaxis.m32); 
	if(nonortho_term > 0.1e-6){
		mat33d Am = boxaxis;
		potential->SetBoxAxis(Am); 
	}else{
		potential->SetBoxsize(boxaxis.m11, boxaxis.m22, boxaxis.m33);
	}
	
	potential->SetParams(&cutoff);
	pLCL->Create(atoms->count, atoms->r, atoms->knd);
	potential->Force(dat->pcnt, 0);



	double* PV;
	potential->GetInfo(3, &PV);

	if(dat->p){
		const vec3f* pp = dat->p; 
		for(int i = 0 ; i < dat->pcnt; i++){
			PV[i] += (double)(pp[i] * pp[i]);
		}
	}

	if(bond->sz_coords < dat->pcnt){
		bond->sz_coords = dat->pcnt;
		delete [] (bond->coords);
		bond->coords = new int[(bond->sz_coords)];
	}
	unsigned char* color_buf = (unsigned char*)(bond->coords);

	
	const double maxval = 10.0;	//magic number//

	

	
	for(int i = 0 ; i < dat->pcnt; i++){
		
		SpectrumColor(PV[i]/maxval, color_buf + i*4, color_buf + i*4+1, color_buf + i*4+2);//規格化済み//
		color_buf[i*4 +3] = (unsigned char)(96.0 * PV[i]/maxval);//規格化済み//
		
	//	valueToRGBA(field[k], maxvalue, tex);
	//	*(tex+3) = (BYTE)(96.0 * (T)(field[k])/maxvalue);//規格化済み//
	//	if(*(tex+3) > 48){ *(tex+3) = 48;}
		

		
		if(color_buf[i*4 +3] < 8){ color_buf[i*4 +3] = 0;}
	
	}


		


	delete atoms;
	delete potential;
	delete pLCL;

}
*/

void glips_ResetColorByPx(ATOMS_DATA *dat, BOND_INFO* bond){
	//ボンドを再計算


	
	if(bond->sz_coords < dat->pcnt){
		bond->sz_coords = dat->pcnt;
		delete [] (bond->coords);
		bond->coords = new int[(bond->sz_coords)];
	}
	unsigned char* color_buf = (unsigned char*)(bond->coords);

	float* PV = new float[dat->pcnt];
	
	if(dat->p){
		const vec3f* pp = dat->p; 
		for(int i = 0 ; i < dat->pcnt; i++){
			PV[i] = pp[i].x;
		}
	}

	
	const double maxval = g_visual_max_pressure;	//magic number//
	const double minval = 0;	//magic number//

	
	for(int i = 0 ; i < dat->pcnt; i++){
		
		SpectrumColor((PV[i]-minval)/(maxval-minval), color_buf + i*4, color_buf + i*4+1, color_buf + i*4+2);//規格化済み//
		color_buf[i*4 + 3] = 96;
		
		
		//if(color_buf[i*4 +3] < 8){ color_buf[i*4 +3] = 0;}
	
	}


		


	delete [] PV;


}
