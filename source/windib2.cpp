#include <stdio.h>
#include <math.h>
#include "windib2.h"

void writeBMH(BITMAPFILEHEADER *lpbmfh,BITMAPINFOHEADER *lpbmih,FILE *fp){
	
	fwrite(&(lpbmfh->bfType),		sizeof(WORD), 1, fp);
	fwrite(&(lpbmfh->bfSize),		sizeof(DWORD), 1, fp);
	fwrite(&(lpbmfh->bfReserved1),	sizeof(WORD), 1, fp);
	fwrite(&(lpbmfh->bfReserved2),	sizeof(WORD), 1, fp);
	fwrite(&(lpbmfh->bfOffBits),	sizeof(DWORD), 1, fp);
	
	fwrite(&(lpbmih->biSize),			sizeof(DWORD), 1, fp);
	fwrite(&(lpbmih->biWidth),			sizeof(LONG), 1, fp);
	fwrite(&(lpbmih->biHeight),			sizeof(LONG), 1, fp);
	fwrite(&(lpbmih->biPlanes),			sizeof(WORD), 1, fp);
	fwrite(&(lpbmih->biBitCount),		sizeof(WORD), 1, fp);
	fwrite(&(lpbmih->biCompression),	sizeof(DWORD), 1, fp);
	fwrite(&(lpbmih->biSizeImage),		sizeof(DWORD), 1, fp);
	fwrite(&(lpbmih->biXPelsPerMeter),	sizeof(LONG), 1, fp);
	fwrite(&(lpbmih->biYPelsPerMeter),	sizeof(LONG), 1, fp);
	fwrite(&(lpbmih->biClrUsed),		sizeof(DWORD), 1, fp);
	fwrite(&(lpbmih->biClrImportant),	sizeof(DWORD), 1, fp);
}

void writeDIB24(FILE *fp,int w, int h, BYTE* bmBuf){
	int size_Color = 3;
	int size_Line;
	int i;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	
	size_Line = w * size_Color;
	if (size_Line % 4){	size_Line += 4 - (size_Line % 4);}
	

	bmfh.bfType = 0x4D42;
	bmfh.bfSize = SZ_BMFH + SZ_BMIH + sizeof(BYTE)*size_Line*h;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = SZ_BMFH+SZ_BMIH;
	
	bmih.biSize = SZ_BMIH;
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1; 
    bmih.biBitCount = 24; 
    bmih.biCompression = 0; 
    bmih.biSizeImage = sizeof(BYTE)*size_Line*h; 
    bmih.biXPelsPerMeter = 0; 
    bmih.biYPelsPerMeter = 0; 
    bmih.biClrUsed = 0; 
    bmih.biClrImportant = 0; 
	
	writeBMH(&bmfh, &bmih, fp);
	
	for(i=0;i<h;i++){
		fwrite(bmBuf + i*size_Line, sizeof(BYTE), size_Line, fp);
	}
	
	return;
}

void convRGBtoDIB(int w, int h, BYTE *bmBuf){
	//ラインデータを4の倍数とする
	//色の並びをRGBからBGRに変える
	
	int size_Color = 3;
	int size_Line;
	int i,ilim,k,klim;
	BYTE b;
	
	size_Line = w * size_Color;
	if (size_Line % 4){	size_Line += 4 - (size_Line % 4);}
		
	klim = size_Line * h;
	for(k = 0; k < klim; k += size_Line){
		ilim = k + w * size_Color;	
		for(i = k; i < ilim; i += size_Color){
			b = bmBuf[i];
			bmBuf[i] = bmBuf[i+2];
			bmBuf[i+2] = b;
		}
	}

}


