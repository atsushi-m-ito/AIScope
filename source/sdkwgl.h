//=====================================
// sdkwgl クラス
// 
//
//=====================================
#ifndef __sdkwgl_h__
#define __sdkwgl_h__

#pragma once

#include <stdio.h>
#include "targetver.h"
#include <windows.h>


#include <GL/gl.h>		//OpenGL利用に必要<br />
#include <GL/glu.h>		//gluPerspectiveを使うためにインクルード

//#define GL_GLEXT_PROTOTYPES 1
#include "wglext.h"		//WGL extensions
#include "glext.h"


#pragma comment( lib , "opengl32.lib" )	//OpenGL用ライブラリをリンク
#pragma comment( lib , "glu32.lib" )	//glu関数用ライブラリをリンク


#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

//HGLRC InitSDKWGL(HWND hWnd);
//void TerminSDKWGL(HWND hWnd, HGLRC hRC);
//HGLRC InitSDKWGL(HDC hOwnDC);
//HDC InitSDKWGL(HWND hWnd);
//HGLRC InitSDKWGL(HWND hWnd, HDC* phOwnDC);
//void TerminSDKWGL(HWND hWnd, HGLRC hRC, HDC hOwnDC);
HDC GetWGLDC(HWND hWnd);
HDC GetWGLDCbyPixelFormat(HWND hWnd, int pixelFormat);
HGLRC CreateRC(HDC hDC);
void DeleteRC(HGLRC hRC);
//void TerminSDKWGL(HDC hOwnDC, HGLRC hRC);
//void TerminSDKWGL(HWND hWnd, HDC hDC);
void Set2D(int scrW, int scrH, int scaling);
void SetScreen2D(int screen_x, int screen_y, int screen_w, int screen_h, float camera_w, float camera_h);

BOOL SetSwapInterval(int interval);
int GetSwapInterval(void);
//void InitWait(HDC hDC, int refleshRate);
int InitWaitVsync();
int InitWait(int refleshRate);
double WaitSleep(int *r_sleepTm);
void ResetWait();


////VBOの準備----拡張関数

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLISBUFFERPROC glIsBuffer;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
extern PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

void LoadGLEXT();
int GetPixelFormatByDummy(HINSTANCE hInstance);


///////////////////////////////////////////////////////////////////////////////
//
//レンダリングコンテキスト(RC)とShareLists関連の管理
//
//仕様:
//OpenGLではウィンドウごとにRCを作る。
//テクスチャ共有するウィンドウは、必ず一度hWndを本ライブラリに登録する。
//(一つのRCの使いまわしはしない。vista用intelドライバで上手く動かないため)
//最初に登録したhWndのRCをマスターとして、
//ShareListsで二番目以降のRCを関連付ける
//全てのテクスチャはマスターのRC上に生成する
//
///////////////////////////////////////////////////////////////////////////////
void RegisterSharedRC(HWND hWnd, HGLRC hRC);
void DeleteSharedRC();
HGLRC GetSharedRC(HWND hWnd);


#endif    // __sdkwgl_h__

