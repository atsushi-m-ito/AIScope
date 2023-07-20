#pragma once
#include <stdio.h>
#include <stdint.h>


#ifndef _WINGDI_
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;


typedef struct tagBITMAPFILEHEADER { // bmfh 
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER { // bmih 
	DWORD  biSize;
	LONG   biWidth;
	LONG   biHeight;
	WORD   biPlanes;
	WORD   biBitCount;
	DWORD  biCompression;
	DWORD  biSizeImage;
	LONG   biXPelsPerMeter;
	LONG   biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD { // rgbq 
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO { // bmi 
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          bmiColors[1];
} BITMAPINFO;

#endif // _WINGDI_

class WriteBMP {
private:
	BYTE* m_buffer;
	
	int m_width;
	int m_height;
	int m_color_depth;
	int m_stride;

	BITMAPINFOHEADER m_bmih;



public:
	WriteBMP(int width, int height, int color_depth);
	~WriteBMP();

private:

	void writeBMH(BITMAPFILEHEADER *lpbmfh, BITMAPINFOHEADER *lpbmih, FILE *fp) {

		fwrite(&(lpbmfh->bfType), sizeof(WORD), 1, fp);
		fwrite(&(lpbmfh->bfSize), sizeof(DWORD), 1, fp);
		fwrite(&(lpbmfh->bfReserved1), sizeof(WORD), 1, fp);
		fwrite(&(lpbmfh->bfReserved2), sizeof(WORD), 1, fp);
		fwrite(&(lpbmfh->bfOffBits), sizeof(DWORD), 1, fp);

		fwrite(&(lpbmih->biSize), sizeof(DWORD), 1, fp);
		fwrite(&(lpbmih->biWidth), sizeof(LONG), 1, fp);
		fwrite(&(lpbmih->biHeight), sizeof(LONG), 1, fp);
		fwrite(&(lpbmih->biPlanes), sizeof(WORD), 1, fp);
		fwrite(&(lpbmih->biBitCount), sizeof(WORD), 1, fp);
		fwrite(&(lpbmih->biCompression), sizeof(DWORD), 1, fp);
		fwrite(&(lpbmih->biSizeImage), sizeof(DWORD), 1, fp);
		fwrite(&(lpbmih->biXPelsPerMeter), sizeof(LONG), 1, fp);
		fwrite(&(lpbmih->biYPelsPerMeter), sizeof(LONG), 1, fp);
		fwrite(&(lpbmih->biClrUsed), sizeof(DWORD), 1, fp);
		fwrite(&(lpbmih->biClrImportant), sizeof(DWORD), 1, fp);
	};

	/*
	sRGBへ変換
	*/
	BYTE sRGBFromFP16(const void* fp16);
	float FP32FromFP16(const void* fp16);

public:
	void CreateDIBfromRGBA(BYTE* rgbaBuf, int src_pitch, int offset_x, int offset_y, int w, int h);
	void CreateDIBfromRGBA_FP16(BYTE* rgbaBuf, int src_pitch, int offset_x, int offset_y, int w, int h);
	void WriteDIB(FILE *fp);
};


