
#ifndef __CommonShader_h__
#define __CommonShader_h__

#pragma once

#include "listTemplate.h"


#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <math.h>

#include <d3d11.h>
//#include <d3dx11.h>
//#include <d3dcompiler.h>


struct COM_SHADER_LIST {
	COM_SHADER_LIST* prev;
	COM_SHADER_LIST* next;
	int refCount;		//テクスチャの参照回数(ロード回数).0になったら削除.
	
	WCHAR* szFileName;
	LPCSTR szEntryPoint;
	LPCSTR szShaderModel;

#define	TYPE_VERTEXSHADER	1
#define	TYPE_PIXELSHADER	2
#define	TYPE_GEOMETRYSHADER	3

	int type;
	union{
		ID3D11VertexShader*     pVertexShader;
		ID3D11PixelShader*      pPixelShader;
		ID3D11GeometryShader*	pGeometryShader;
	};
	ID3D11InputLayout*      pInputLayout;

	//コンストラクタ
	COM_SHADER_LIST(){
		memset(this, 0, sizeof(COM_SHADER_LIST));
	};

	//デストラクタ
	~COM_SHADER_LIST(){
		delete [] szFileName;
		delete [] szEntryPoint;
		delete [] szShaderModel;
		switch(type){
		case TYPE_VERTEXSHADER:
			if(pVertexShader){ 
				pVertexShader->Release();}
			if(pInputLayout){
				pInputLayout->Release();}
			break;
		case TYPE_PIXELSHADER:
			if(pPixelShader){ pPixelShader->Release();}
			break;
		case TYPE_GEOMETRYSHADER:
			if(pGeometryShader){ pGeometryShader->Release();}
			break;
		}
		
	};
};

class CommonShader : public MasterList<COM_SHADER_LIST>{
private:
	

	ID3D11Device*			m_pD3DDevice;

	COM_SHADER_LIST* FindList(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel);
	COM_SHADER_LIST* FindList(LPVOID pShader);

	//HRESULT CompileShaderFromFile(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	

public:
	
	CommonShader(ID3D11Device* pD3DDevice);
	virtual ~CommonShader();

	ID3D11VertexShader*     LoadVertexShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, const D3D11_INPUT_ELEMENT_DESC* inputElements, UINT numElements, ID3D11InputLayout** pInputLayout);
	ID3D11GeometryShader*	LoadGeometryShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel);
	ID3D11PixelShader*		LoadPixelShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel);
			
	void UnloadShader(LPVOID pShader);

};



// Define the input layout
const D3D11_INPUT_ELEMENT_DESC DX11LAYOUT_P3N3[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

};

const D3D11_INPUT_ELEMENT_DESC DX11LAYOUT_P3N3T2[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

};


#endif	//__CommonShader_h__