void readBMH(BITMAPFILEHEADER *lpbmfh,BITMAPINFOHEADER *lpbmih,FILE *fp){
	
	fread(&(lpbmfh->bfType),		sizeof(WORD), 1, fp);
	fread(&(lpbmfh->bfSize),		sizeof(DWORD), 1, fp);
	fread(&(lpbmfh->bfReserved1),	sizeof(WORD), 1, fp);
	fread(&(lpbmfh->bfReserved2),	sizeof(WORD), 1, fp);
	fread(&(lpbmfh->bfOffBits),	sizeof(DWORD), 1, fp);
	
	fread(&(lpbmih->biSize),			sizeof(DWORD), 1, fp);
	fread(&(lpbmih->biWidth),			sizeof(LONG), 1, fp);
	fread(&(lpbmih->biHeight),			sizeof(LONG), 1, fp);
	fread(&(lpbmih->biPlanes),			sizeof(WORD), 1, fp);
	fread(&(lpbmih->biBitCount),		sizeof(WORD), 1, fp);
	fread(&(lpbmih->biCompression),	sizeof(DWORD), 1, fp);
	fread(&(lpbmih->biSizeImage),		sizeof(DWORD), 1, fp);
	fread(&(lpbmih->biXPelsPerMeter),	sizeof(LONG), 1, fp);
	fread(&(lpbmih->biYPelsPerMeter),	sizeof(LONG), 1, fp);
	fread(&(lpbmih->biClrUsed),		sizeof(DWORD), 1, fp);
	fread(&(lpbmih->biClrImportant),	sizeof(DWORD), 1, fp);
}


int readDIB(FILE *fp, BITMAPINFOHEADER* pbmih, BYTE** bmBuf){
	int datSize, size_Line;
	BITMAPFILEHEADER bmfh;
	//BITMAPINFOHEADER bmih;
	
	//size_Line = w * size_Color;
	//if (size_Line % 4){	size_Line += 4 - (size_Line % 4);}
	
	
	readBMH(&bmfh, pbmih, fp);

	if(bmfh.bfType != 0x4D42) return 0;
	datSize = bmfh.bfSize - bmfh.bfOffBits;
	
	//if(pbmih->biSizeImage != datSize) 
	datSize = pbmih->biSizeImage;
	*bmBuf = new BYTE[datSize];

	size_Line = datSize / (pbmih->biHeight);
	fread(*bmBuf, sizeof(BYTE) * size_Line, pbmih->biHeight, fp);
	
	return datSize;
}


int convDIBtoRGB(BITMAPINFOHEADER* pbmih, BYTE *bmBuf){
	//4の倍数になっているデータをつめる
	//色の並びをBGRからRGBに変える
	int size_Color;
	int size_Line, w, h;
	int i,ilim,k,klim;
	BYTE b, *a;
	
	
	w = pbmih->biWidth;
	h = pbmih->biHeight;
	if(pbmih->biPlanes != 1) return 0;
    if(pbmih->biCompression != 0) return 0;
	
	switch(pbmih->biBitCount){
	case 24:

		size_Color = 3;
		size_Line = (pbmih->biSizeImage) / h;
		
		a = bmBuf;
		klim = size_Line * h;
		for(k = 0; k < klim; k += size_Line){
			ilim = k + w * size_Color;	
			for(i = k; i < ilim; i += size_Color){
				b = bmBuf[i];
				*a = bmBuf[i+2]; a++;
				*a = bmBuf[i+1]; a++;
				*a = b; a++;
			}
		}
		return pbmih->biSizeImage;
		break;
	default:
		return 0;
	}
		
}


