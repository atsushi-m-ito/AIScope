#ifndef __windib_h__
#define __windib_h__

/*------------------------------------------------
	windows環境以外でBitMapファイルを作成するためのヘッダー
	TCHARを利用する
-------------------------------------------------------*/
#include <stdio.h>
#include <memory.h>

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

typedef struct tagBITMAPINFOHEADER{ // bmih 
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




const DWORD SZ_BMFH = 3 * sizeof(WORD) + 2 * sizeof(DWORD);
const DWORD SZ_BMIH = 2 * sizeof(WORD) + 5 * sizeof(DWORD)
				 + 4 * sizeof(LONG);


void writeDIB24(FILE *fp, int w, int h, BYTE* bmBuf);
void convRGBtoDIB(int w, int h, BYTE *bmBuf);
int readDIB(FILE *fp, BITMAPINFOHEADER* pbmih, BYTE** bmBuf);
int convDIBtoRGB(BITMAPINFOHEADER* pbmih, BYTE *bmBuf);
int convRGBtoRGBA(BITMAPINFOHEADER* pbmih, BYTE *bmBuf, BYTE **newBuf,BYTE red, BYTE green, BYTE blue, BYTE alpha);
unsigned char* arrangeTexture(unsigned char* buf, int colorbyte, int width, int height, int* retW, int* retH);

BYTE* convDIBtoGLTEX(BITMAPINFOHEADER* pbmih, BYTE *bmBuf, int destbyte, int* retW, int* retH);
int setGLTEXAlpha(BYTE *buf, int bufsize, BYTE red, BYTE green, BYTE blue, BYTE alpha);

BYTE* CreateDIBfromRGBA(BYTE* rgbaBuf, int src_pitch, int w, int h, BITMAPINFOHEADER* pbmih);
BYTE* CreateDIBfromRGBA_FP16(BYTE* rgbaBuf, int src_pitch, int w, int h, BITMAPINFOHEADER* pbmih);
BYTE* CreateDIBfromRGBA(BYTE* rgbaBuf, int w, int h, BITMAPINFOHEADER* pbmih);
BYTE* CreateDIBfromRGB(BYTE* rgbBuf, int w, int h, BITMAPINFOHEADER* pbmih);
void writeDIB(FILE *fp, BITMAPINFOHEADER* pbmih, BYTE* bmBuf);

#endif    // __windib_h__