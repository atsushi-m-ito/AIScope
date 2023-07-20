////////////////////////////////////////////////////////
//
//  sample class of bond order potential for muli-body force  //
//
/////////////////////////////////////////////////////////


#include "AFSIv10.h"
#include "omptune.h"
#include "mod.h"

#include <math.h>
#include <stdio.h>



/*   note:
	AFSIv7_CUTOFF_SHORTの値によりBulk Modulusは大きく変化する
*/

static double WW_CUTOFF_SHORT = 3.235582;//3.27;
static double WW_CUTOFF_LONG = 4.274613;//0.8;

static double RHO_CUTOFF_SHORT = 3.151734;//2.969405;//
static double RHO_CUTOFF_LONG = 4.069256;//4.098425;//

static const double HeHe_CUTOFF_SHORT = 3.8;//3.27;
static const double HeHe_CUTOFF_RANGE = 0.5;//0.8;

static const double HeW_CUTOFF_SHORT = 3.5;//3.27;
static const double HeW_CUTOFF_RANGE = 0.5;//0.8;



static const double AFSIv7_CUTOFF_LONG = 4.3;//HeHe_CUTOFF_SHORT + HeHe_CUTOFF_RANGE;
static const double AFSIv7_CUTOFF_L2 = AFSIv7_CUTOFF_LONG * AFSIv7_CUTOFF_LONG;

//#define CUTOFF_5D

AFSIv7::AFSIv7(int maxcount) :
		m_param_HeW_z0(2131.1474378659768122975845832822),//(2131.1474378659768122975845832822)//
		m_param_HeW_a0(3.170542),//(3.3.170542)//
		m_param_HeW_b0(-0.984959),//(-0.0.984959)//
		m_param_HeW_c0(0.436908),//(0.436908)//

		m_param_HeHe_z0(57.598579401783157089123907656276),//=(2.0*2.0*CoulombCoef_eV_A)//
		m_param_HeHe_a0(3.887407),
		m_param_HeHe_b0(5.302379),
		m_param_HeHe_c0(-2.619318),


		m_param_WW_z0(74.0*74.0*CoulombCoef_eV_A),//(78852.455201041142055010629581442),//(355.530601 * 27.21138), //(10.0),	//(4.0),
		m_param_WW_a0(4.081955),//(4.172371),//
		m_param_Rho_B(13.856588),//(24.611181),//
		m_param_Rho_c(1.418559),//(1.107012),//
		m_param_Rho_d(4.904212),//(34.474691),//
//		m_param_Rho_A(0.0),//(-0.011274),//
		m_param_Rho_short(RHO_CUTOFF_SHORT),
		m_param_Rho_long(RHO_CUTOFF_LONG),

/*
		m_param_Rho_n(0.5),
		m_param_Rho_B2(0),
		m_param_Rho_n2(1.0),
		m_param_Rho_l2(3.634552),
		m_param_WW_short(WW_CUTOFF_SHORT),
		m_param_WW_long(WW_CUTOFF_LONG),
		m_param_WW_k0(0.0),
		m_param_WW_b0(0.0),
		m_param_WW_c0(0.0),
		m_param_WW_A1(0.0),//(-10.0),
		m_param_WW_a1(1.421571),//(1.0),
*/
		m_pLCL(NULL),
		m_bufsz_atom(0),
		m_bufsz_bond(0),
		m_bvecStartEnd(NULL),
		m_bvector(NULL),
		sh_aforce(NULL),
		m_rho(NULL),
		m_PV(NULL)
		{



	// allocate work array for openmp parallel calculation //
	if(maxcount > 0){
		ReallocAtom( maxcount);
	}
}

AFSIv7::~AFSIv7(){
	
	
	// delete work array for openmp parallel calculation //
	delete [] m_bvector;
	delete [] m_bvecStartEnd;
	
	delete [] m_rho;

	delete [] m_PV;

#ifdef _OPENMP

	delete [] sh_aforce;
//	delete [] sh_apot;
	
#endif	//_OPENMP


}

double AFSIv7::GetCutoffLength(){
	return AFSIv7_CUTOFF_LONG;
}

double AFSIv7::GetForceRange(){
	return GetCutoffLength();
}

