#ifndef __IO64__
#define __IO64__

#include <stdio.h>


#if defined(_WIN32) || defined(_WIN64)

#include "targetver.h"
#include <windows.h>
#include <winbase.h>
typedef HANDLE FILE64;
typedef LONGLONG long64;
#define LONG64_ZERO 0I64
#define LONG64_FULL -1I64
#include <tchar.h>

#else //unix
typedef FILE FILE64;
typedef long long64;
#define LONG64_ZERO 0L
#define LONG64_FULL -1L

typedef char TCHAR;
#endif


FILE64* fopen64(const TCHAR* filename, const TCHAR* b);
int fclose64(FILE64* fp);
size_t fread64(void* buf, size_t sz, size_t cnt, FILE64* fp);
size_t fwrite64(const void* buf, size_t sz, size_t cnt, FILE64* fp);
int fseek64(FILE64* fp, long64 l, int i);
long64 ftell64(FILE64* fp);

#endif    // __IO64__