#include "targetver.h"
#include <windows.h>

#include <tchar.h>
#include <GL/gl.h>		//OpenGL利用に必要<br />
#include <GL/glu.h>		//gluPerspectiveを使うためにインクルード



GLuint RegisterTextureGL(const int width, const int height, const BYTE* pixels){

	GLuint texture_id;

	//画像ロード
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	//転送を4バイトごとに,windibと仕様が合うし、32bitでは高速
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	/*
	if (alpha == 255){	//alphaが255なら透過しない

		//データフォーマット赤青入れ替えと幅を2の累乗化
		newBuf = convDIBtoGLTEX(&bmih, texturebuf, 3, &resW, &resH);
		if (newBuf){
			delete[] texturebuf;
			texturebuf = newBuf;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, 3, resW, resH,
			0, GL_RGB, GL_UNSIGNED_BYTE, texturebuf);
		delete[] texturebuf;


	} else{	//alphaが255以下なら指定色(red,green,blue)に指定alphaをセット

		newBuf = convDIBtoGLTEX(&bmih, texturebuf, 4, &resW, &resH);
		if (newBuf){
			delete[] texturebuf;
			texturebuf = newBuf;
		}
		setGLTEXAlpha(newBuf, resW*resH * 4, red, green, blue, alpha);

		glTexImage2D(GL_TEXTURE_2D, 0, 4, resW, resH,
			0, GL_RGBA, GL_UNSIGNED_BYTE, newBuf);
		delete[] newBuf;

	}
	*/


	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	

	//次のパラメータを各テクスチャごとに設定しないと表示されない場合がある			
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//マテリアルカラーを無視する
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glDisable(GL_TEXTURE_2D);

	//登録後のtexture_idを返す//
	return texture_id;
}