// allocate work array for openmp parallel calculation //
/*
void AFSIv7::AllocateBuffer(int maxcount){

	//m_ijmax = maxcount * MAX_NUM_BOND;
	m_bvecStartEnd = new BVEC_REGION[maxcount];
	m_bvector = new BVECTOR[maxcount * MAX_NUM_BOND];
	
	
	m_rho = new double[maxcount];
	
#ifdef _OPENMP
	int num_threads = omp_get_max_threads();
	sh_aforce = new vec3d[maxcount];
//	sh_apot = new double[maxcount * num_threads];
	
#endif	//_OPENMP

}
*/


void AFSIv7::ReallocAtom(int maxcount){

	if(maxcount <= m_bufsz_atom){ return;}

	m_bufsz_atom = maxcount;

	if(m_rho) delete [] m_rho;
	m_rho = new double[maxcount];
	m_PV = new double[maxcount];

	if(m_bvecStartEnd) delete [] m_bvecStartEnd;
	m_bvecStartEnd = new BVEC_REGION[maxcount];

	const int maxbond = maxcount * MAX_NUM_BOND;
	ReallocBond(maxbond);
	
#ifdef _OPENMP
	if(sh_aforce) delete [] sh_aforce;
	sh_aforce = new vec3d[maxcount];
#endif	//_OPENMP

}


void AFSIv7::ReallocBond(int maxbond){

	if(maxbond <= m_bufsz_bond){ return;}
	
	m_bufsz_bond = maxbond;
	
	if(m_bvector) delete [] m_bvector;
	m_bvector = new BVECTOR[m_bufsz_bond];
}



void AFSIv7::SetParticlePointers(AtomArray* atoms){
	m_knd = atoms->knd;
	m_r = atoms->r;
	m_p = atoms->p;
	m_f = atoms->f;
	m_mass = atoms->mass;
	m_pot = atoms->pot;

}

// clear (zero fill) the openmp work arrays. //
inline void AFSIv7::Reset(int count){
	
#ifdef _OPENMP

	{//simd
		const int sz = (sizeof(vec3d) / sizeof(double)) * count; 
		double* __restrict re_af = (double*)sh_aforce;

		#pragma omp for
		for(int i = 0; i < sz; i++){
			re_af[i] = 0.0;
		}
	}

	

	// m_bvector and m_bvecStartEnd shold not be cleared //
	
#endif	// _OPENMP
	
	{//simd

		double* __restrict re_rho = m_rho;

		#pragma omp for
		for(int i = 0; i < count; i++){
			re_rho[i] = 0.0;
		}
	}

	{//simd

		double* __restrict re_PV = m_PV;

		#pragma omp for
		for(int i = 0; i < count; i++){
			re_PV[i] = 0.0;
		}
	}

}


void AFSIv7::Force(int count, int noloop_count){
	
	//check buffer//
	if(count > m_bufsz_atom){
		ReallocAtom( count * 2 );
	}

// Program is parallelized in this function. //
// Users should not consider openmp parallelizaton in main loop. //
#pragma omp parallel
{

DEBUG_TM_START;

	// reset work arrays //

	Reset(count);

DEBUG_TM_SET(0);



	// judge and create the bonding list (m_bvector and m_bvecStartEnd) //
	if(-1 == CreateBondList(0, count - noloop_count, m_r, AFSIv7_CUTOFF_L2, m_pLCL, m_bvector, m_bvecStartEnd, m_bufsz_bond)){
		printf("error: CreateBondList needs larger buffer.\n");
	} 
	

DEBUG_TM_SET(2);

	// set the array pointer for eash threads ==================== //
#ifndef _OPENMP
	vec3d* sh_force = m_f;
#endif


	// ==================== set the array pointer for eash threads //

	mat33d a_Fr_m;
	a_Fr_m.Clear();

	// calculation force and potential, which are written into openmp work array here.//
	ForceByBondList(count - noloop_count, m_bvector, m_bvecStartEnd, m_knd, m_f, sh_aforce, m_pot, &a_Fr_m);

DEBUG_TM_SET(3);

	//SingleAtomEnergy(count, a_pot);


	// marge the fore and potentail ========================== //


#ifdef _OPENMP
	// this barrier is necessary before marging force and potential ..
	#pragma omp barrier
DEBUG_TM_SET(4);


	{//simd
		vec3d* __restrict re_f = m_f;
		const vec3d* __restrict re_af = sh_aforce;

		#pragma omp for
		for(int i = 0; i < count; i++){
			re_f[i] += re_af[i];
		}
	}

	// marge stress tensor also //
	omptuneReductionSum(&a_Fr_m);
	#pragma omp master
	{
		m_Fr_m = a_Fr_m;
	}

#else	// no-openmp
	m_Fr_m = a_Fr_m;
#endif

	
DEBUG_TM_SET(5);
	
}// finish openmp parallelization //

}



