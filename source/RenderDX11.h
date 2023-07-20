#ifndef RenderDX11_H
#define RenderDX11_H

#pragma once

#include <stdio.h>
#include <string.h>


#include "sdkdx11.h"

#include "filelist.h"
//#include "camera.h"
#include "RenderingProperty.h"

void InitDX11(DX11_CORE& dx11_core);
void ResetDX11(DX11_CORE& dx11_core);
void ClearRenderingTargetDX11(const float* bg_color);
void RenderingDX11(std::vector<LawData>& data_list, int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property);
void RenderingCoreDX11(std::vector<LawData>& data_list, int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property);
void TerminateDX11();
void CheckRtvDX11();

//GLで描画した内容をビットマップ画像として保存//
void OutputDX11(std::vector<LawData>& data_list, const TCHAR *fName, int width, int height, int magnify, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property);

#endif    // RenderDX11_H