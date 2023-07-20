#include "CommonShader.h"
#include "bin_reader.h"

CommonShader::CommonShader(ID3D11Device* pD3DDevice) : 
	m_pD3DDevice(pD3DDevice)
{
}


CommonShader::~CommonShader()
{
}

COM_SHADER_LIST* CommonShader::FindList(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel){

	for(COM_SHADER_LIST* p = this->first; p ; p = p->next){
		if(_tcscmp(p->szFileName, szFileName)==0){
			if(strcmp(p->szEntryPoint, szEntryPoint)==0){
				if(strcmp(p->szShaderModel, szShaderModel)==0){
					return p;
				}
			}
		}
	}

	return NULL;
}

COM_SHADER_LIST* CommonShader::FindList(LPVOID pShader){

	for(COM_SHADER_LIST* p = this->first; p ; p = p->next){
		if((LPVOID)(p->pVertexShader) == pShader){
			return p;
		}
	}

	return NULL;
}




ID3D11VertexShader* CommonShader::LoadVertexShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, 
								const D3D11_INPUT_ELEMENT_DESC* inputElements, UINT numElements, ID3D11InputLayout** ppInputLayout){

	COM_SHADER_LIST* p = FindList(szFileName,szEntryPoint, szShaderModel);
	if(p){
		p->refCount++;
		*ppInputLayout = p->pInputLayout;
		return p->pVertexShader;
	}
/*
	// Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    HRESULT hr = CompileShaderFromFile( szFileName, szEntryPoint, szShaderModel, &pVSBlob );
    if( FAILED( hr ) ) {
        return NULL;
    }
*/
	unsigned char* cso_data;
	int cso_sz = ReadBinFile(szFileName, &cso_data);
	if( cso_sz < 0 ) {
        return NULL;
    }

	// Create the vertex shader
	ID3D11VertexShader* pVertexShader;
	HRESULT hr = m_pD3DDevice->CreateVertexShader( cso_data, cso_sz, NULL, &pVertexShader );
	//hr = m_pD3DDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVertexShader );
	if( FAILED( hr ) ) {	
	//	pVSBlob->Release();
		delete [] cso_data;
        return NULL;
	}


    // Create the input layout
	ID3D11InputLayout* pInputLayout;
	hr = m_pD3DDevice->CreateInputLayout( inputElements, numElements, cso_data,
                                          cso_sz, &pInputLayout );
	//hr = m_pD3DDevice->CreateInputLayout( inputElements, numElements, pVSBlob->GetBufferPointer(),
    //                                      pVSBlob->GetBufferSize(), &pInputLayout );
	delete [] cso_data;
	//pVSBlob->Release();
	if( FAILED( hr ) ){
		pVertexShader->Release();
	    return NULL;
	}

	p = AddFirst();
	p->pVertexShader = pVertexShader;
	p->pInputLayout = pInputLayout;
	p->type = TYPE_VERTEXSHADER;
	p->refCount = 1;
	p->szFileName = _tcsdup(szFileName);
	p->szEntryPoint = _strdup(szEntryPoint);
	p->szShaderModel = _strdup(szShaderModel);
	
	
	*ppInputLayout = pInputLayout;
    return pVertexShader;
}

ID3D11GeometryShader* CommonShader::LoadGeometryShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel){
	
	COM_SHADER_LIST* p = FindList(szFileName,szEntryPoint, szShaderModel);
	if(p){
		p->refCount++;
		return p->pGeometryShader;
	}

/*	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
    HRESULT hr = CompileShaderFromFile( szFileName, szEntryPoint, szShaderModel, &pPSBlob );
	if( FAILED( hr ) ){
		return NULL;
    }
	*/
	unsigned char* cso_data;
	int cso_sz = ReadBinFile(szFileName, &cso_data);
	if( cso_sz < 0 ) {
        return NULL;
    }


	// Create the Geometry shader
	ID3D11GeometryShader* pGeometryShader;
	HRESULT hr = m_pD3DDevice->CreateGeometryShader( cso_data, cso_sz, NULL, &pGeometryShader );
//	hr = m_pD3DDevice->CreateGeometryShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pGeometryShader );
//	pPSBlob->Release();
	delete [] cso_data;
	if( FAILED( hr ) ){
        return NULL;
	}

	p = AddFirst();
	p->pGeometryShader = pGeometryShader;
	p->type = TYPE_GEOMETRYSHADER;
	p->refCount = 1;
	p->szFileName = _tcsdup(szFileName);
	p->szEntryPoint = _strdup(szEntryPoint);
	p->szShaderModel = _strdup(szShaderModel);


    return pGeometryShader;
}

ID3D11PixelShader* CommonShader::LoadPixelShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel){
	
	COM_SHADER_LIST* p = FindList(szFileName,szEntryPoint, szShaderModel);
	if(p){
		p->refCount++;
		return p->pPixelShader;
	}

	/*
	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
    HRESULT hr = CompileShaderFromFile( szFileName, szEntryPoint, szShaderModel, &pPSBlob );
	if( FAILED( hr ) ){
		return NULL;
    }
	*/

	unsigned char* cso_data;
	int cso_sz = ReadBinFile(szFileName, &cso_data);
	if( cso_sz < 0 ) {
        return NULL;
    }


	// Create the pixel shader
	ID3D11PixelShader* pPixelShader;
	HRESULT hr = m_pD3DDevice->CreatePixelShader( cso_data, cso_sz, NULL, &pPixelShader );
//	hr = m_pD3DDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pPixelShader );
//	pPSBlob->Release();
	delete [] cso_data;
	if( FAILED( hr ) ){
        return NULL;
	}

	p = AddFirst();
	p->pPixelShader = pPixelShader;
	p->type = TYPE_PIXELSHADER;
	p->refCount = 1;
	p->szFileName = _tcsdup(szFileName);
	p->szEntryPoint = _strdup(szEntryPoint);
	p->szShaderModel = _strdup(szShaderModel);


    return pPixelShader;
}

void CommonShader::UnloadShader(LPVOID pShader){


	COM_SHADER_LIST* p = FindList(pShader);
	if(p){
		p->refCount--;
		if(p->refCount == 0){
			Delete(p);
		}
	}

}