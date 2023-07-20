#ifndef __epsrender_h__
#define __epsrender_h__

#include <tchar.h>
#include "sdkwgl.h"
#include "outeps.h"
#include "MDLoader.h"

FILE* OpenAtomEPS(const TCHAR* filename, int w, int h);
void CloseAtomEPS(FILE* fp);
void DrawAtomEPS(FILE* fp, ATOMS_DATA *dat, int w, int h);


#endif    // __epsrender_h__
