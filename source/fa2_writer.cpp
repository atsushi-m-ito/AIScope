
#pragma warning ( disable : 4996 )
#include "fa2_writer.h"
#include <stdint.h>
#include <string.h>



FA2_Writer::FA2_Writer(){
	m_fp = NULL;
		
}

FA2_Writer::~FA2_Writer(){
	this->Close();
}


int FA2_Writer::Open(const char* fileName){
	
	m_fp = fopen(fileName, "wb");
	
	if(m_fp){
		return 1;
	}else{
		return 0;
	}

}


int FA2_Writer::Append(const char* fileName){
	
	m_fp = fopen(fileName, "ab");
	
	if(m_fp){
		return 1;
	}else{
		return 0;
	}

}

void FA2_Writer::Close(){
	if(m_fp){fclose(m_fp);}
	m_fp = NULL;
}


void FA2_Writer::Write(int count, int* knd, int* id, double* r, double* p, double* boxaxisorg){
	
	//header////////////////////////////////////////////////////
	int8_t header[32] = {0};
	int64_t count64 = (int64_t)count;
	memcpy(header, &count64, sizeof(int64_t));
	
	//unit size of box//
	if(boxaxisorg){	header[8] = sizeof(float);}
	
	//unit size of kind//
	if(knd){ header[9] = sizeof(int8_t);	}

	//unit size of id//
	if(id){	header[10] = sizeof(int); }

	//unit size of r//
	if(r){ header[11] = sizeof(float); }

	//unit size of p//
	if(p){ header[12] = sizeof(float);}

	//unit size of f//
	//if(f){ header[13] = sizeof(float); }

	
	//memcpy(header + 16, &step, sizeof(double));

	fwrite(header, 32 * sizeof(int8_t), 1, m_fp);

	
	if(boxaxisorg){
		float boxf[12];
		for(int i = 0; i < 12; i++){
			boxf[i] = (float)boxaxisorg[i];
		}
		fwrite(boxf, sizeof(float)*12, 1, m_fp);
	}


	int8_t* buffer = new int8_t[count * sizeof(float)*3];

	if(knd){
		int8_t* pknd = buffer;
		for(int i = 0; i < count; i++){
			pknd[i] = (int8_t)(knd[i]);
		}
		fwrite(pknd, count * sizeof(int8_t), 1, m_fp);
	}

	if(id){
		/*
		int* pint = (int*)buffer;
		for(int i = 0; i < count; i++){
			pint[i] = id[i];
		}
		fwrite(pint, count * header[0], 1, m_fp);
		*/
		fwrite(id, count * sizeof(int), 1, m_fp);
		
	}


	if(r){
		float* pr = (float*)buffer;
		for(int i = 0; i < count * 3; i++){
			pr[i] = (float)(r[i]);
		}
		fwrite(pr, count * sizeof(float)*3, 1, m_fp);
	}

	if(p){
		float* pp = (float*)buffer;
		for(int i = 0; i < count * 3; i++){
			pp[i] = (float)(p[i]);
		}
		fwrite(pp, count * sizeof(float)*3, 1, m_fp);
	}

	/*
	if(r){
		float* pf = (float*)buffer;
		for(int i = 0; i < count * 3; i++){
			pf[i] = (float)(f[i]);
		}
		fwrite(pf, count * sizeof(float)*3, 1, m_fp);
	}
	*/

#ifdef _DEBUG
	long long filepointer_cur = _ftelli64(m_fp);
	printf("offset = %d\n", filepointer_cur);
#endif

	delete [] buffer;

}


void FA2_Writer::Write(int count, int* knd, int* id, float* r, float* p, float* boxaxisorg){
	
	//header////////////////////////////////////////////////////
	int8_t header[32] = {0};
	int64_t count64 = (int64_t)count;
	memcpy(header, &count64, sizeof(int64_t));
	
	//unit size of box//
	if(boxaxisorg){	header[8] = sizeof(float);}
	
	//unit size of kind//
	if(knd){ header[9] = sizeof(int8_t);	}

	//unit size of id//
	if(id){	header[10] = sizeof(int); }

	//unit size of r//
	if(r){ header[11] = sizeof(float); }

	//unit size of p//
	if(p){ header[12] = sizeof(float);}

	//unit size of f//
	//if(f){ header[13] = sizeof(float); }

	
	//memcpy(header + 16, &step, sizeof(double));

	fwrite(header, 32 * sizeof(int8_t), 1, m_fp);

	
	if(boxaxisorg){
		float boxf[12] = {0.0f};
		for(int i = 0; i < 9; i++){
			boxf[i] = (float)boxaxisorg[i];
		}
		fwrite(boxf, sizeof(float)*12, 1, m_fp);
	}


	int8_t* buffer = new int8_t[count * sizeof(float)*3];

	if(knd){
		int8_t* pknd = buffer;
		for(int i = 0; i < count; i++){
			pknd[i] = (int8_t)(knd[i]);
		}
		fwrite(pknd, count * sizeof(int8_t), 1, m_fp);
	}

	if(id){
		/*
		int* pint = (int*)buffer;
		for(int i = 0; i < count; i++){
			pint[i] = id[i];
		}
		fwrite(pint, count * header[0], 1, m_fp);
		*/
		fwrite(id, count * sizeof(int), 1, m_fp);
		
	}


	if(r){
		fwrite(r, count * sizeof(float)*3, 1, m_fp);
	}

	if(p){
		fwrite(p, count * sizeof(float)*3, 1, m_fp);
	}

	
#ifdef _DEBUG
	long long filepointer_cur = _ftelli64(m_fp);
	printf("offset = %d\n", filepointer_cur);
#endif

	delete [] buffer;

}

