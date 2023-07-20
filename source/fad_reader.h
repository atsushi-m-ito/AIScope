
#ifndef __fad_reader_h__
#define __fad_reader_h__

#pragma once

#include "io64w2l.h"

#include <stdio.h>

class FAD_Reader
{
private:
	FILE* m_fp;	//ファイル
	long m_pos_after_head;	//ヘッダ後のファイルポインタ
	int m_state;
	union{
		int m_header[5];
		struct{
			int m_dummy1;
			int m_pcnt;
			int m_bcnt;
			int m_flag;
			int m_dummy2;
		};
	};

	//float* m_buffer4;	//floatからdoubleへの変換用バッファ.
	//int m_buffer4_sz;

	//long m_current_header_pos;

//	int m_current_frame;	//現在のフレーム(Open時は0番)
	//long m_nextheaderpos;		//現在のフレームのヘッダーの位置
	/*
	//内部関数
	int _ReadHeader(int *p_pcnt, int *p_bcnt, int *p_flag);

	int _Read(int count, int offset, int* knd, int* id, double* r, double* rx, double* ry, double* rz,
					  double* p, double* px, double* py, double* pz, double* boxaxis);
	
	void ReadFloat3AsVector3D(long filepos, int read_count, int offset, double* v3d);
	void ReadFloatAsDouble(long filepos, int read_count, int offset, double* xxx);
	*/

public:


	FAD_Reader();
	virtual ~FAD_Reader();

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
	
	long long GetFilePointer();
	void SetFilePointer(long long offset);

	////////////////////////////////////////////////////////////////////////////////
	//ファイルを読み込みして必要なデータを返す.
	//
	int GetParticleCount();
	//
	//	粒子数を返す.
	//	Readの後にこの関数を呼ぶと次のフレームに進める.
	//
	//
	///////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	//ファイルを読み込みして必要なデータを返す.
	//
	void Read(int* knd, int* id, double* r, double* p, double* boxaxis);
	void ReadF(int* knd, int* id, float* r, float* p, float* boxaxis);
	//int Read(int count, int offset, int* knd, int* id, double* r, double* p, double *boxaxis);
	//int Read(int count, int offset, int* knd, int* id, double* rx, double* ry, double* rz, double* px, double* py, double* pz, double *boxaxis);
	//
	//	戻り値には読み込んだ粒子数を返す.
	//	第一引数に確保したバッファサイズを粒子数(byteサイズではない)で指定する.
	//	第二引数は読み飛ばす粒子数. offset = 4 なら先頭から4粒子を読み飛ばしてそこからcount数の粒子を読む.
	//	第三引数以降はNULLにすることで読み飛ばすことができる.
	//
	///////////////////////////////////////////////////////////////////////////////
	
	void SkipFrame();
};






#endif	//__fad_reader_h__
