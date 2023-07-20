#include "md3_writer.h"


MD3_Writer::MD3_Writer() : 
		m_fp (NULL)
{
	
	
}

MD3_Writer::~MD3_Writer(){
	Close();
}


int MD3_Writer::Open(const char* filepath){
	
	if(m_fp){
		return -2;
	}

	m_fp = fopen(filepath, "w");
	if( ! m_fp){
		m_fp = NULL;
		return -1;
	}

	m_frameno=0;
	return 0;
}

void MD3_Writer::Close(){
	if(m_fp){
		fclose(m_fp);
		m_fp = NULL;
	}

}



int MD3_Writer::Write(int count, const int* knd, const double* r, const double* f, const double *boxaxis, const char* comment){

	if(m_fp == NULL){ return -1;}

	//粒子数の出力//
	fprintf(m_fp, "%d\n", count);
	
	//コメント行の出力//
	if(comment){
		fprintf(m_fp, "%s\n", comment);
	}else{
		fprintf(m_fp, "frame = %d\n", m_frameno);
	}

	//BOXの出力//
	if(boxaxis){
		fprintf(m_fp,"BOX   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
			boxaxis[0], boxaxis[1], boxaxis[2], 
			boxaxis[3], boxaxis[4], boxaxis[5],
			boxaxis[6], boxaxis[7], boxaxis[8]);
	}else{
		fprintf(m_fp,"BOX   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
			10.0, 0.0, 0.0, 
			0.0, 10.0, 0.0,
			0.0, 0.0, 10.0);
	}
	
	if(f){
		for( int i = 0; i < count; i++){
			fprintf(m_fp,"%4s   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
                  msz::GetAtomicSymbol(knd[i]),
                  r[i*3], r[i*3+1], r[i*3+2],
                  f[i*3], f[i*3+1], f[i*3+2]);
		}
	}else{
		for( int i = 0; i < count; i++){
			fprintf(m_fp,"%4s   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
				msz::GetAtomicSymbol(knd[i]),
                r[i*3], r[i*3+1], r[i*3+2],
                0.0, 0.0, 0.0);
		}

	}

	m_frameno++;

	return m_frameno;
}



int MD3_Writer::Write_f(int count, const int* knd, const float* r, const float* f, const float *boxaxis, const char* comment){

	if(m_fp == NULL){ return -1;}

	//粒子数の出力//
	fprintf(m_fp, "%d\n", count);
	
	//コメント行の出力//
	if(comment){
		fprintf(m_fp, "%s\n", comment);
	}else{
		fprintf(m_fp, "frame = %d\n", m_frameno);
	}

	//BOXの出力//
	if(boxaxis){
		fprintf(m_fp,"BOX   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
			boxaxis[0], boxaxis[1], boxaxis[2], 
			boxaxis[3], boxaxis[4], boxaxis[5],
			boxaxis[6], boxaxis[7], boxaxis[8]);
	}else{
		fprintf(m_fp,"BOX   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
			10.0, 0.0, 0.0, 
			0.0, 10.0, 0.0,
			0.0, 0.0, 10.0);
	}
	
	if(f){
		for( int i = 0; i < count; i++){
			fprintf(m_fp,"%4s   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
                  msz::GetAtomicSymbol(knd[i]),
                  r[i*3], r[i*3+1], r[i*3+2],
                  f[i*3], f[i*3+1], f[i*3+2]);
		}
	}else{
		for( int i = 0; i < count; i++){
			fprintf(m_fp,"%4s   %15.8f  %15.8f  %15.8f  %15.8f  %15.8f  %15.8f\n",
				msz::GetAtomicSymbol(knd[i]),
                  r[i*3], r[i*3+1], r[i*3+2],
                  0.0, 0.0, 0.0);
		}

	}

	m_frameno++;

	return m_frameno;
}
