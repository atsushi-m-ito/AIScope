
#if 0

#define _CRT_SECURE_NO_WARNINGS

//#include "plugTDTemplate.h"

#include "ATOMS_DATA.h"
#include "debugprint.h"

#include <stdio.h>
#include <string.h>


static FILE* fp = NULL;


void plugReadyTD_Template(const TCHAR* filepath, int dirlen){

	TCHAR* myfilepath = new TCHAR[dirlen + 20];

	_tcsncpy(myfilepath,filepath, dirlen);
	_tcscpy(myfilepath + dirlen, _T("tddepth.txt"));

	fp = _tfopen(myfilepath, _T("w"));
	delete [] myfilepath;


}


void plugFrameTD_Template(ATOMS_DATA *dat){
	

}

void plugFinishTD_Template(ATOMS_DATA *dat){
	//datには最終フレームが入る.


	fclose(fp);
	fp = NULL;


}

#endif