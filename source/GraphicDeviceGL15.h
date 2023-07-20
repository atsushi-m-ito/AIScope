#pragma once

#define WIN32_LEAN_AND_MEAN
#include "IGraphicDevice.h"
#include <Windows.h>
#include "sdkwgl.h"
#include "RenderGL15.h"

class GraphicDeviceGL15 : public IGraphicDevice
{
private:
	HWND m_hWnd;
	HDC m_hDC;	//レンダリング中のwin32gdi device context

public:
	GraphicDeviceGL15(HWND hWnd):
		m_hWnd(hWnd)
	{

	};


	~GraphicDeviceGL15()
	{
		//delete g_spriteGL15;
		DeleteSharedRC();

	};

	int InitializeWindow(int width, int height, int init_windowmode, int vsync_mode)
	{
		int result = 0;
		HWND hWnd = m_hWnd;

		//OpenGLの初期化/////////////////////////////////////////////

		HDC hDC = GetWGLDC(hWnd);
		HGLRC hRC = CreateRC(hDC);
		RegisterSharedRC(hWnd, hRC);
		wglMakeCurrent(hDC, hRC);

		LoadGLEXT();

		InitGL15();

		//垂直同期の設定////////////////////

		switch (vsync_mode){
		case 0:
			SetSwapInterval(0);	//第一引数が0なら垂直同期しない, 1なら垂直同期.
			break;
		case 1:
			int res = SetSwapInterval(1);	//第一引数が0なら垂直同期しない, 1なら垂直同期.
			if (res == 0){
				vsync_mode = 3;
				result = 3;
			}
			break;
		}


		wglMakeCurrent(NULL, NULL);
		ReleaseDC(hWnd, hDC);

		return result;
	};

	//virtual void Terminate() = 0;
	void BeginRendering()
	{
		HWND hWnd = m_hWnd;
		m_hDC = GetDC(hWnd);
		HGLRC hRC = GetSharedRC(hWnd);
		wglMakeCurrent(m_hDC, hRC);

		InitGL15();
	};

	void EndRendering()
	{
		SwapBuffers(m_hDC);
		
		wglMakeCurrent(NULL, NULL);
		ReleaseDC(m_hWnd, m_hDC);
	};

	void Resize(int width, int height)
	{
		//何もしない//
	};
};
