
#ifdef _OPENMP
#include <omp.h>
#endif 

#include "vec3.h"

////////////////////////////////////////////////////////////
//openmp用スレッド間共有配列の作成
//
//	共有メモリを一つ確保し、各privateなポインタ変数に同じ値をセット
//	openmpコンパイル時は必ずomp parallel内で呼び出すこと.
//	openmp未使用時も呼び出し可能.
//
///////////////////////////////////////////////////////////

//共有メモリの作成

template <class T>
void omptuneCreateSharedArray(T** pbuffer, int size){
	T* p;	
	#pragma omp single copyprivate(p)	
	{
		p = new T[size];
	}//copyprivateの宣言によりここで全スレッドのpに値がコピーされる.
	*pbuffer = p;
}

//共有メモリの解放
template <class T>
void omptuneDeleteSharedArray(T** pbuffer){
	#pragma omp barrier		//別スレッドが共有メモリにアクセスしているのを防ぐ
	#pragma omp master
	{
		delete [] (*pbuffer);
	}
	*pbuffer = NULL;
}


//////////////////////////////////////////////////////////////////////
//openmp用 private変数のreduction
//
//	スレッド毎に異なるprivate変数を足し合わせて返す.
//
//
////////////////////////////////////////////////////////////////////////////
/*
void Gather_i(int* pvalue){
	#pragma omp master
	{
		shared_pointer = new int*[omp_get_num_threads()];
	}
	#pragma omp barrier		//別スレッドがshared_pointerにアクセスしているのを防ぐ
	int** p = (int**)shared_pointer;
	p += omp_get_thread_num();
	*p = pvalue;
	#pragma omp barrier
	#pragma omp master
	{
		int sum = 0;
		for(int t = 0; t < omp_get_num_threads(); t++){
			sum += *(p[t]);
		}
		for(int t = 0; t < omp_get_num_threads(); t++){
			*(p[t]) = sum;
		}
	}
	#pragma omp barrier
	#pragma omp master
	{
		delete [] shared_pointer;
	}
	
}
*/
template <class T>
void omptuneReductionSum(T* pvalue){
	T* p;
	#pragma omp single copyprivate(p)	
	{
		p = new T[omp_get_num_threads()];
	}//copyprivateの宣言によりここで全スレッドのpに値がコピーされる.
	//single後はdefaultでスレッド同期のwaitがある.

	p[omp_get_thread_num()] = *pvalue;
	#pragma omp barrier
	
	T sum = {0};
	#pragma omp single copyprivate(sum)
	{
		for(int t = 0; t < omp_get_num_threads(); t++){
			sum += *(p + t);
		}
		delete [] p;
	}//copyprivateの宣言によりここで全スレッドのsumに値がコピーされる.
	//single後はdefaultでスレッド同期のwaitがある.

	*pvalue = sum;
}


/*コンパイル通らない
template <class T>
void omptuneReductionSum(T* pvalue){
	T sum;
	#pragma omp for schedule(static) reduction(+:sum)
	for(int t = 0; t < omp_get_num_threads(); t++){
		sum = *pvalue;
	}

	*pvalue = sum;
}
*/