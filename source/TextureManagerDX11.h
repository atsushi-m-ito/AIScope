//Visual Studio2012 + DirectXSDK(inWindouwsSDK8)からD3DXがなくなたっためテクスチャは一時使用停止//
#pragma once

#ifndef TextureManagerDX11_H
#define TextureManagerDX11_H




#include <stdio.h>
#include <tchar.h>
#include <map>
#include <string>


#include <d3d11.h>


class TextureManager{
private:
	
    struct TEXTURE_INFO {
        //TCHAR* filepath;
        int refer_count;		//テクスチャの参照回数(ロード回数).0になったら削除.
        UINT width;
        UINT height;
        BYTE* buffer;
        ID3D11ShaderResourceView* pTextureRV;
    };

    std::map< std::wstring , TEXTURE_INFO> m_texture_list;
    typedef std::map<std::wstring, TextureManager::TEXTURE_INFO>::iterator IteratorKeyValue;


	ID3D11Device*			m_pD3DDevice;

public:
	
    TextureManager(ID3D11Device* pD3DDevice) :
        m_pD3DDevice(pD3DDevice)
    { };
    
    virtual ~TextureManager();

	ID3D11ShaderResourceView* LoadTexture(const WCHAR* filepath, int* pwidth, int* pheight);
    void UnloadTexture(const WCHAR* filepath);
    void UnloadTexture(ID3D11ShaderResourceView* pTextureRV);

};



#endif	//TextureManagerDX11_H

