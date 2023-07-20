#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class IGraphicDevice
{
public:
	virtual ~IGraphicDevice(){};

	virtual int InitializeWindow(int width, int height, int init_windowmode, int vsync_mode) = 0;
	virtual void BeginRendering() = 0;
	virtual void EndRendering() = 0;
	virtual void Resize(int width, int height) = 0;

};
