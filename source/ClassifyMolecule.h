
#ifndef ClassifyMolecule_H
#define ClassifyMolecule_H

#include "ATOMS_DATA.h"
#include "BOND_INFO.h"

#include <tchar.h>

//int* AI_CreateMolList(ATOMS_DATA *dat);
int AI_WriteClassifyMolecule2(ATOMS_DATA *dat, BOND_INFO* bond, const TCHAR* filename);
int AI_WriteClassifyMolecule(ATOMS_DATA *dat, BOND_INFO* bond, const TCHAR* filename);
//int AI_nearestAtom(ATOMS_DATA *dat, float x, float y, float z, float limit_r);
//int AI_WriteAdsorbedH(MDLoader* a_atm, int frameskip, const TCHAR* filename);


/*
void AI_eraceBulk(ATOMS_DATA *dat, float limit_z);
void AI_eraceUpper(ATOMS_DATA *dat, float limit_z);

void WriteAdsH(MDLoader* a_atm, int frameskip, const TCHAR* spath);


double fperiodic(double a, double size);
float fperiodic(float a, float size);
*/

#endif    // ClassifyMolecule_H
