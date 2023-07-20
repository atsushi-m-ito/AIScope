#ifndef JOBPARAM_H
#define JOBPARAM_H

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>


class JobParam{
private:
	std::map<std::string, const char*> m_key_values;
	std::map<std::string, std::vector<const char*> > m_key_multilines;
	
	char* block_begin_head;
	char* block_begin_foot;
	char* block_end_head;
	char* block_end_foot;

public:
	JobParam(const char* input_file_path, const char* block_begen_word, const char* block_end_word);
	~JobParam();

	int GetInt(const char* key);
	double GetDouble(const char* key);
	int GetIntArray(const char* key, int* buffer, int size);
	int GetDoubleArray(const char* key, double* buffer, int size);
	const char* GetString(const char* key);
	char* DumpString(const char* key);
    int IsEqualString(const char* key, const char* compared_str);
	//int GetStringArray(const char* key, const char** buffer, int size);
	const char* GetBlockString(const char* key, const int line);
	int GetBlockNumLines(const char* key);

	int Find(const char* key);

	int PrintAll(FILE* fp);

    //ver.2機能/////////////////////////////////////////
    int SetInt(const char* key, int value);
    int SetDouble(const char* key, double value);
    int SetIntArray(const char* key, const int* buffer, int size);
    int SetDoubleArray(const char* key, const double* buffer, int size);
    int SetString(const char* key, const char* text);
    int AddBlockString(const char* key, const char* text);
    int Erase(const char* key);


};


#endif	//!JOBPARAM_H
