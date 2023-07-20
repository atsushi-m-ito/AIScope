
#define _CRT_SECURE_NO_DEPRECATE
#include "bin_reader.h"
#include <tchar.h>
#include <stdio.h>


int ReadBinFile(const TCHAR* filepath, unsigned char** ppBuf){
	
	FILE* fp = _tfopen(filepath, _T("rb"));
	//FILE* fp = fopen("vs_inst.cso", "rb");
	if( fp == NULL){
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	
	unsigned char* buf = new unsigned char[sz];
	fread(buf, sz, 1, fp);
	fclose(fp);

	
	*ppBuf = buf;

	return sz;

}