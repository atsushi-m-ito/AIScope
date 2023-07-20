#pragma once
#ifndef OutCoordinationNumber_H
#define OutCoordinationNumber_H

#include <tchar.h>
#include "ATOMS_DATA.h"
#include "BOND_INFO.h"
class AIScope;

int OutCoordinationNumber(ATOMS_DATA *dat, BOND_INFO* bond, const TCHAR* filename);
void OutputSequentialCoordinateNumber(const TCHAR* filename, AIScope& aiscope);


#endif    // OutCoordinationNumber_H
