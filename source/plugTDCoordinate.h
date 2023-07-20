#ifndef __plugTDCoordinate_h__
#define __plugTDCoordinate_h__

#include "MDLoader.h"

void plugReadyTD_Coordinate(const TCHAR* filepath, int dirlen);

void plugFinishTD_Coordinate(ATOMS_DATA *dat);

void plugFrameTD_Coordinate(ATOMS_DATA *dat);


#endif	//!__plugTDCoordinate_h__