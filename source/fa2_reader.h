#ifndef FA2_READER_H
#define FA2_READER_H


//#include <stdint.h>
#include "io64w2l.h"

#ifndef BYTE
typedef unsigned char uint8_t;
#endif

struct FA2_DATASIZE{
    int box_size;
    int kind_size;
    int id_size;
    int r_size;
    int p_size;
};

class FA2_Reader
{
private:
	FILE* m_fp;
	int m_state;
	int m_readcount;
	long long m_current_frame_pos;	//カレントフレームの先頭ファイルポンタ//
	uint8_t m_header[32];
	
public:


	FA2_Reader();
	virtual ~FA2_Reader();

	int Open(const char* fileName);
	//int Append(const char* fileName);
	void Close();
	
	long long GetFilePointer();
	void SetFilePointer(long long offset);

	int GetParticleCount();
    int GetInfo(FA2_DATASIZE* data_size);

	//the function is enable only after call GetParticleCount//
	void Read(int* knd, int* id, double* r, double* p, double* boxaxisorg);
	//
	void ReadF(int* knd, int* id, float* r, float* p, float* boxaxisorg);
	
	int ReadPart(int count, int* knd, int* id, double* r, double* p, double* boxaxisorg);
	
	//the function is enable only after call GetParticleCount//
	void SkipFrame();
	

};



#endif    // !FA2_READER_H
