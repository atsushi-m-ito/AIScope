#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "OutCoordinationNumber.h"

#include "winmain.h"
#include "ATOMS_DATA.h"
#include "BOND_INFO.h"
#include "AIScope.h"


static const int SPECIES_COUNT = 118 + 1;

//OpenMXで入力する元素記号//
static const TCHAR* openmx_species_list_t[] = {
		_T("E"),		//empty軌道//
		_T("H"), _T("He"),	//第1周期
		_T("Li"), _T("Be"), _T("B"), _T("C"), _T("N"), _T("O"), _T("F"), _T("Ne"),		//第2周期
		_T("Na"), _T("Mg"), _T("Al"), _T("Si"), _T("P"), _T("S"), _T("Cl"), _T("Ar"),	//第3周期
		_T("K"), _T("Ca"), _T("Sc"), _T("Ti"), _T("V"), _T("Cr"), _T("Mn"), _T("Fe"), _T("Co"), _T("Ni"), _T("Cu"), _T("Zn"), _T("Ga"), _T("Ge"), _T("As"), _T("Se"), _T("Br"), _T("Kr"),	//第4周期
		_T("Rb"), _T("Sr"), _T("Y"), _T("Zr"), _T("Nb"), _T("Mo"), _T("Tc"), _T("Ru"), _T("Rh"), _T("Pd"), _T("Ag"), _T("Cd"), _T("In"), _T("Sn"), _T("Sb"), _T("Te"), _T("I"), _T("Xe"),	//第5周期
		_T("Cs"), _T("Ba"),	//第6周期(Cs, Ba)
		_T("La"), _T("Ce"), _T("Pr"), _T("Nd"), _T("Pm"), _T("Sm"), _T("Eu"), _T("Gd"), _T("Tb"), _T("Dy"), _T("Ho"), _T("Er"), _T("Tm"), _T("Yb"), _T("Lu"),	////第6周期(ランタノイド)
		_T("Hf"), _T("Ta"), _T("W"), _T("Re"), _T("Os"), _T("Ir"), _T("Pt"), _T("Au"), _T("Hg"), _T("Tl"), _T("Pb"), _T("Bi"), _T("Po"), _T("At"), _T("Rn"),//第6周期(Hf-Rn)
		_T("Fr"), _T("Ra"),	//第7周期(Fr, Ra)
		_T("Ac"), _T("Th"), _T("Pa"), _T("U"), _T("Np"), _T("Pu"), _T("Am"), _T("Cm"), _T("Bk"), _T("Cf"), _T("Es"), _T("Fm"), _T("Md"), _T("No"), _T("Lr"),	////第7周期(アクチノイド)
		//104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,//第7周期(Rf-Uuo)
};

int OutCoordinationNumber(ATOMS_DATA *dat, BOND_INFO* bond, const TCHAR* filename){
	

	FILE *fp = _tfopen(filename, _T("w"));
	if (fp == NULL){
		printf("error; open file\n");
		return 0;
	}


	_ftprintf(fp, _T("#index, Z, coordination\n"));

	//int histgram[24] = {0};


	//配位数を参照
	for(int i = 0; i < dat->pcnt; i++){
		_ftprintf(fp, _T("%d\t%s\t%d\n"), i, openmx_species_list_t[dat->knd[i]], bond->coords[i]);

		int index = (bond->coords[i] < 24 ? bond->coords[i] : 23);
		//histgram[index] ++;
	}


	fclose(fp);


	return 1;
}


void OutputSequentialCoordinateNumber(const TCHAR* filename, AIScope& aiscope) {
	

	FILE* fp = _tfopen(filename, _T("w"));

	_ftprintf(fp, _T("#frame, num_atoms, coord0, 1, 2, 3, 4, 5, 6, more\n"));
	
	int number = 0;
	int res = 0;
	while (res == 0) {

		ATOMS_DATA* dat = aiscope.GetAtomData();
		if (dat == NULL) break;

		BOND_INFO* bond = aiscope.GetBondData();

		{
			int num_coord[8] = { 0 };

			for (int i = 0; i < dat->pcnt; i++) {
				int index = bond->coords[i] < 7 ? bond->coords[i] : 7;
				num_coord[index]++;
			}

			_ftprintf(fp, _T("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n"), number, dat->pcnt, num_coord[0],
				 num_coord[1], num_coord[2], num_coord[3], num_coord[4], num_coord[5], num_coord[6], num_coord[7]);

		}

		Sleep(0);
		//1フレーム進める
		res = PlayMovie();
		number++;
	}
	fclose(fp);


}