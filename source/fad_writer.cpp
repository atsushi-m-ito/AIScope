
#include "fad_writer.h"
#pragma warning ( disable : 4996 )


#define FAD_FLAG_J		(0x1)		//エネルギー流データがある.
#define FAD_FLAG_SIMBOX	(0x2)	//simboxの指定がある.
#define FAD_FLAG_NONORTHOGONALBOX	(0x20)	//simboxが非直交軸である.
#define FAD_FLAG_TEMPERATURE		(0x4)	//温度情報がある.
#define FAD_FLAG_STEP		(0x8)		//int4型のstep数情報がある.
#define FAD_FLAG_ATOMEXT		(0x10)		//粒子のidの配列がある.


FAD_Writer::FAD_Writer(){
	m_fp = NULL;
	m_buffer4 = NULL;
		
}

FAD_Writer::~FAD_Writer(){
	if(m_fp){fclose(m_fp);}
	delete [] m_buffer4;
}


void FAD_Writer::Open(const char* fileName){
	
	m_fp = fopen(fileName, "wb");
	//m_frameNum = 0;
	m_buffer4 = NULL;
	m_buffer4Sz = 0;
		
}

void FAD_Writer::Close(){
	fclose(m_fp);
	delete [] m_buffer4;
	m_buffer4 = NULL;
	m_fp = NULL;
}


void FAD_Writer::Write(int count, int* knd, int* id, double* r, double* p, double* boxaxisorg){
	int flag = FAD_FLAG_SIMBOX | FAD_FLAG_NONORTHOGONALBOX;
	if( (p != NULL) && (id != NULL) ){
		flag |= FAD_FLAG_ATOMEXT;
	}
	
	int i;
	
	int sz;
	int bcnt = 0;

	
		
		sz = 12;
		fwrite(&sz, 4, 1, m_fp);
	fwrite(&count, 4, 1, m_fp);
	fwrite(&bcnt, 4, 1, m_fp);	//m_bondCnt;
	fwrite(&flag, 4, 1, m_fp);
		 fwrite(&sz, 4, 1, m_fp);
		

	//4バイト変換用のバッファー
	sz = bcnt;
	if (sz < count){ sz = count;}
	if(m_buffer4Sz < sz){
		delete [] m_buffer4;
		m_buffer4Sz = sz;
		m_buffer4 = new float[m_buffer4Sz];
	}
	int* buf4i = (int*)m_buffer4;;


		sz = 4*count;
		fwrite(&sz, 4, 1, m_fp);
	fwrite(knd, sz, 1, m_fp);
		fwrite(&sz, 4, 1, m_fp);

	if( (p != NULL) && (id != NULL) ){
			fwrite(&sz, 4, 1, m_fp);
		fwrite(id, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);
	}

	for( i = 0; i < count; i++){
		m_buffer4[i] = (float)r[i*3];
	}
			sz = 4*count;
			fwrite(&sz, 4, 1, m_fp);
	fwrite(m_buffer4, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);
		
		
	for( i = 0; i < count; i++){
		m_buffer4[i] = (float)r[i*3+1];
	}
			sz = 4*count;
			fwrite(&sz, 4, 1, m_fp);
	fwrite(m_buffer4, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);

	for( i = 0; i < count; i++){
		m_buffer4[i] = (float)r[i*3+2];
	}
			sz = 4*count;
			fwrite(&sz, 4, 1, m_fp);
	fwrite(m_buffer4, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);
	

	if( (p != NULL) && (id != NULL) ){
		
		//momentum
		for( i = 0; i < count; i++){
			m_buffer4[i] = (float)p[i*3];
		}
				sz = 4*count;
				fwrite(&sz, 4, 1, m_fp);
		fwrite(m_buffer4, sz, 1, m_fp);
				fwrite(&sz, 4, 1, m_fp);

		for( i = 0; i < count; i++){
			m_buffer4[i] = (float)p[i*3+1];
		}
				sz = 4*count;
				fwrite(&sz, 4, 1, m_fp);
		fwrite(m_buffer4, sz, 1, m_fp);
				fwrite(&sz, 4, 1, m_fp);

		for( i = 0; i < count; i++){
			m_buffer4[i] = (float)p[i*3+2];
		}
				sz = 4*count;
				fwrite(&sz, 4, 1, m_fp);
		fwrite(m_buffer4, sz, 1, m_fp);
				fwrite(&sz, 4, 1, m_fp);

	}


		
	float boxf[12];
	for(int i = 0; i < 12; i++){
		boxf[i] = (float)boxaxisorg[i];
	}
	sz = 4 * 12;
		fwrite(&sz, 4, 1, m_fp);
	fwrite(boxf, sz, 1, m_fp);
		fwrite(&sz, 4, 1, m_fp);

//	fflush(m_fp);
}


