////////////////////////////////////////////////////
//
//	AIScope用のグラフィックライブラリ
//	SEGRAのラッパー
//
//	主な機能
//	(1)粒子の描画(ポリゴン)
//	(2)粒子の描画(ポイントスプライト)
//	(3)3D場の描画(半透明マルチテクスチャ)
//
/////////////////////////////////////////



#ifndef __ai_graphic_h__
#define __ai_graphic_h__

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#include <windows.h>


///////////////////////////////////////////////
//   OpenGL
//
#include <GL/gl.h>		//OpenGL利用に必要<br />
#include <GL/glu.h>		//gluPerspectiveを使うためにインクルード

#pragma comment( lib , "opengl32.lib" )	//OpenGL用ライブラリをリンク


///////////////////////////////////////////////
//   DirectX
//
#include <d3d11.h>
//#include <d3dx11.h>
//#include <d3dcompiler.h>

#pragma comment( lib, "d3d11.lib" )
//#if defined(DEBUG) || defined(_DEBUG)
//#pragma comment( lib, "d3dx11d.lib" )
//#else
//#pragma comment( lib, "d3dx11.lib" )
//#endif
//#pragma comment( lib, "dxerr.lib" )
//#pragma comment( lib, "dxgi.lib" )

//d3dx11_43.dllを必ず遅延読み込みにすること.
//dll名の末尾の数字はSDKとランタイムのバージョンに依存する.


/////////////////////////////////////////////////////////////////

int aigCheckLibrary();		//使用可能なライブラリを返す(DirectX11, OpenGL3.3, OpenGL1.5).

int aigInitialize(int graphictype, void* params);
int aigTerminate(void* params);






///////////////////////////////////////
//以下は selective graphic engine (SGE)
///////////////////////////////////////


#define SGE_TYPE_DX11		1
#define SGE_TYPE_GL15		1015
#define SGE_TYPE_GL33		1033

int sgeInitialize(int graphictype, void* params);

int sgeClearScreenBuffer(float* color4);


#endif	//__ai_graphic_h__


