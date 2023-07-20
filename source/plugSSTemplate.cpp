
#if 0

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "plugSSCoordinate.h"
#include "ClassifyMolecule.h"
#include "debugprint.h"




void plugFrameSS_Coordinate(const TCHAR* filepath, ATOMS_DATA *dat){

//ファイル初期化///////////////////////////////////////
	const TCHAR* extpos = _tcsrchr(filepath, _T('.'));
	size_t length = extpos - filepath;
	TCHAR* myfilepath = new TCHAR[length + 20];
	

	_tcsncpy(myfilepath,filepath, length);
	_tcscpy(myfilepath + length, _T("_coordinate.txt"));

	FILE* fp = _tfopen(myfilepath, _T("w"));
	delete [] myfilepath;

	
	_ftprintf(fp,_T("#atom\tbond\tcoord[0]\tcoord[1]...\n"));

//メイン処理//////////////////////////

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

	int valid_count=0;
/*
	//粒子ループ
	int* knd = dat->knd;
	for(int i = 0; i < count; i++){
		if(knd[i] > 1){
			num_atom_coord[coord[i]] ++;
			valid_count++;
		}
	}
*/

	/*
	//粒子ループ
	int* id= dat->id;
	bool *flags = new bool[count+1];
	memset(flags, 0, sizeof(bool) * (count+1));
	for(int i = 0; i < count; i++){
		if(flags[id[i]]){
			num_atom_coord[coord[i]] ++;
			valid_count++;
		}else{
			flags[id[i]] = true;
		}
	}
	delete [] flags;
	*/
	for(int i = 0; i < count; i++){
		num_atom_coord[coord[i]] ++;
		valid_count++;
	}

	delete [] coord;

	//ファイル出力
	_ftprintf(fp,_T("%d\t%d"), valid_count, dat->bcnt);
	for(int i = 0; i < 20; i++){
		_ftprintf(fp,_T("\t%d"), num_atom_coord[i]);
	}
	
//ファイル終了//////////////////////////

	fclose(fp);
	fp = NULL;

}
#endif