int convRGBtoRGBA(BITMAPINFOHEADER* pbmih, BYTE *bmBuf, BYTE **newBuf,BYTE red, BYTE green, BYTE blue, BYTE alpha){
	//24bitRGBカラーバッファから32bitRGBAカラーバッファを作る
	//指定色(red,green,blue)が見つかった場合はalphaに指定値をセット
	//指定色以外にはalphaに255をセット
	//元画像の横幅は4の倍数であってもよいし無くてもよい
	int size_Color;
	int size_Line, w, h;
	int i,ilim,inew,k,klim,knew;
	
	
	w = pbmih->biWidth;
	h = pbmih->biHeight;
	if(pbmih->biPlanes != 1) return 0;
    if(pbmih->biCompression != 0) return 0;
	
	switch(pbmih->biBitCount){
	case 24:

		size_Color = 3;
		size_Line = (pbmih->biSizeImage) / h;
		*newBuf = new BYTE[w * h * 4];

		for(k = 0; k < h; k ++){
			klim = k*size_Line;
			knew = k*w*4;
			for(i = 0; i < w; i ++){
				ilim = i * size_Color + klim;
				inew = i * 4 + knew;
				(*newBuf)[inew] = bmBuf[ilim];
				(*newBuf)[inew+1] = bmBuf[ilim+1];
				(*newBuf)[inew+2] = bmBuf[ilim+2];
				
				if(((*newBuf)[inew] == red) &&
					((*newBuf)[inew+1] == green) &&
					((*newBuf)[inew+2] == blue) ){
					(*newBuf)[inew + 3] = alpha;
				}else{
					(*newBuf)[inew + 3] = 255;
				}
			}
		}
		return pbmih->biSizeImage;
		break;
	default:
		return 0;
	}
		
}

inline int get2n(int num){
	int i = num - 1;
	int new2n = 1;
	while (i){
		i >>= 1;
		new2n <<=1;
	}
	return new2n;
}

unsigned char* arrangeTexture(unsigned char* buf, int colorbyte, int width, int height, int* retW, int* retH){
	unsigned char* newbuf;
	unsigned char* p;
	int i;
	int newW, newH;

	newW = get2n(width);
	*retW = newW;

	newH = get2n(height);
	*retH = newH;

	if((newW == width) && (newH == height)){ return NULL;}

	 
	
	p = newbuf = new unsigned char[newW * newH * colorbyte];
	memset(p,0xff,newW * newH * colorbyte);
	p += (newH - height) * newW * colorbyte;
	
	for (i = 0; i < height; i++){
		memcpy(p, buf, width * colorbyte);
		buf += width * colorbyte;
		p += newW * colorbyte;
	}
	return newbuf;

}

unsigned char* create2nBuffer(int colorbyte, int width, int height, int* retW, int* retH){
	int newW, newH;
	
	newW = get2n(width);
	*retW = newW;
	
	newH = get2n(height);
	*retH = newH;

	return new unsigned char[newW * newH * colorbyte];
}

BYTE* convDIBtoGLTEX(BITMAPINFOHEADER* pbmih, BYTE *bmBuf, int destbyte, int* retW, int* retH){
	//4の倍数になっているデータをつめる
	//縦横幅を2の累乗にする
	//色の並びをBGRからRGBに変える
	int srcbyte;
	int srcline, w, h;
	int i,k, id, is;
	BYTE *ps, *pd, *newBuf;
	//const int destbyte = 3;

	
	
	w = pbmih->biWidth;
	h = pbmih->biHeight;
	if(pbmih->biPlanes != 1) return NULL;
    if(pbmih->biCompression != 0) return NULL;
	
	srcline = (pbmih->biSizeImage) / h;


	switch(pbmih->biBitCount){
	case 24:
		srcbyte = 3;
		break;
	case 32:
		srcbyte = 4;
		break;
	default:
		return NULL;
	}
	
	newBuf = create2nBuffer(destbyte, w, h, retW, retH);
	memset(newBuf,0xff,(*retW) * (*retH) * destbyte);

	for(k = 0; k < h; k ++){
		ps = bmBuf + srcline * (h - 1 - k);
		pd = newBuf + (*retW) * destbyte * k;
		for(i = 0; i < w; i ++){
			id = i * destbyte;
			is = i * srcbyte;
			pd[id] = ps[is+2];
			pd[id+1] = ps[is+1];
			pd[id+2] = ps[is];
		}
		
	}
	return newBuf;
		
}


