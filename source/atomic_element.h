//公開レベル1//
#pragma once

#ifndef ATOMIC_ELEMENT_H
#define ATOMIC_ELEMENT_H

#include <tchar.h>

const double GLIPS_MASS[] = { 
		12.0,		//empty軌道//
		1.0079,	4.0026,	//第1周期
		6.941,	9.012,	10.811,	12.0, 14.0067, 15.9994, 18.9984,	20.1797,	//第2周期
		1,	2,	3,	4,	5,	6,	7,	39.948,	//第3周期
		1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,	18,	//第4周期
		1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,	18,	//第5周期
		9,	10,	//第6周期(Cs, Ba)
		3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,	////第6周期(ランタノイド)
		178.49,	180.947, 183.84, 15, 14,	15, 16,	17,	17,	19,	14,	15,	84,	85,	86,//第6周期(Hf-Rn)(W(74)->14)
		87,	88,	//第7周期(Fr, Ra)
		3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,	////第7周期(アクチノイド)
		104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,//第7周期(Rf-Uuo)
};



/*
//OpenMXで入力する元素記号//
static const char* atomic_element_list[] = {
    "E",		//empty軌道//
    "H", "He",	//第1周期
    "Li", "Be", "B", "C", "N", "O", "F", "Ne",		//第2周期
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar",	//第3周期
    "K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",	//第4周期
    "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe",	//第5周期
    "Cs", "Ba",	//第6周期(Cs, Ba)
    "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu",	////第6周期(ランタノイド)
    "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",//第6周期(Hf-Rn)
    "Fr", "Ra",	//第7周期(Fr, Ra)
    "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",	////第7周期(アクチノイド)
    //104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,//第7周期(Rf-Uuo)
};
*/

//OpenMXで入力する元素記号//

static const TCHAR* atomic_element_list[] = {
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

//元素記号の文字列から原子番号を返す//
inline static int GetAtomicNumber(const TCHAR* atomicsymbol){
	static const int SPECIES_COUNT = 118 + 1;

    for (int i = 0; i < SPECIES_COUNT; i++){
        if (_tcscmp(atomicsymbol, atomic_element_list[i]) == 0){
            return i;
        }
    }
    return -1;
}


//元素記号の文字列から原子番号を返す//
inline static const TCHAR* GetAtomicSymbol(const int atomic_number){
    return atomic_element_list[atomic_number];
}



#endif	//!ATOMIC_ELEMENT_H