#ifndef FA2_WRITER_H
#define FA2_WRITER_H


#include <stdlib.h>
#include <stdio.h>



class FA2_Writer
{
private:
	FILE* m_fp;
	float* m_buffer4;
	
public:


	FA2_Writer();
	virtual ~FA2_Writer();

	int Open(const char* fileName);
	int Append(const char* fileName);
	void Close();

	void Write(int count, int* knd, int* id, double* r, double* p, double* boxaxisorg);
	//void Write(long long count, int* knd, int* id, double* r, double* p, double* boxaxisorg);
	void Write(int count, int* knd, int* id, float* r, float* p, float* boxaxisorg);

};



#endif    // !FA2_WRITER_H