int setGLTEXAlpha(BYTE *buf, int bufsize, BYTE red, BYTE green, BYTE blue, BYTE alpha){
	//GL_TEXTURE用の32bitRGBAカラーバッファのみを対象とする
	//指定色(red,green,blue)が見つかった場合はalphaに指定値をセット
	int i,changecnt;
	const int colorbyte = 4;

	changecnt = 0;

	for(i = 0; i < bufsize; i += colorbyte){
		if( (buf[i] == red) &&
			(buf[i+1] == green) &&
			(buf[i+2] == blue) ){
				buf[i + 3] = alpha;
				changecnt++;
		}
	}
	
	return changecnt;
		
}

BYTE* CreateDIBfromRGBA(BYTE* rgbaBuf, int src_pitch, int w, int h, BITMAPINFOHEADER* pbmih){
	
	int size_Color = 3;
	int size_Line = w * size_Color;
	if (size_Line % 4){	size_Line += 4 - (size_Line % 4);}

	pbmih->biSize = 40; 
    pbmih->biWidth = w; 
    pbmih->biHeight = h; 
    pbmih->biPlanes = 1; 
    pbmih->biBitCount = 24;
    pbmih->biCompression = 0; 
    pbmih->biSizeImage = size_Line * h; 
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0; 
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0; 


	BYTE *dibBuf = new BYTE[pbmih->biSizeImage];

	
	int rk = h * src_pitch;
	for(int dk = 0; dk < h * size_Line; dk += size_Line){
		rk -= src_pitch;
		for(int i = 0; i < w; i ++){
			int di = i * 3 + dk;
			int ri = i * 4 + rk;
			dibBuf[di] = rgbaBuf[ri + 2];
			dibBuf[di + 1] = rgbaBuf[ri + 1];
			dibBuf[di + 2] = rgbaBuf[ri];
		}
	}

	return dibBuf;

}

BYTE ByteFromFP16(const void* fp16){

    unsigned short *ui16 = (unsigned short *)fp16;
    union{
        unsigned int ui32;
        float fp32;
    };

    //ui32 = (((*ui16) & 0x3FF) << 13) | ((((*ui16) & 0x3C00) >> 10) << 23) | (((*ui16) & 0xC000)<<16);
    //ui32 = (((*ui16) & 0x3FF)<<13) | ((((*ui16) & 0x7C00)>>10) << 26);
    const int shift = (-10 + ((((*ui16) & 0x7C00) >> 10) - 15) + 8);
    if (shift < 0){
        ui32 = (((*ui16) & 0x3FF) | 0x400) >> (-shift);

    } else{
        ui32 = (((*ui16) & 0x3FF) | 0x400) << (shift);
    }

    if (ui32){
        BYTE b = (BYTE)(ui32 - 1);
        return b;
    } else{
        return 0;
    }

}



float FP32FromFP16(const void* fp16){

    unsigned short *ui16 = (unsigned short *)fp16;
    union{
        unsigned int ui32;
        float fp32;
    };

    ui32 = ((unsigned int)((*ui16) & 0x3FF) << 13) | (((unsigned int)(((*ui16) & 0x7C00) >> 10) - 15 + 127) << 23) | ((unsigned int)((*ui16) & 0x8000) << 16);

    return fp32;


}

/*
 sRGBへ変換
*/
BYTE sRGBFromFP16(const void* fp16){
    const float i_gamma = (1.0f / 2.2f);
    const float power = pow(FP32FromFP16(fp16), i_gamma);
    if (power > 1.0f){
        return 255;
    } else{
        return (BYTE)(255.0f * power);
    }
}

