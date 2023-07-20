#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincodec.h> 
#include <d2d1_1.h>
#include <map>
#include "WIC_LoadImage.h"
#include "resource.h"

class PanelD2D {
private:
	IWICImagingFactory *m_pFactory = nullptr; // ファクトリ。デコーダを生成。
	ID2D1RenderTarget *m_pRenderTarget;

	std::map<int, ID2D1Bitmap*> image_list;

	
	struct Object {		
		int state;
		int id;
		enum class Type {
			Image, Button,
		} type;

		int image_id;
		enum class RelativeX {
			Left, Right, Center,
		} x_relative;
		enum class RelativeY {
			Top, Bottom, Center,
		} y_relative;
		float x;
		float y;
		float w;
		float h;

	public:
		D2D_RECT_F Rect(float screen_w = 0.0f, float screen_h = 0.0f) {

			D2D_RECT_F rect;
			switch (x_relative) {
			case RelativeX::Right:
				rect.left = screen_w + x - w;
				rect.right = screen_w + x;
				break;
			case RelativeX::Center:
				rect.left = (screen_w - w) / 2.0f + x;
				rect.right = (screen_w + w) / 2.0f + x;
				break;
			default:
				rect.left = x;
				rect.right = x + w;
				break;
			}

			switch (y_relative) {
			case RelativeY::Bottom:
				rect.top = screen_h + y - h;
				rect.bottom = screen_h + y;
				break;
			case RelativeY::Center:
				rect.top = (screen_h - h) / 2.0f + y;
				rect.bottom = (screen_h + h) / 2.0f + y;
				break;
			default:
				rect.top = y;
				rect.bottom = y + h;
				break;
			}
			return rect;
		}
	};

	std::vector<Object> object_list;

	
public:
	PanelD2D(ID2D1RenderTarget *pRenderTarget):
		m_pRenderTarget(pRenderTarget)
	{
		// ファクトリの生成。
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			(LPVOID*)&m_pFactory
		);
		if (SUCCEEDED(hr)) {
		}

		mSetting();

	}

	~PanelD2D() {

		for (auto&& pbmp : image_list) {
			SAFE_RELEASE(pbmp.second);
		}

		SAFE_RELEASE(m_pFactory);
	}

	HRESULT AddImage(PCWSTR resourceName, PCWSTR resourceType, int id) {
		if (m_pRenderTarget == nullptr) {
			return -1;
		}
		if(m_pFactory == nullptr) {
			return -1;
		}
		
		ID2D1Bitmap *pBitmap;
		HRESULT hr = LoadResourceBitmap(
			m_pRenderTarget,
			m_pFactory,
			resourceName,
			resourceType,
			//destinationWidth,
			//destinationHeight,
			&pBitmap
		);
		if (SUCCEEDED(hr)){
			image_list.insert(std::make_pair(id, pBitmap));
		}
		return hr;
	}

	D2D_RECT_F RectFromXYWH(float x, float y, float w, float h) {
		return D2D_RECT_F{ x, y, x + w, y + h };
	}

	D2D_RECT_F RectFromCenterXYWH(float x, float y, float w, float h) {
		return D2D_RECT_F{ x - w/2.0f, y - h/2.0f, x + w/2.0f, y + h/2.0f };
	}

public:
	void Draw() {
		const D2D1_SIZE_F size = m_pRenderTarget->GetSize();
		/*
		float dpi_x, dpi_y;
		m_pRenderTarget->GetDpi(&dpi_x, &dpi_y);
		*/



		/*
		constexpr D2D1_COLOR_F color_orange{ 0.92f,  0.38f,  0.208f,  1.0f };
		ID2D1SolidColorBrush* m_brush;

		
		auto res = m_pRenderTarget->CreateSolidColorBrush(
			color_orange,
			D2D1::BrushProperties(0.8f),
			&m_brush);

		
		m_pRenderTarget->DrawRectangle(D2D_RECT_F{ 10.0f, 10.0f, 100.0f, 100.0f }, m_brush, 10.0f);
		m_brush->Release();
		
		*/

		const int current_state = 1;
		for (auto&& a : object_list) {
			if (a.state != current_state) continue;

			m_pRenderTarget->DrawBitmap(image_list[a.image_id],
				a.Rect(size.width, size.height), 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

			
		}

	}

	int Push(float x, float y) {
		const D2D1_SIZE_F size = m_pRenderTarget->GetSize();
		
		const int current_state = 1;
		for (auto&& a : object_list) {
			if (a.state != current_state) continue;

			//if(a.type == Object::Type::Button){
			auto rect = a.Rect(size.width, size.height);
			if (rect.left <= x) {
				if (x < rect.right) {
					if (rect.top <= y) {
						if (y < rect.bottom) {
							return a.id;
						}
					}
				}
			}

		}

		return 0;
	}


private:
	/*
	setting depending on application
	*/
	void mSetting() {

		//button image load 
		HRESULT hr = AddImage(MAKEINTRESOURCE(IDB_PNG_NEXT_MOV), L"PNG", 2);
		hr = AddImage(MAKEINTRESOURCE(IDB_PNG_NEXT_1), L"PNG", 1);
		hr = AddImage(MAKEINTRESOURCE(IDB_PNG_PREV_MOV), L"PNG", 4);
		hr = AddImage(MAKEINTRESOURCE(IDB_PNG_PREV_1), L"PNG", 3);
		hr = AddImage(MAKEINTRESOURCE(IDB_PNG_STOP), L"PNG", 5);

		//locate button
		const float width_button = 64.0;
		const float height_button = 64.0;

		object_list.push_back(Object{ 1, 5, Object::Type::Button, 5,
			Object::RelativeX::Center, Object::RelativeY::Bottom,
			0.0, -height_button / 2.0f, width_button, height_button });


		object_list.push_back(Object{ 1, 2, Object::Type::Button, 2,
			Object::RelativeX::Center, Object::RelativeY::Bottom,
			width_button * 1.5f, -height_button / 2.0f, width_button, height_button });


		object_list.push_back(Object{ 1, 1, Object::Type::Button, 1,
			Object::RelativeX::Center, Object::RelativeY::Bottom,
			width_button * 3.0f, -height_button / 2.0f, width_button, height_button });


		object_list.push_back(Object{ 1, 4, Object::Type::Button, 4,
			Object::RelativeX::Center, Object::RelativeY::Bottom,
			-width_button * 1.5f, -height_button / 2.0f, width_button, height_button });

		object_list.push_back(Object{ 1, 3, Object::Type::Button, 3,
			Object::RelativeX::Center, Object::RelativeY::Bottom,
			-width_button * 3.0f, -height_button / 2.0f, width_button, height_button });




	}



};

