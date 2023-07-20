#ifndef __glsprite_h__
#define __glsprite_h__


#include "targetver.h"
#include <windows.h>

#include <stdio.h>
//#include <stdlib.h>
#include <tchar.h>
#include <string.h>

#include <GL/gl.h>		//OpenGL利用に必要<br />
#include <GL/glu.h>		//gluPerspectiveを使うためにインクルード
/*
#pragma comment( lib , "opengl32.lib" )	//OpenGL用ライブラリをリンク
#pragma comment( lib , "glu32.lib" )	//glu関数用ライブラリをリンク
*/
#include "windib2.h"
//#include "glimage.h"


struct TEXTURE_LIST{
	GLuint texture;
	int w;
	int h;
	TCHAR* filename;
	int refer;			//参照されている(読み込み指示のあった)回数(0になったらDeleteする)
	TEXTURE_LIST* next;
};

struct SPRITE_LIST{
	int destx;
	int desty;
	int destw;
	int desth;
	float srcx1;
	float srcy1;
	float srcx2;
	float srcy2;
	GLuint texture;
	
	SPRITE_LIST* next;
};


class glsprite  
{
private:
	SPRITE_LIST* m_spr_fst;
	TEXTURE_LIST* m_txt_fst;

//textureListの操作
	TEXTURE_LIST* FindTextureList(const TCHAR* filename);
	TEXTURE_LIST* AddTextureList(GLuint texture, int w, int h, const TCHAR* filename);


//指定の色に、指定のアルファチャンネルを加えてBMPを読み込む
	GLuint tagLoadTextureAlphaBMP24(const TCHAR* fname, BYTE red, BYTE green, BYTE blue, BYTE alpha);
	
public:


	glsprite();
	virtual ~glsprite();

	GLuint LoadTextureBMP(const TCHAR* fname);
	GLuint LoadTextureAlphaBMP(const TCHAR* fname, BYTE red, BYTE green, BYTE blue);
	void ClearSprite();
	void SetSprite(int destx, int desty, int destw, int desth, int srcx, int srcy, int srcw, int srch, GLuint textureId);
	void DrawSprite();
	void DrawSpriteAlpha();
	
	//void PngReadFunc(png_struct *pPng, png_bytep buf, png_size_t size);
	//BYTE* LoadPNGBuf32(const char* fname, int* retW, int* retH);
//	GLuint LoadTexturePNG(const TCHAR* fname);

	int GetTextureSize(GLuint textureId, int* pwidth, int* pheight);

	//テクスチャの参照数を一つ増やす
	void ReferTexture(GLuint textureId);
	//テクスチャの開放
	void ReleaseTexture(GLuint textureId);
	int ReleaseTextureAll();
	
};


/*
//以下は後に外す
//文字列表示
void AI_DrawString(void *font, const char* msgstr);
int AI_WidthString(void *font, const char* msgstr);
*/

#endif    // __glsprite_h__