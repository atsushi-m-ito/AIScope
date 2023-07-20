
#include "io64.h"


FILE64* fopen64(const TCHAR* filename, const TCHAR* b) {
#if defined(_WIN32) //win
	HANDLE hFile;
	if((b[0] == _T('r')) && (b[1] == _T('b')) && (b[2] == _T('\0'))){
		hFile = CreateFile(filename,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
					NULL);
		if (hFile != INVALID_HANDLE_VALUE){
			FILE64* fp = new FILE64;
			*fp = hFile;
			return fp;
		}
	}else if((b[0] == _T('w')) && (b[1] == _T('b')) && (b[2] == _T('\0'))){
		hFile = CreateFile(filename,
					GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile != INVALID_HANDLE_VALUE){
			FILE64* fp = new FILE64;
			*fp = hFile;
			return fp;
		}
	}
	return NULL;
#else	//unix
	return fopen(filename,b);
#endif
}

int fclose64(FILE64* fp) {
#if defined(_WIN32) //win
	BOOL res = CloseHandle(*fp);
	if (res){
		delete fp;
		return 0;
	}else{
		return EOF;
	}
#else	//unix
	return fclose(fp);
#endif
}


size_t fread64(void* buf, size_t sz, size_t cnt, FILE64* fp){
#if defined(_WIN32) //win
	BOOL res;
	DWORD dd;
	res = ReadFile(*fp, buf, sz * cnt, &dd, NULL);
	if (res){
		return ((size_t)dd) / ((size_t)sz);
	}else{
		return 0;
	}
#else	//unix
	return fread(buf, sz, cnt, fp);
#endif
}


size_t fwrite64(const void* buf, size_t sz, size_t cnt, FILE64* fp){
#if defined(_WIN32) //win
	BOOL res;
	DWORD dd;
	res = WriteFile(*fp, buf, sz * cnt, &dd, NULL);
	if (res){
		return ((size_t)dd) / ((size_t)sz);
	}else{
		return 0;
	}
#else	//unix
	return fwrite(buf, sz, cnt, fp);
#endif
}

int fseek64(FILE64* fp, long64 l, int i){
#if defined(_WIN32) //win
	LONG lh[2];
	
	memcpy(lh, &l, sizeof(long64));	
	lh[0];
	lh[1];
	switch (i){
	case SEEK_CUR:
		lh[0] = SetFilePointer(*fp, lh[0], lh + 1, FILE_CURRENT);
		break;
	case SEEK_END:
		lh[0] = SetFilePointer(*fp, lh[0], lh + 1, FILE_END);
		break;
	case SEEK_SET:
		lh[0] = SetFilePointer(*fp, lh[0], lh + 1, FILE_BEGIN);
		break;
	}
	if (lh[0] != -1L){
		return 0;
	}else{
		return -1;
	}
#else	//unix
	return fseek(fp, l, i);
#endif
}


long64 ftell64(FILE64* fp){
#if defined(_WIN32) //win
	LONG lh[2];
	lh[0] = 0;
	lh[1] = 0;

	lh[0] = SetFilePointer(*fp, lh[0], lh + 1, FILE_CURRENT);
	if (lh[0] != -1L){
		return *((LONGLONG*)lh);
	}else{
		return -1I64;
	}
#else	//unix
	return ftell(fp);
#endif
}


