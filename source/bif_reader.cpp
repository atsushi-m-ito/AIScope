
#include "bif_reader.h"
#include "io64.h"





BIF_Reader::BIF_Reader() : 
	m_fp(NULL) 
{
		
}

BIF_Reader::~BIF_Reader(){
	Close();
}


void BIF_Reader::Open(const TCHAR* filepath){

	
	
	m_fp = fopen64(filepath, _T("rb"));

	const char header_string[] = "BINFIELD";
	//const char reserve_region[8] = {0};
	char buffer[16];

	fread64(buffer, 16, 1, m_fp);

	if( strncmp(buffer, header_string, 8) != 0){
		//error//
		Close();
		m_fp = NULL;
	}	
}

void BIF_Reader::Close(){
	if(m_fp){ fclose64(m_fp);}
	m_fp = NULL;
}


int BIF_Reader::GetSize(field_int* size_x, field_int* size_y, field_int* size_z, double* delta_x, double* delta_y, double* delta_z){
	
	if(m_fp == NULL) { return 0;}

	
	size_t res = fread64(size_x, sizeof(field_int), 1, m_fp);
	if(res==0){
		return 0;
	}
	
	fread64(size_y, sizeof(field_int), 1, m_fp);
	fread64(size_z, sizeof(field_int), 1, m_fp);
	fread64(delta_x, sizeof(double), 1, m_fp);
	fread64(delta_y, sizeof(double), 1, m_fp);
	fread64(delta_z, sizeof(double), 1, m_fp);

	m_grid_x = *size_x;
	m_grid_y = *size_y;
	m_grid_z = *size_z;

	return (int)m_grid_x;

}

void BIF_Reader::Read(double* field){
	
	if(m_fp == NULL) { return ;}

	const field_int ONE_GIGA_BYTE = 1073741824;
	field_int full_size = m_grid_x * m_grid_y * m_grid_z * sizeof(double);
	if(full_size <= ONE_GIGA_BYTE){
		fread64(field, (int)full_size, 1, m_fp);
	}else{
		field_int size_xy = m_grid_x * m_grid_y * sizeof(double);
		for(field_int iz = 0; iz < full_size; iz += size_xy){
			fread64(field + iz, (int)size_xy, 1, m_fp);
		}
	}
	

}

