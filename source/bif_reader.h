#pragma once
#ifndef BIF_READER_H
#define BIF_READER_H


#include "io64.h"
#include <tchar.h>

typedef long long field_int;

class BIF_Reader
{
private:
	FILE64* m_fp;
	field_int m_grid_x;
	field_int m_grid_y;
	field_int m_grid_z;
	
public:


	BIF_Reader();
	virtual ~BIF_Reader();

	void Open(const TCHAR* filepath);
	void Close();

	void Read(double* field);
	int GetSize(field_int* size_x, field_int* size_y, field_int* size_z, double* delta_x, double* delta_y, double* delta_z);

};



#endif    // BIF_READER_H
