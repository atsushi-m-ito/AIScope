#ifndef __mod_h__
#define __mod_h__

#pragma once


#include "atomic_element.h"

const double PI = (3.141592653589793);
const double JeV	= (1.602176462e-19);		//(J / eV)
const double Kb = (1.3806503e-23);			//Boltzmann constant (J / K)
const double KbeV = (Kb / JeV);			//coefficient (eV / K)
const double GPaeV_A3 = ( 1e-21 / JeV);	//coefficient (Pa / (eV / A^3))


const double Epsilon0 = (8.854187817620e-12);		//真空の誘電率//

//クーロンポテンシャルの係数をeV*Angstrom単位にしたもの(e^2を含む)
const double CoulombCoef_eV_A = (14.399644850445789272280976914069);	//(JeV * (e^2/ (4.0*PI*Epsilon0)))
			
const double EV_PAR_HATREE = 27.21138503233568;



const int KIND_C = 0;		//special setting for REBO potential.
const int KIND_H = 1;

const int KIND_He = 2;
const int KIND_Ne = 10;
const int KIND_Ar = 18;

const int KIND_V = 23;
const int KIND_Nb = 41;
const int KIND_Ta = 73;
const int KIND_Cr = 24;
const int KIND_Mo = 42;
const int KIND_W = 74;
const int KIND_Fe = 26;


#define		MASS_C		(GLIPS_MASS[KIND_C])
#define		MASS_H		(GLIPS_MASS[KIND_H])
#define		MASS_D		(2.0)
#define		MASS_T		(3.0)
#define		MASS_W		(GLIPS_MASS[KIND_W])
#define		MASS_He		(GLIPS_MASS[KIND_He])
#define		MASS_Ne		(GLIPS_MASS[KIND_Ne])
#define		MASS_Ar		(GLIPS_MASS[KIND_Ar])


#endif    // __mod_h__