// calculate force and potential //
void AFSIv7::ForceByBondList(int count, const BVECTOR* bvector, const BVEC_REGION* bvecStartEnd, const int* knd, vec3d* __restrict a_force, vec3d* __restrict sh_force, double* __restrict a_pot, mat33d* __restrict a_Fr_m){
	
	double* __restrict re_rho = m_rho;

#ifdef _OPENMP
	const int th = omp_get_thread_num();
	const int thread_count = omp_get_num_threads();
#else
	const int th = 0;
	const int thread_count = 1;
#endif
	const int i_start = (th * count)/thread_count;
	const int i_end = ((th + 1) * count)/thread_count;
	
	for (int i = i_start; i < i_end; i++){

	//loop of atom is parallelized by openmp //
	//#pragma omp for //schedule(static,1)	
	//for (int i = 0; i < count; i++){
		
		const int ki = knd[i];
		
		double coef3 = 0.0;

		// loop of j-th atom which forms i-j bond //
		const int twobody_endpoint = bvecStartEnd[i].twobodyendpoint;
		const int bv_idx_i_start = bvecStartEnd[i].startpoint;
		const int bv_idx_i_end = bvecStartEnd[i].endpoint;

		vec3d force_i = {0.0};
		double pot_i = 0.0;


		if(ki == KIND_W){

			double rho = 0.0;

			for(int bv_idx_i = bv_idx_i_start; bv_idx_i < bv_idx_i_end; bv_idx_i ++){
				const BVECTOR& bv_i = bvector[bv_idx_i];
				const int j = bv_i.pairIndex;
				//if(j < i) continue;		// prevent double count of bond //
			
				const int kj = knd[j];
			
				if(kj == KIND_W){
//					vec3d rij = bv_i.rij;
//					double r2 = rij * rij;
		
					const double phi = CalcRho2(i, bv_i.dr);
					rho += phi;
				}
			}

			
			/*v2.8.2
			sqr_rho = sqrt(re_rho[i] + m_param_Rho_d*m_param_Rho_d);
			a_pot[i] -= (re_rho[i]/sqr_rho) * 2.0;	//(和を取る時にまとめて0.5倍するのでここで2をかけてキャンセル)
			coef3 = (1.0 - re_rho[i]/(2.0*sqr_rho*sqr_rho))/sqr_rho;
			*/
			//simd//
			const double sqr_rho = sqrt(rho + m_param_Rho_d*m_param_Rho_d);
			pot_i -= (rho/sqr_rho) * 2.0;	//(和を取る時にまとめて0.5倍するのでここで2をかけてキャンセル)
			coef3 = (1.0 - rho/(2.0*sqr_rho*sqr_rho))/sqr_rho;
			
		}




		for(int bv_idx_i = bv_idx_i_start; bv_idx_i < bv_idx_i_end; bv_idx_i ++){
			const BVECTOR& bv_i = bvector[bv_idx_i];
			
			// pair atom //
			const int j = bv_i.pairIndex;
			//if(j < i) continue;		// prevent double count of bond //
			
			const int kj = knd[j];
			
			const vec3d rij = bv_i.rij;
			const double r = bv_i.dr;
	
			// calculate 2 body force and potnetial================== //
		
//			double r2 = rij * rij;
//			double r = sqrt(r2);
			vec3d force_ij = {0.0, 0.0, 0.0};

			//元素の組み合わせ//
			switch(ki){
			case KIND_W:
				switch(kj){
				case KIND_W:
		
					// 2 body force //
					//if( i < j ){
					if( bv_idx_i < twobody_endpoint){
						double V, dV;
						if(Force2body_WW(r, &V, &dV)){
					
										
							//a_pot[i] += V;	//m_potに収められるのは0.5倍しない.(和を取る時にまとめて)
							//a_pot[j] += V;
	
							//a_pot[i] += V*2.0;
							pot_i += V*2.0;

							force_ij += rij * ( -dV / r);
						
						}
					}


					// 3 body force //
	
					{
						double d_phi;
						if(ForceMultibody_Rho(r, &d_phi)){
							force_ij += rij * (coef3 * d_phi / r);
						}
					}
					// set force_ij //
					force_i += force_ij;
					SubForceAtomic(j,i_start,i_end,force_ij, a_force+j, sh_force+j);
					

					AddFr(force_ij, rij, a_Fr_m);// add stress tensor //
					AddPressure(i,j,i_start,i_end,force_ij, rij);

					break;
				case KIND_He:
					{
						double V, dV;

						if(Force2body_HeW(r, &V, &dV)){
						
							//a_pot[i] += V;	//m_potに収められるのは0.5倍しない.(和を取る時にまとめて)
							//a_pot[j] += V;
							//a_pot[i] += V * 2.0;
							pot_i += V * 2.0;

							force_ij += rij * ( -dV / r);
							// set force_ij //
							force_i += force_ij;
							SubForceAtomic(j,i_start,i_end,force_ij, a_force+j, sh_force+j);

							AddFr(force_ij, rij, a_Fr_m);// add stress tensor //
							AddPressure(i,j,i_start,i_end,force_ij, rij);

						}
					}
					break;
				}
				break;
			case KIND_He:
					
				switch(kj){
				//				//i - j ペアの一方を省く//
				//case KIND_W:
				//	
				//	break;
					
				case KIND_He:
					// 2 body force //
					//if( i < j ){
					if( bv_idx_i < twobody_endpoint){
						double V, dV;
						if(Force2body_HeHe(r, &V, &dV)){
					
										
						//	a_pot[i] += V;	//m_potに収められるのは0.5倍しない.(和を取る時にまとめて)
						//	a_pot[j] += V;
						//	a_pot[i] += V*2.0;
							pot_i += V*2.0;

							force_ij += rij * ( -dV / r);
					
							// set force_ij //
							
							force_i += force_ij;
							SubForceAtomic(j,i_start,i_end,force_ij, a_force+j, sh_force+j);


							AddFr(force_ij, rij, a_Fr_m);// add stress tensor //
							
							AddPressure(i,j,i_start,i_end,force_ij, rij);
						}
					
					}
					break;
				}//switch//
				break;
			}//switch//


		}// loop of bv_idx_i //
	
		a_force[i] += force_i;
		a_pot[i] = pot_i;

	}// parallel loop of i-th atom //

		
}



