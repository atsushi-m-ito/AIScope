#if 0
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "plugTDCoordinate.h"
#include "ai_molecule.h"
#include "debugprint.h"

static FILE* fp = NULL;


static int framenum = 0;


void plugReadyTD_Coordinate(const TCHAR* filepath, int dirlen){

	TCHAR* myfilepath = new TCHAR[dirlen + 20];

	_tcsncpy(myfilepath,filepath, dirlen);
	_tcscpy(myfilepath + dirlen, _T("tdcoordinate.txt"));

	fp = _tfopen(myfilepath, _T("w"));
	delete [] myfilepath;

	
	_ftprintf(fp,_T("#atom\tbond\tcoord[0]\tcoord[1]...\n"));

	framenum=0;

}


void plugFrameTD_Coordinate(ATOMS_DATA *dat){
	
	//結合状態の再判定
	if(dat->bcnt==0){ AI_ResetBond(dat);}

	int count = dat->pcnt;

	int* coord = new int[count];
	memset(coord, 0, sizeof(int) * count);

	//ボンドループ
	SIMPLE_IJ* bond_end = dat->bonds + dat->bcnt;
	for(SIMPLE_IJ* bonds = dat->bonds; bonds != bond_end; bonds++){
		coord[bonds->i] ++;
		coord[bonds->j] ++;
	}
	
	int num_atom_coord[20] = {0};

	//粒子ループ
	for(int i = 0; i < count; i++){
		num_atom_coord[coord[i]] ++;
	}

	delete [] coord;

	//ファイル出力
	_ftprintf(fp,_T("%d\t%d\t%d"), framenum, count, dat->bcnt);
	for(int i = 0; i < 20; i++){
		_ftprintf(fp,_T("\t%d"), num_atom_coord[i]);
	}
	_ftprintf(fp,_T("\n"));
	
	framenum++;
}

void plugFinishTD_Coordinate(ATOMS_DATA *dat){
	//datには最終フレームが入る.

	
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