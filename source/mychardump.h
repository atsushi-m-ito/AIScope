
#ifndef __mbsdup_h__
#define __mbsdup_h__

#pragma once


#include <stdio.h>
#include "targetver.h"
#include <windows.h>


//マルチバイト文字からTCHAR文字へ変更
inline char* MbsDup(const TCHAR* tstr){

#ifdef _UNICODE

    //必要バッファサイズの取得(NULL終端込みのサイズが帰る)//
    int mbsz = WideCharToMultiByte(CP_ACP, 0, tstr, -1, NULL, 0, NULL, NULL);


    char* mbstr = new char[mbsz];
    //変換(NULL終端もしてくれる)
    WideCharToMultiByte(CP_ACP, 0, tstr, -1, mbstr, mbsz, NULL, NULL);

#else
    char* mbstr = _strdup(tstr);
#endif			


    return mbstr;
}

template <typename TOUT, typename TIN>
inline TOUT* MyCharDump(const TIN* tstr);

template<> inline char* MyCharDump<char, char>(const char* tstr)
{
    return _strdup(tstr);
}

template<> inline wchar_t* MyCharDump<wchar_t, wchar_t>(const wchar_t* tstr)
{
    return _wcsdup(tstr);
}

//UTF16からSHIFT-JISへ変換//
template<> inline char* MyCharDump<char, wchar_t>(const wchar_t* tstr)
{

    int mbsz = WideCharToMultiByte(CP_ACP, 0, tstr, -1, NULL, 0, NULL, NULL);


    char* mbstr = new char[mbsz];
    //変換(NULL終端もしてくれる)
    WideCharToMultiByte(CP_ACP, 0, tstr, -1, mbstr, mbsz, NULL, NULL);
    return mbstr;
}


//SHIFT-JISからUTF16へ変換//
template<> inline wchar_t* MyCharDump<wchar_t, char>(const char* tstr)
{

    int sz = MultiByteToWideChar(CP_ACP, 0, tstr, -1, NULL, 0);


    wchar_t* wstr = new wchar_t[sz];
    //変換(NULL終端もしてくれる)
    MultiByteToWideChar(CP_ACP, 0, tstr, -1, wstr, sz);
    return wstr;
}


#endif	//!__mbsdup_h__
