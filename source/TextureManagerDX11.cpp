//Visual Studio2012 + DirectXSDK(inWindouwsSDK8)からD3DXがなくなたっためテクスチャは一時使用停止//

#include "TextureManagerDX11.h"
#include "WIC_LoadImage.h"



TextureManager::~TextureManager(){
//全てのテクスチャを解放//

    for (IteratorKeyValue it = m_texture_list.begin(); it != m_texture_list.end(); it++){
        it->second.pTextureRV->Release();
        delete[] it->second.pTextureRV;
    }
}


ID3D11ShaderResourceView* TextureManager::LoadTexture(const WCHAR* filepath, int* pwidth, int* pheight){
	
	

	//既に読み込まれていないかチェック//
	//読み込み済みならResourceViewポインタを返す//

    std::wstring key(filepath);
    IteratorKeyValue it = m_texture_list.find(key);
    if (it != m_texture_list.end()) {
        // キーの要素がある場合の処理
        it->second.refer_count++;
        if (pwidth)*pwidth = it->second.width;
        if (pheight)*pheight = it->second.height;
        return it->second.pTextureRV;
    }


    //まだ読み込まれていないファイルの指定の場合は新規にLoadする//
    
    //画像ファイルを読み込んでRGBA形式のデータを取得//
    UINT width;
    UINT height;
    BYTE* rgba;
    WIC_LoadImage(filepath, &width, &height, &rgba);
	

	//DirectXにリソースとして登録//
    D3D11_TEXTURE2D_DESC Texture2DDesc;
    Texture2DDesc.ArraySize = 1;
    Texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;   //textureの場合//
    Texture2DDesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
    Texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Texture2DDesc.Height = height;
    Texture2DDesc.Width = width;
    Texture2DDesc.MipLevels = 1;
    Texture2DDesc.MiscFlags = 0;
    Texture2DDesc.SampleDesc.Count = 1;
    Texture2DDesc.SampleDesc.Quality = 0;
    Texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
    
    D3D11_SUBRESOURCE_DATA resource;
    resource.pSysMem = rgba;
    resource.SysMemPitch = width * 4;
    resource.SysMemSlicePitch = 0;  //3Dテクスチャではざっくりwidth*height*4の意味//

    ID3D11Texture2D *hTexture;
    HRESULT hr = m_pD3DDevice->CreateTexture2D(&Texture2DDesc, &resource, &hTexture);
    if (FAILED(hr)){
        return NULL;
    }


	ID3D11ShaderResourceView* pTextureRV;
    hr = m_pD3DDevice->CreateShaderResourceView(hTexture, NULL, &pTextureRV);
    if( FAILED( hr ) ){
		return NULL;
	}

	

    TEXTURE_INFO texture;
    texture.refer_count = 1;
    texture.width = width;
    texture.height = height;
    texture.buffer = rgba;
    texture.pTextureRV = pTextureRV;
    m_texture_list.insert(std::pair<std::wstring, TEXTURE_INFO>(key, texture));

    if (pwidth)  *pwidth = width;
    if (pheight) *pheight = height;

	return pTextureRV;
}

void TextureManager::UnloadTexture(const WCHAR* filepath){


    std::wstring key(filepath);
    IteratorKeyValue it = m_texture_list.find(key);
    if (it != m_texture_list.end()) {
        // キーの要素がある場合の処理
        it->second.refer_count--;
        if (it->second.refer_count == 0){
            it->second.pTextureRV->Release();
            m_texture_list.erase(it);
            
        }
    }

}

void TextureManager::UnloadTexture(ID3D11ShaderResourceView* pTextureRV){


    for (IteratorKeyValue it = m_texture_list.begin(); it != m_texture_list.end(); it++){
        if (it->second.pTextureRV == pTextureRV){
            // キーの要素がある場合の処理
            it->second.refer_count--;
            if (it->second.refer_count == 0){
                it->second.pTextureRV->Release();
                m_texture_list.erase(it);
            }
            return;
        }
        
    }

}