BYTE* CreateDIBfromRGBA_FP16(BYTE* rgbaBuf, int src_pitch, int w, int h, BITMAPINFOHEADER* pbmih){

    int size_Color = 3;
    int size_Line = w * size_Color;
    if (size_Line % 4){ size_Line += 4 - (size_Line % 4); }

    pbmih->biSize = 40;
    pbmih->biWidth = w;
    pbmih->biHeight = h;
    pbmih->biPlanes = 1;
    pbmih->biBitCount = 24;
    pbmih->biCompression = 0;
    pbmih->biSizeImage = size_Line * h;
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0;
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0;


    BYTE *dibBuf = new BYTE[pbmih->biSizeImage];


    int rk = h * src_pitch;
    for (int dk = 0; dk < h * size_Line; dk += size_Line){
        rk -= src_pitch;
        for (int i = 0; i < w; i++){
            int di = i * 3 + dk;
            int ri = i * 8 + rk;
            
            dibBuf[di] = sRGBFromFP16(rgbaBuf + ri + 4);
            dibBuf[di + 1] = sRGBFromFP16(rgbaBuf + ri + 2);
            dibBuf[di + 2] = sRGBFromFP16(rgbaBuf + ri);

            /*
            dibBuf[di] = ByteFromFP16(rgbaBuf + ri + 4);
            dibBuf[di + 1] = ByteFromFP16(rgbaBuf + ri +2);
            dibBuf[di + 2] = ByteFromFP16(rgbaBuf + ri);
            */
            /*
            dibBuf[di] = (rgbaBuf[ri + 5] & 0x7F) << 1;
            dibBuf[di + 1] = (rgbaBuf[ri + 3] & 0x7F) << 1;
            dibBuf[di + 2] = (rgbaBuf[ri + 1] & 0x7F) << 1;
            */
        }
    }

    return dibBuf;

}


BYTE* CreateDIBfromRGBA(BYTE* rgbaBuf, int w, int h, BITMAPINFOHEADER* pbmih){

	return CreateDIBfromRGBA(rgbaBuf, w, w, h, pbmih);


}


BYTE* CreateDIBfromRGB(BYTE* rgbBuf, int w, int h, BITMAPINFOHEADER* pbmih){
	
	int size_Color = 3;
	int size_Line = w * size_Color;
	if (size_Line % 4){	size_Line += 4 - (size_Line % 4);}

	pbmih->biSize = 40; 
    pbmih->biWidth = w; 
    pbmih->biHeight = h; 
    pbmih->biPlanes = 1; 
    pbmih->biBitCount = 24;
    pbmih->biCompression = 0; 
    pbmih->biSizeImage = size_Line * h; 
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0; 
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0; 


	BYTE *dibBuf = new BYTE[pbmih->biSizeImage];

	
	int i, di, ri, dk, rk;
		
	rk = h * w * 3;
	for(dk = 0; dk < h * size_Line; dk += size_Line){
		rk -= w * 3;
		for(i = 0; i < w; i ++){
			di = i * 3 + dk;
			ri = i * 3 + rk;
			dibBuf[di] = rgbBuf[ri + 2];
			dibBuf[di + 1] = rgbBuf[ri + 1];
			dibBuf[di + 2] = rgbBuf[ri];
		}
	}

	return dibBuf;

}


void writeDIB(FILE *fp, BITMAPINFOHEADER* pbmih, BYTE* bmBuf){
	int size_Line;
	BITMAPFILEHEADER bmfh;
	
	if (pbmih->biSizeImage){
		size_Line = pbmih->biSizeImage / pbmih->biHeight;
	}else{
		size_Line = pbmih->biWidth * (pbmih->biBitCount / 8);
		if (size_Line % 4){	size_Line += 4 - (size_Line % 4);}
		pbmih->biSizeImage = size_Line * pbmih->biHeight;
	}
	

	bmfh.bfType = 0x4D42;
	bmfh.bfSize = SZ_BMFH + pbmih->biSize + pbmih->biSizeImage;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = SZ_BMFH+pbmih->biSize;
		
	writeBMH(&bmfh, pbmih, fp);
	
	fwrite(bmBuf, pbmih->biSizeImage, 1, fp);
	
	return;
}
