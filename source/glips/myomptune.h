
#ifndef __myomptune_h__
#define __myomptune_h__
#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>

//何か一つのファイルでGLOBAL_DEFINEを定義してからincludeすると実態となる//
#ifdef GLOBAL_DEFINE		
#define GLOBAL
#else
#define GLOBAL extern
#endif


GLOBAL volatile double tm1;
GLOBAL volatile double tm2;
GLOBAL volatile double tm_linkedlist;
GLOBAL volatile double tm_bondcheck;
GLOBAL volatile double tm_force;
GLOBAL volatile double tm_other;
GLOBAL volatile double tm_memflush1;
GLOBAL volatile double tm_memflush2;
GLOBAL volatile double start_tm;


#define  myomp_tm_count 16
GLOBAL volatile double myomp_tm[myomp_tm_count];
GLOBAL volatile double myomp_stm;

#ifdef _OPENMP

#define OMP_DEBUG
#define MYOMP_DEBUG_TIME

inline void myompTimeStart(){
	#pragma omp master
	myomp_stm = omp_get_wtime();
}

inline void myompTimeSet(int num){
	#pragma omp master
	{
		double tm = omp_get_wtime();
		myomp_tm[num] += tm - myomp_stm;
		myomp_stm = tm;
	}
}
#endif //_OPENMP


#ifdef MYOMP_DEBUG_TIME


#define DEBUG_TM_CLEAR	{for(int i = 0; i<myomp_tm_count;i++){myomp_tm[i]=0.0;}}
#define DEBUG_TM_START	{myompTimeStart();}
#define DEBUG_TM_SET(num)	{myompTimeSet(num);}

#else

#define DEBUG_TM_CLEAR
#define DEBUG_TM_START
#define DEBUG_TM_SET(num)

#endif	//MYOMP_DEBUG_TIME

inline void debug_tm_print(){
#ifdef MYOMP_DEBUG_TIME
		
		printf("tm_reset %lf s\n", myomp_tm[0]);
		printf("tm_linkedlist %lf s\n", myomp_tm[1]);
		printf("tm_bondcheck %lf s\n", myomp_tm[2]);
		printf("tm_force %lf s\n", myomp_tm[3]);
		printf("tm_marge_bforce %lf s\n", myomp_tm[4]);
		printf("tm_marge_aforce %lf s\n", myomp_tm[5]);
		double tm_force = 0.0;
		for(int i = 0; i < 6; i++){
			tm_force += myomp_tm[i];
		}
		printf("Force %lf s\n", tm_force);

#ifdef _DDM
		printf("DDM_Redeploy %lf s\n", myomp_tm[6]);
		printf("DDM_MergePosition %lf s\n", myomp_tm[7]);
		printf("DDM_MergeForce %lf s\n", myomp_tm[8]);
		for(int i = 6; i < 9; i++){
			tm_force += myomp_tm[i];
		}
		printf("DDM_Force %lf s\n", tm_force);
#endif	//_DDM

		
		printf("integrator %lf s\n", myomp_tm[10]);
		

#endif	//MYOMP_DEBUG_TIME
}


#endif