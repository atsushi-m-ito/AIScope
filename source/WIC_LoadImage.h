#pragma once
/*
Windows Imaging Componentを利用した画像ファイル読み込み
引用元 http://tanayalog.blogspot.jp/2013_12_01_archive.html
http://tanayalog.blogspot.jp/2013/09/windows-1.html
*/

#ifndef WIC_LoadImage_H
#define WIC_LoadImage_H

#include "targetver.h"
#include <Windows.h>
#include <wincodec.h> 
#pragma comment(lib, "windowscodecs.lib")

#include <d2d1.h> 

#include "release.h"

inline
HRESULT WIC_LoadImage(LPCWSTR imageFileName, UINT *puiWidth, UINT *puiHeight, BYTE **ppcbBuffer)
{
	//変換後のカラーformat//
	const WICPixelFormatGUID targetPixelFormat = GUID_WICPixelFormat32bppRGBA;  //if for OpenGL, set GUID_WICPixelFormat32bppRGBA//
	const int color_width = 4;

	IWICImagingFactory *pFactory = NULL; // ファクトリ。デコーダを生成。
	IWICBitmapDecoder *pDecoder = NULL;  // デコーダ。画像ファイルからのフレーム取得に使用。
	IWICBitmapFrameDecode *pFrame = NULL; // フレーム。画像一枚を格納。
	IWICFormatConverter *pConverter = NULL; // コンバータ。目的のフォーマットに変換。

	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		// ファクトリの生成。
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			(LPVOID*)&pFactory
		);
	}

	if (SUCCEEDED(hr))
	{
		// デコーダの生成。ファイル名からデコーダを判別して生成。
		hr = pFactory->CreateDecoderFromFilename(
			imageFileName,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&pDecoder
		);
	}

	if (SUCCEEDED(hr))
	{
		// 一枚目のフレームを取得。
		hr = pDecoder->GetFrame(0, &pFrame);
	}

	if (SUCCEEDED(hr))
	{
		// コンバータの生成。ファイル名からデコーダを判別して生成。
		hr = pFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		// コンバータの初期化。
		hr = pConverter->Initialize(
			pFrame,
			targetPixelFormat, // フォーマット指定
			WICBitmapDitherTypeNone,  // ディザリング指定
			NULL,       // パレット指定
			0.f,       // α スレッショルド
			WICBitmapPaletteTypeMedianCut // パレットトランスレーション
		);
	}

	WICPixelFormatGUID guidPixelFormat;
	if (SUCCEEDED(hr))
	{
		// フォーマット情報を取得。
		hr = pConverter->GetPixelFormat(&guidPixelFormat);
	}

	if (SUCCEEDED(hr))
	{
		// フォーマットが正しいか検査。
		if (guidPixelFormat != targetPixelFormat)
		{
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{
		// 画像の幅、高さを取得。
		hr = pConverter->GetSize(puiWidth, puiHeight);
	}

	if (SUCCEEDED(hr))
	{
		// 画素の領域を確保。
		*ppcbBuffer = new BYTE[*puiWidth * *puiHeight * color_width];
		if (*ppcbBuffer == NULL)
		{
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{
		// 画素を取得。
		hr = pConverter->CopyPixels(NULL, (*puiWidth * color_width), (*puiWidth * *puiHeight * color_width), *ppcbBuffer);
	}

	if (FAILED(hr))
	{
		// エラーが発生した場合は、画素の領域を解放。
		if (*ppcbBuffer)
		{
			delete[] * ppcbBuffer;
		}
		*ppcbBuffer = NULL;
	}

	// ファクトリ、デコーダ、フレームの解放。



	if (pConverter)
	{
		pConverter->Release();
	}
	if (pFrame)
	{
		pFrame->Release();
	}

	if (pDecoder)
	{
		pDecoder->Release();
	}
	if (pFactory)
	{
		pFactory->Release();
	}

	return hr;
}


inline 
HRESULT LoadResourceBitmap(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	PCWSTR resourceName,
	PCWSTR resourceType,
	//UINT destinationWidth,
	//UINT destinationHeight,
	ID2D1Bitmap **ppBitmap
)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	/*入力］リソースが入った実行可能ファイルを持つモジュールのハンドルを指定します。 
	NULL を指定すると、現在のプロセスを作成するときに使われたイメージファイルに関連付けられたモジュールが使われます。*/
	const HMODULE HINST_THISCOMPONENT = NULL;

	// Locate the resource.
	imageResHandle = FindResourceW(HINST_THISCOMPONENT, resourceName, resourceType);
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageResDataHandle ? S_OK : E_FAIL;
	}


	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImageFile = LockResource(imageResDataHandle);

		hr = pImageFile ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageFileSize ? S_OK : E_FAIL;

	}

	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = pIWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(pImageFile),
			imageFileSize
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = pIWICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{

		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);

	}

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler)

	return hr;
}

#if 0
inline
HRESULT WIC_LoadResourceBitmap(
	ID2D1RenderTarget *pRenderTarget,
	PCWSTR resourceName,
	PCWSTR resourceType,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap
)
{
	IWICImagingFactory *pFactory = NULL; // ファクトリ。デコーダを生成。
	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		// ファクトリの生成。
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			(LPVOID*)&pFactory
		);
	}

	if (SUCCEEDED(hr))
	{
		LoadResourceBitmap(
			pRenderTarget,
			pFactory,
			resourceName,
			resourceType,
			destinationWidth,
			destinationHeight,
			ppBitmap
		);
	}

	if (pFactory)
	{
		pFactory->Release();
	}
}
#endif

#endif //WIC_LoadImage_H//