void FAD_Writer::Write(int count, int* knd, int* id, float* r, float* p, float* boxaxisorg){
	int flag = FAD_FLAG_SIMBOX | FAD_FLAG_NONORTHOGONALBOX;
	if( (p != NULL) && (id != NULL) ){
		flag |= FAD_FLAG_ATOMEXT;
	}
	
	int i;
	
	int sz;
	int bcnt = 0;

	
		
		sz = 12;
		fwrite(&sz, 4, 1, m_fp);
	fwrite(&count, 4, 1, m_fp);
	fwrite(&bcnt, 4, 1, m_fp);	//m_bondCnt;
	fwrite(&flag, 4, 1, m_fp);
		 fwrite(&sz, 4, 1, m_fp);
		

	//4バイト変換用のバッファー
	sz = bcnt;
	if (sz < count){ sz = count;}
	if(m_buffer4Sz < sz){
		delete [] m_buffer4;
		m_buffer4Sz = sz;
		m_buffer4 = new float[m_buffer4Sz];
	}
	int* buf4i = (int*)m_buffer4;;


		sz = 4*count;
		fwrite(&sz, 4, 1, m_fp);
	fwrite(knd, sz, 1, m_fp);
		fwrite(&sz, 4, 1, m_fp);

	if( (p != NULL) && (id != NULL) ){
			fwrite(&sz, 4, 1, m_fp);
		fwrite(id, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);
	}

	for( i = 0; i < count; i++){
		m_buffer4[i] = (float)r[i*3];
	}
			sz = 4*count;
			fwrite(&sz, 4, 1, m_fp);
	fwrite(m_buffer4, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);
		
		
	for( i = 0; i < count; i++){
		m_buffer4[i] = (float)r[i*3+1];
	}
			sz = 4*count;
			fwrite(&sz, 4, 1, m_fp);
	fwrite(m_buffer4, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);

	for( i = 0; i < count; i++){
		m_buffer4[i] = (float)r[i*3+2];
	}
			sz = 4*count;
			fwrite(&sz, 4, 1, m_fp);
	fwrite(m_buffer4, sz, 1, m_fp);
			fwrite(&sz, 4, 1, m_fp);
	

	if( (p != NULL) && (id != NULL) ){
		
		//momentum
		for( i = 0; i < count; i++){
			m_buffer4[i] = (float)p[i*3];
		}
				sz = 4*count;
				fwrite(&sz, 4, 1, m_fp);
		fwrite(m_buffer4, sz, 1, m_fp);
				fwrite(&sz, 4, 1, m_fp);

		for( i = 0; i < count; i++){
			m_buffer4[i] = (float)p[i*3+1];
		}
				sz = 4*count;
				fwrite(&sz, 4, 1, m_fp);
		fwrite(m_buffer4, sz, 1, m_fp);
				fwrite(&sz, 4, 1, m_fp);

		for( i = 0; i < count; i++){
			m_buffer4[i] = (float)p[i*3+2];
		}
				sz = 4*count;
				fwrite(&sz, 4, 1, m_fp);
		fwrite(m_buffer4, sz, 1, m_fp);
				fwrite(&sz, 4, 1, m_fp);

	}


		
	float boxf[12];
	for(int i = 0; i < 12; i++){
		boxf[i] = (float)boxaxisorg[i];
	}
	sz = 4 * 12;
		fwrite(&sz, 4, 1, m_fp);
	fwrite(boxf, sz, 1, m_fp);
		fwrite(&sz, 4, 1, m_fp);

//	fflush(m_fp);
}

