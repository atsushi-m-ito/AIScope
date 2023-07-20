#if 0
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "plugTDDepth.h"
#include "ai_molecule.h"
#include "debugprint.h"

static FILE* fp = NULL;

static const int bufsize = 10000;
static double* surface = NULL;
static int maxid = 0;
static int initid = -1;

static double GetSurface(ATOMS_DATA* dat);
/*
void plugReadyTD_Depth(const TCHAR* filepath, int dirlen){

	TCHAR* myfilepath = new TCHAR[dirlen + 20];

	_tcsncpy(myfilepath,filepath, dirlen);
	_tcscpy(myfilepath + dirlen, _T("tddepth.txt"));

	fp = _tfopen(myfilepath, _T("w"));
	delete [] myfilepath;

	surface = new double[bufsize];

}

void plugFrameTD_Depth(ATOMS_DATA *dat){

	int count = dat->pcnt;
	int *id = dat->id;
	for(int i = 0; i < dat->pcnt; i++){
		if(id[i] > maxid){
			maxid = id[i];
			if(initid != -1){

				//ここで表面位置を記録
				surface[maxid - initid] = GetSurface(dat);
			}
		}
	}

	//初期にあるバルクは除くため.
	if(initid == -1){ initid = maxid+1;}

}

void plugFinishTD_Depth(ATOMS_DATA *dat){
	//datには最終フレームが入る.

	int count = dat->pcnt;
	int* id = dat->id;
	for(int i = 0; i <= count; i++){
		if(id[i] >= initid){
			TRACE(_T("%d\t%lf\t%lf\n"), id[i], surface[id[i]-initid], dat->r[i].z);
			_ftprintf(fp, _T("%d\t%lf\t%lf\n"), id[i], surface[id[i]-initid], dat->r[i].z);
		}
	}

	fclose(fp);
	fp = NULL;

	delete [] surface;

}
*/


static double min_depth;
static double cur_surface;
static bool reflect_flag = false;
static int calc_counter = 0;

void plugReadyTD_Depth(const TCHAR* filepath, int dirlen){

	TCHAR* myfilepath = new TCHAR[dirlen + 20];

	_tcsncpy(myfilepath,filepath, dirlen);
	_tcscpy(myfilepath + dirlen, _T("tddepth.txt"));

	fp = _tfopen(myfilepath, _T("w"));
	delete [] myfilepath;

	surface = new double[bufsize];
	//depth = new double
	calc_counter = 2;
	maxid = 1;
}


void plugFrameTD_Depth(ATOMS_DATA *dat){
	
	calc_counter--;
	if(calc_counter == 0){

		surface[maxid] = GetSurface(dat);


		maxid++;
		calc_counter=5;
	}
}

void plugFinishTD_Depth(ATOMS_DATA *dat){
	//datには最終フレームが入る.

	int count = dat->pcnt;
	int* id = dat->id;
	int* knd = dat->knd;

	for(int m = 1; m < maxid; m++){

		int num = 0;
		for(int i = 0; i <= count; i++){
			if(m == id[i]){ 
				num++;
				if(num == 2){


					TRACE(_T("%d\t%lf\t%lf\n"), m, surface[m], dat->r[i].z);
					_ftprintf(fp, _T("%d\t%lf\t%lf\n"), m, surface[m], dat->r[i].z);


					break;
				}
			}
		}
	}


	fclose(fp);
	fp = NULL;


}

double GetSurface(ATOMS_DATA* dat){
	//MolListを元に、大きい分子バルクと認識して表面座標を返す
	//
	
	
	double max_z = -1.0E100;

	if(dat->bcnt==0){ AI_ResetBond(dat);}
	
	int *pNext = AI_CreateMolList(dat);

	for (int i = 0; i < dat->pcnt; i++){
		//末尾ならheadの番号で符号を負にした値が入る
		if (pNext[i] >=0){ continue;}

		//ここに進んだ場合は分子の先頭の原子.
		int count = 0;
		double mol_max_z = -1.0E100;


		//繋がっている原子のループ
		int head = -pNext[i] -1;
		for (int li = head; li >= 0; li = pNext[li]){

			count ++;
			if(mol_max_z < dat->r[li].z) mol_max_z = dat->r[li].z;
		}

		//大きな分子なら表面の更新
		if(count > 50){
			if(max_z < mol_max_z) max_z = mol_max_z;
		}
		

	}

	delete [] pNext;

	return max_z;

}
#endif