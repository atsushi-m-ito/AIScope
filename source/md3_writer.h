#pragma once 
#ifndef md3_writer2_h
#define md3_writer2_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec3.h"
#include "atomic_number.h"


class MD3_Writer
{
private:
	FILE* m_fp;	//ファイル
	int m_frameno;	
public:


	MD3_Writer();
	virtual ~MD3_Writer();

	///////////////////////////////////////////////////////
	//ファイルをオープンする.
	//
	int Open(const char* filepath);
	//
	//	正しくオープンできたときは 0 を返す.
	//	既に別のファイルをオープン済みの場合は -2 を返す.
	//	指定したファイルが見つからないときは -1 を返す.
	//	読み込んだデータが壊れているとき -3を返す.
	//	
	/////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////
	//ファイルを閉じる.
	//
	void Close();
	//
	//	delete してもこの関数が自動で呼ばれる.
	//
	///////////////////////////////////////////////////////////

	
	////////////////////////////////////////////////////////////////////////////////
	//ファイルを読み込みして必要なデータを返す.
	//
	int Write(int count, const int* knd, const double* r, const double* f, const double *boxaxis, const char* comment);
	int Write_f(int count, const int* knd, const float* r, const float* f, const float *boxaxis, const char* comment);
	//
	//	戻り値にはフレーム数を返す.
	//	第一引数に確保したバッファサイズを粒子数(byteサイズではない)で指定する.
	//	第二引数は読み飛ばす粒子数. offset = 4 なら先頭から4粒子を読み飛ばしてそこからcount数の粒子を読む.
	//	第三引数以降はNULLにすることで読み飛ばすことができる.
	//
	///////////////////////////////////////////////////////////////////////////////
	

};






#endif	//__md3_writer_h__