inline double AFSIv7::CalcRho2(int i, double r){

	
	/* v2.5 */
	const double rcs = RHO_CUTOFF_SHORT;//AFSIv7_CUTOFF_SHORT;
	const double cutoff = RHO_CUTOFF_LONG;
	const double rcd = cutoff - rcs;//AFSIv7_CUTOFF_RANGE;
	//double r = sqrt(r2);
	
	if(r < cutoff){
		double phi = m_param_Rho_B*m_param_Rho_B * exp(-m_param_Rho_c * r);
		const double S = r;//r*r;//r*r);
		phi = phi * S;
		
		
		if (r > rcs) {
#ifndef CUTOFF_3D
				const double rn = ( r - rcs) / rcd;
				const double rn2 = rn * rn;
				const double rn3 = rn2 * rn;
				const double fc = (- 6.0 * rn2 + 15.0 * rn - 10.0) * rn3 + 1.0;
			//	double dfc = - 30.0 * (rn - 1.0) * (rn - 1.0) * rn2 / (rcd);		
			
#else				
				double rn = ( r - rcs) / rcd;
				double rn_1 = (rn - 1.0);
				double fc = 2.0 * rn_1 * rn_1 * (rn + 0.5);
				double dfc = 6.0 * rn_1 * rn / (rcd);		
#endif			

				//dV = dV * fc + V * dfc;
				//V *= fc;
				phi *= fc;
		}
		
		return phi;	//周辺密度.
	}

	return 0.0;
}


