
////////////////////////////////////////////////////////
//
//  AFS potential//
//
/////////////////////////////////////////////////////////


#ifndef __afsiv7_h__
#define __afsiv7_h__

#pragma once


#ifdef _OPENMP
#include <omp.h>
#endif	//_OPENMP
#include "myomptune.h"

#include "vec3.h"
#include "mat33.h"
#include "IPotential.h"
#include "LinkedCellList.h"
#include "atomarray.h"
//#include "debugprint.h"


	
class AFSIv7 : public IPotential {
private:

	LCL* m_pLCL;

	static const int MAX_NUM_BOND = 24;		// the avarage of bond number for each atom.

	struct BVECTOR{
		int pairIndex;	// index j which is pair of i //
		vec3d rij;		// normalized relative vector //
		double dr;
	};

	
	int m_bufsz_atom;
	int m_bufsz_bond;

	BVECTOR* m_bvector;	//omp:shared//
	BVEC_REGION* m_bvecStartEnd;
	

	double* m_Jij;
	mat33d m_Fr_m; //F x r		//omp:gatherd//
		
	void ReallocAtom(int maxcount);
	void ReallocBond(int maxbond);

	void Reset(int count);

	void ForceByBondList(int count, const BVECTOR* bvector, const BVEC_REGION* bvecStartEnd, const int* knd, vec3d* __restrict a_force, vec3d* __restrict sh_force, double* __restrict a_pot, mat33d* __restrict a_Fr_m);

	void SubForceAtomic(int j, int i_start, int i_end, const vec3d& force_j, vec3d* a_force, vec3d* sh_force);
	void AddFr(const vec3d& force_ij, const vec3d& r_ij, mat33d* a_Fr_m);
	
	//special tuning//
	void AddPressure(const int i, const int j, const int i_start, const int i_end, const vec3d& force_ij, const vec3d& r_ij);

	int* m_knd;			//omp:shared//
	vec3d* m_r;			//omp:shared//
	vec3d* m_p;			//omp:shared//
	vec3d* m_f;			//omp:shared|gathered//
	double* m_mass;		//omp:shared//
	double* m_pot;		//omp:shared|gathered//
	
	vec3d* sh_aforce;	//omp:gatherd//


	// AFS固有の変数 ////////////////////////////////////////////////////
	double* m_rho;	//周辺電子密度.
	double* m_PV;

	//ポテンシャル固有のパラメーター.
	// m_afs_ で始まる.
			
//	double m_afs_cc;	//2-bodyカットオフ長の二乗.
//	double m_afs_dd;	//3-bodyカットオフ長の二乗.

//#define AFSIv7_DF_NUMPARAMS		(3)		
	static const int DF_NUMPARAMS = 3;

	union{
		double m_afs_params[14];
		struct {				//ここの順番がAFS_PARAMS配列と一致すること.
			
			

			double m_param_HeW_a0;
			double m_param_HeW_b0;
			double m_param_HeW_c0;
			
			double m_param_WW_a0;
			double m_param_Rho_B;
			double m_param_Rho_c;
			double m_param_Rho_d;
			
			double m_param_HeHe_a0;
			double m_param_HeHe_b0;
			double m_param_HeHe_c0;
			
			double m_param_Rho_short;
			double m_param_Rho_long;
			double m_param_WW_short;
			double m_param_WW_long;
			//double m_param_Rho_A;
			
			double m_param_WW_z0;
			double m_param_HeHe_z0;
			double m_param_HeW_z0;

/*
			double m_param_Rho_n2;
			double m_param_Rho_B2;
			double m_param_Rho_l2;
			double m_param_WW_k0;
			double m_param_Rho_n;
			double m_param_WW_a1;
			double m_param_WW_c0;
			double m_param_WW_b0;
			double m_param_WW_A1;
*/			

//			double m_param_HeHe_b0;
//			double m_param_HeHe_c0;
//			double m_param_HeW_b0;
//			double m_param_HeW_c0;
/*			double m_param_phiW_g0;
			double m_param_phiW_g1;
			double m_param_phiW_x0;
			double m_param_phiW_y0;
			double m_param_phiW_y1;
			double m_param_phiW_f1;	*/
//			double m_a_phi;
			//double m_co_phi;
/*			double m_afs_c;		//2-bodyカットオフ長.
			double m_afs_d;		//3-bodyカットオフ長.
			double m_afs_c0;
			double m_afs_c1;
			double m_afs_c2;
			double m_afs_A;
			double m_afs_beta;
			double m_afs_B;		//Ackland core 補正
			double m_afs_alpha;	//Ackland core 補正
			double m_afs_b0;	//Ackland core 補正*/
//			double m_param_HeHe_z1;
//			double m_param_HeHe_a1;
//			double m_param_HeHe_b0;

		};
	};
	
//	void CalcRho(int i, double r2);
	double CalcRho2(int i, double r);
	int Force2body_WW(double r, double *pV, double *pdV);
	int Force2body_HeW(double r, double *pV, double *pdV);
	int Force2body_HeHe(double r, double *pV, double *pdV);
	int ForceMultibody_Rho(double r, double *pdphi);

	
//	void SingleAtomEnergy(int count, double *a_pot);
//	double m_E_W;
//	double m_E_He;

public:


	AFSIv7(int maxcount);
	virtual ~AFSIv7();
	
	void SetMaxNumParticles(int maxcount){
		ReallocAtom(maxcount);
		m_pLCL->SetMaxNumParticles(maxcount);
	};

	void SetParticlePointers(AtomArray* atoms);
	void SetLinkedCellList(LCL* pLCL){m_pLCL = pLCL;};
	void SetBoxsize(double x, double y, double z){m_pLCL->SetBoxsize(x, y, z);};
	//void SetBoxAxis(const vec3d& a, const vec3d& b, const vec3d& c){m_pLCL->SetBoxAxis(a, b, c);};
	void SetBoxAxis(const mat33d& Am){m_pLCL->SetBoxAxis(Am);};

	void Force(int count, int noloop_count);

	void GetFr(mat33d* a_Fr_m);
	
	double GetCutoffLength();
	double GetForceRange();


	void GetParams(double* params);
	void SetParams(const double* params);
	int GetNumParams(){return DF_NUMPARAMS;};	//Downfoldingで調整できるパラメーターの数を返す.

	void GetInfo(int mode, void* params);

};




#endif	//!__afsiv7_h__