inline int AFSIv7::Force2body_WW(double r, double *pV, double *pdV){
	const double rcs = WW_CUTOFF_SHORT;
	const double rcd = WW_CUTOFF_LONG - WW_CUTOFF_SHORT;
	const double cutoff  = WW_CUTOFF_LONG;		//カットオフ長の二乗.

	
	// v2.5
	if(r < cutoff){
		double V = m_param_WW_z0 * exp(-m_param_WW_a0 * r) / r;
		double dV = -(m_param_WW_a0 + 1.0 / r) * V;

		
	
	/*
		double S = (1.0 + m_param_WW_k0*r +  m_param_WW_b0*r*r + m_param_WW_c0*r*r*r);//遮蔽の補正//
		double dS = (-m_param_WW_k0 + 2.0*m_param_WW_b0*r + 3.0*m_param_WW_c0*r*r);//遮蔽の補正//
		double V = Vr * S;
		double dV = dVr * S + Vr * dS;
	*/	
/*	
		double Va = m_param_WW_A1 * exp(- m_param_WW_a1*r) * (1.0);
		V += Va;
		dV -= - m_param_WW_a1 * Va +0.0*Va/r;
*/
		//カットオフ///////////////////////////
		if (r > rcs) {
#ifndef CUTOFF_3D
			const double rn = ( r - rcs) / rcd;
			const double rn2 = rn * rn;
			const double rn3 = rn2 * rn;
			const double fc = (- 6.0 * rn2 + 15.0 * rn - 10.0) * rn3 + 1.0;
			const double dfc = - 30.0 * (rn - 1.0) * (rn - 1.0) * rn2 / (rcd);		
			
#else				
			double rn = ( r - rcs) / rcd;
			double rn_1 = (rn - 1.0);
			double fc = 2.0 * rn_1 * rn_1 * (rn + 0.5);
			double dfc = 6.0 * rn_1 * rn / (rcd);		
#endif			
		
			dV = dV * fc + V * dfc;
			V *= fc;
		}
		///////////////////////////カットオフ//

		*pV = V;
		*pdV = dV;
		return 1;
	}
	
	return 0;
}

// 2 body force //
inline int AFSIv7::Force2body_HeW(double r, double *pV, double *pdV){
	const double rcs = HeW_CUTOFF_SHORT;
	const double rcd = HeW_CUTOFF_RANGE;
	const double cutoff  = rcs + rcd;		//カットオフ長の二乗.

	if(r < cutoff){

		const double Vr = m_param_HeW_z0 * exp(-m_param_HeW_a0 * r) / r;
		const double S = (1.0 +  m_param_HeW_b0*r*r + m_param_HeW_c0*r*r*r);//遮蔽の補正//
		double V = Vr * S;
		double dV = -(m_param_HeW_a0 + 1.0 / r) * Vr * S;
		dV += Vr * (2.0*m_param_HeW_b0*r + 3.0*m_param_HeW_c0*r*r);


		//カットオフ///////////////////////////
		if (r > rcs) {
#ifndef CUTOFF_3D
			const double rn = ( r - rcs) / rcd;
			const double rn2 = rn * rn;
			const double rn3 = rn2 * rn;
			const double fc = (- 6.0 * rn2 + 15.0 * rn - 10.0) * rn3 + 1.0;
			const double dfc = - 30.0 * (rn - 1.0) * (rn - 1.0) * rn2 / (rcd);		
			
#else				
			double rn = ( r - rcs) / rcd;
			double rn_1 = (rn - 1.0);
			double fc = 2.0 * rn_1 * rn_1 * (rn + 0.5);
			double dfc = 6.0 * rn_1 * rn / (rcd);		
#endif					
			dV = dV * fc + V * dfc;
			V *= fc;
		}
		///////////////////////////カットオフ//

		*pV = V;
		*pdV = dV;
		return 1;
	}
				
	return 0;
}


inline int AFSIv7::Force2body_HeHe(double r, double *pV, double *pdV){
	
	const double rcs = HeHe_CUTOFF_SHORT;
	const double rcd = HeHe_CUTOFF_RANGE;
	const double cutoff  = rcs + rcd;		//カットオフ長

	if(r < cutoff){
		
		const double Vr = m_param_HeHe_z0 * exp(-m_param_HeHe_a0 * r) / r;
		const double S = (1.0 +  m_param_HeHe_b0*r*r + m_param_HeHe_c0*r*r*r);//遮蔽の補正//
		double V = Vr * S;
		double dV = -(m_param_HeHe_a0 + 1.0 / r) * Vr * S;
		dV += Vr * (2.0*m_param_HeHe_b0*r + 3.0*m_param_HeHe_c0*r*r);
		
	
		//カットオフ///////////////////////////
		if (r > rcs) {
#ifndef CUTOFF_3D
			const double rn = ( r - rcs) / rcd;
			const double rn2 = rn * rn;
			const double rn3 = rn2 * rn;
			const double fc = (- 6.0 * rn2 + 15.0 * rn - 10.0) * rn3 + 1.0;
			const double dfc = - 30.0 * (rn - 1.0) * (rn - 1.0) * rn2 / (rcd);		
			
#else				
			double rn = ( r - rcs) / rcd;
			double rn_1 = (rn - 1.0);
			double fc = 2.0 * rn_1 * rn_1 * (rn + 0.5);
			double dfc = 6.0 * rn_1 * rn / (rcd);		
#endif		

			dV = dV * fc + V * dfc;
			V *= fc;
		}
		///////////////////////////カットオフ//
		
		*pV = V;
		*pdV = dV;
		return 1;
	}

	
	return 0;//カットオフ外なら0を返す//
}


inline int AFSIv7::ForceMultibody_Rho(double r, double *pdphi){

	const double rcs = RHO_CUTOFF_SHORT;//AFSIv7_CUTOFF_SHORT;
	const double cutoff = RHO_CUTOFF_LONG;
	const double rcd = cutoff - rcs;//AFSIv7_CUTOFF_RANGE;
		
	if(r < cutoff){
		double phi = m_param_Rho_B*m_param_Rho_B * exp(-m_param_Rho_c * r);
		double d_phi = -m_param_Rho_c*phi;
		
		const double S = r;
		const double dS = 1.00;

		d_phi = d_phi * S +phi*dS;
		phi = phi * S;

		if (r > rcs) {
#ifndef CUTOFF_3D
			const double rn = ( r - rcs) / rcd;
			const double rn2 = rn * rn;
			const double rn3 = rn2 * rn;
			const double fc = (- 6.0 * rn2 + 15.0 * rn - 10.0) * rn3 + 1.0;
			const double dfc = - 30.0 * (rn - 1.0) * (rn - 1.0) * rn2 / (rcd);		
			
#else				
			double rn = ( r - rcs) / rcd;
			double rn_1 = (rn - 1.0);
			double fc = 2.0 * rn_1 * rn_1 * (rn + 0.5);
			double dfc = 6.0 * rn_1 * rn / (rcd);		
#endif					
				//dV = dV * fc + V * dfc;
				//V *= fc;
				d_phi = d_phi * fc + phi * dfc;
		}

		*pdphi = d_phi;
		return 1;

	}
						
	return 0;//カットオフ外なら0を返す//
}
					

inline void AFSIv7::SubForceAtomic(const int j, const int i_start, const int i_end, const vec3d& force_j, vec3d* a_force, vec3d* sh_force){
	if( (i_start <= j) && (j < i_end) ){
		*a_force -= force_j;
	}else{
		#pragma omp atomic
		sh_force->x -= force_j.x;
		#pragma omp atomic
		sh_force->y -= force_j.y;
		#pragma omp atomic
		sh_force->z -= force_j.z;
	}
					
}

inline void AFSIv7::AddFr(const vec3d& force_ij, const vec3d& r_ij, mat33d* a_Fr_m){
	a_Fr_m->a += force_ij * r_ij.x;
	a_Fr_m->b += force_ij * r_ij.y;
	a_Fr_m->c += force_ij * r_ij.z;
}


inline void AFSIv7::AddPressure(const int i, const int j, const int i_start, const int i_end, const vec3d& force_ij, const vec3d& r_ij){
	double PV = (force_ij * r_ij) * 0.5;

	#pragma omp atomic
	m_PV[i] += PV;
	
	#pragma omp atomic
	m_PV[j] += PV;
	
					
}


void AFSIv7::GetFr(mat33d* a_Fr_m){
		*a_Fr_m = m_Fr_m;

}


void AFSIv7::GetParams(double* params){


	memcpy(params, m_afs_params, sizeof(double) * (DF_NUMPARAMS));

//	params[AFSIv7_DF_NUMPARAMS] = m_E_W;
//	params[AFSIv7_DF_NUMPARAMS+1] = m_E_He;

}

void AFSIv7::SetParams(const double* params){

	memcpy(m_afs_params, params, sizeof(double) * (DF_NUMPARAMS));
	
	/*
	RHO_CUTOFF_SHORT = m_param_Rho_short;
	RHO_CUTOFF_LONG = m_param_Rho_long;

	WW_CUTOFF_SHORT = m_param_WW_short;
	WW_CUTOFF_LONG = m_param_WW_long;
	*/
}

/*
void AFSIv7::SingleAtomEnergy(int count, double *a_pot){
	#pragma omp for
	for(int i = 0; i < count; i++){
		if(m_knd[i] == KIND_W){
			a_pot[i] += m_E_W * 2.0;
		}else if(m_knd[i] == KIND_He){
			a_pot[i] += m_E_He * 2.0;
		}
	}
}
*/

void AFSIv7::GetInfo(int mode, void* params){
	*(double**)params = m_PV;
	
};
