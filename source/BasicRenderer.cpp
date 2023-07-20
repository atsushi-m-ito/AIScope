
#define _CRT_SECURE_NO_DEPRECATE

#include "BasicRenderer.h"
static const float PI = 3.141529f;


BasicRenderer::BasicRenderer(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader, UINT vertexBufSz, UINT indexBufSz ) :
		m_pD3DDevice(pD3DDevice), m_pD3DDeviceContext(pD3DDeviceContext),
		m_comShader(comShader),
	//	m_comTexture(comTexture),
	//DX系オブジェクト変数のNULLセット.
		m_pVertexShader(NULL),
		m_pPixelShader(NULL),
		m_pVertexLayout(NULL),
		//m_pVertexBuffer(NULL),
		m_vertexBuffer(pD3DDevice, pD3DDeviceContext, 0),
		m_pIndexBuffer(NULL),
		m_pWVPBuffer(NULL),
	//m_vertexBufSz(vertexBufSz),
		m_indexBufSz(indexBufSz),
		m_pInstanceBuffer(NULL),
		m_instanceBufSz(0),
		m_instance_stride(0)
{

	m_ViewMatrix.SetIdentity();
	m_ProjectionMatrix.SetIdentity();

//材質の色設定===================================
	m_ambient_color[0] = 1.0f;
	m_ambient_color[1] = 1.0f;
	m_ambient_color[2] = 1.0f;
	m_ambient_color[3] = 1.0f;
//

}


BasicRenderer::~BasicRenderer(){

	
	
    //SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer );
    SAFE_RELEASE(m_pWVPBuffer );
    
	//if( m_pVertexLayout ) m_pVertexLayout->Release();
    //if( m_pVertexShader ) m_pVertexShader->Release();
    //if( m_pPixelShader ) m_pPixelShader->Release();
	if( m_pVertexShader ) m_comShader->UnloadShader((LPVOID)m_pVertexShader);
	if( m_pPixelShader ) m_comShader->UnloadShader((LPVOID)m_pPixelShader);

}

void BasicRenderer::DrawIndexed(UINT index_count, D3D_PRIMITIVE_TOPOLOGY primitive_topology) {
	Draw(index_count, primitive_topology, 0);
}


void BasicRenderer::Draw(UINT indexCountPerInstance, D3D_PRIMITIVE_TOPOLOGY primitive_topology, UINT instanceCount) {
	

	// Set the input layout
	m_pD3DDeviceContext->IASetInputLayout(m_pVertexLayout);
	// Set primitive topology
	m_pD3DDeviceContext->IASetPrimitiveTopology(primitive_topology);	
	
	UINT offset = 0;
	/*
	//UINT stride = sizeof(VERTEX_F3F3UI1);
	m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertex_stride, &offset);
	*/
	m_vertexBuffer.Attach();
	m_pD3DDeviceContext->IASetIndexBuffer(m_pIndexBuffer, m_index_format, 0);


	//動的な頂点バッファ(複数インスタンス用)の更新 (UpdateSubresourceを使う場合).
	//目的のバッファはUsage = D3D11_USAGE_DEFAULT, CPUAccessFlags = 0.
	//第三引数がNULLの時はVertexBuffer(VRAM)と同じサイズをコピーするので,
	//コピー元(CPU)はVertexBuffer以上のサイズが確保されていなければならない.
	//このとき第五・第六引数は無視される.
	//	m_pD3DDeviceContext->UpdateSubresource( m_pVertexBuffer, 0, NULL, positions, 0, 0);//sizeof(vec3f) * particle_count
	//結果: Map/UnmapとUpdateSubresourceでは殆ど速度差はない.


	// Render materials
	m_pD3DDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pD3DDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers);

	m_pD3DDeviceContext->GSSetShader(NULL, NULL, 0);
	//	m_pD3DDeviceContext->GSSetShader( m_pGeometryShader, NULL, 0 );
	//	m_pD3DDeviceContext->GSSetConstantBuffers(0, 1, m_pConstantBuffers);

	m_pD3DDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
	//	m_pD3DDeviceContext->PSSetShaderResources( 0, 1, &(m_pTextureRV) );
	//	m_pD3DDeviceContext->PSSetSamplers( 0, 1, &m_pSampler );
	if (instanceCount > 0) {
		// インスタンスvertex buffer
		
		m_pD3DDeviceContext->IASetVertexBuffers(1, 1, &m_pInstanceBuffer, &m_instance_stride, &offset);

		m_pD3DDeviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, 0, 0, 0);
	} else {
		m_pD3DDeviceContext->DrawIndexed(indexCountPerInstance, 0, 0);
	}
	m_pD3DDeviceContext->Flush();
	
}

#if 1
void BasicRenderer::UpdateVertexBuffer(LPVOID pBuffer, UINT count, UINT stride) {
	m_vertexBuffer.Update(pBuffer, count, stride);
}
#else
void BasicRenderer::UpdateVertexBuffer(LPVOID pBuffer, UINT count, UINT stride){
	
	if(count * stride > m_vertexBufSz){
		//バッファの作成・サイズ変更

		//すでにバッファがある場合は記憶しておき,
		//新しいバッファをGPUにセットしてから古いほうをRelease	
		ID3D11Buffer* m_pOldBuffer = NULL;
		if(	m_pVertexBuffer ){
			m_pOldBuffer = m_pVertexBuffer;
            m_pOldBuffer->Release();
		}

		m_vertexBufSz = count * stride;
		
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = stride * count;
		bd.Usage = D3D11_USAGE_DEFAULT;		//D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;				//D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.StructureByteStride=0;

		HRESULT hr = m_pD3DDevice->CreateBuffer( &bd, NULL, &m_pVertexBuffer );
        //if (FAILED(hr)){
        if (hr != S_OK){
            m_pVertexBuffer = NULL;
            return;
        }

		//if(m_pOldBuffer) m_pOldBuffer->Release();

	}///バッファの作成終わり

	if(pBuffer == NULL){//NULLの場合はバッファ作成だけして終了.
		return;
	}

	UINT offset = 0;
    m_pD3DDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	m_vertex_stride = stride;

	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = count * stride;
	destRegion.top = 0;
	destRegion.bottom = 1;
	destRegion.front = 0;
	destRegion.back = 1;
	m_pD3DDeviceContext->UpdateSubresource( m_pVertexBuffer, 0, &destRegion, pBuffer, 0, 0);
	//m_pD3DDeviceContext->UpdateSubresource( m_pVertexBuffer, 0, NULL, pBuffer, 0, 0);
}
#endif

void BasicRenderer::UpdateIndexBuffer(LPVOID pBuffer, UINT count, UINT stride){
	UINT offset = 0;
	if(stride == 2){
		m_index_format = DXGI_FORMAT_R16_UINT;
	}else{
		m_index_format = DXGI_FORMAT_R32_UINT;
	}

	
	if(count * stride > m_indexBufSz){
		//バッファの作成・サイズ変更

		//すでにバッファがある場合は記憶しておき,
		//新しいバッファをGPUにセットしてから古いほうをRelease	
		ID3D11Buffer* m_pOldBuffer = NULL;
		if(m_pIndexBuffer){
			m_pOldBuffer = m_pIndexBuffer;
		}

		m_indexBufSz = count * stride;
		
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = stride * count;
		bd.Usage = D3D11_USAGE_DEFAULT;		//D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;				//D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.StructureByteStride=0;

		HRESULT hr = m_pD3DDevice->CreateBuffer( &bd, NULL, &m_pIndexBuffer );
		if( FAILED( hr ) )
			return;
	
		if(m_pOldBuffer) m_pOldBuffer->Release();

	}///バッファの作成終わり

	if(pBuffer == NULL){//NULLの場合はバッファ作成だけして終了.
		return;
	}

	m_pD3DDeviceContext->IASetIndexBuffer( m_pIndexBuffer, m_index_format, 0 );
	//m_pD3DDeviceContext->UpdateSubresource( m_pIndexBuffer, 0, NULL, pBuffer, count * stride, count * stride);
	
	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = count * stride;
	destRegion.top = 0;
	destRegion.bottom = 1;
	destRegion.front = 0;
	destRegion.back = 1;
	m_pD3DDeviceContext->UpdateSubresource( m_pIndexBuffer, 0, &destRegion, pBuffer, 0, 0);
	//m_pD3DDeviceContext->UpdateSubresource( m_pIndexBuffer, 0, NULL, pBuffer, count * stride, count * stride);

}

#if 1
void BasicRenderer::SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix){
	struct MatrixWVP {
		mat44f world;
		mat44f view;
		mat44f projection;
	} wvp;
	wvp.world.Clear();
	wvp.world.SetIdentity();

	for (int i = 0; i < 16; ++i) {
		wvp.view.elements[i] = (float)(pViewMatrix[i]);
	}
	for (int i = 0; i < 16; ++i) {
		wvp.projection.elements[i] = (float)(pProjectionMatrix[i]);
	}

#if 1
	/* note
	D3D11_USAGE_DEFAULT, D3D11_CPU_ACCESS_WRITEなし、UpdateSubresourceの組み合わせだと
	VNTRandererで表示されないバグが起こる。
	コンスタントバッファの寿命に関連した問題か
	*/
	{
		if (!m_pWVPBuffer) {
			D3D11_BUFFER_DESC bd;
			bd.ByteWidth = sizeof(mat44f) * 3;	//16の倍数である必要がある//
			bd.Usage = D3D11_USAGE_DYNAMIC; 
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			//SUBRESOURCEによる初期値は設定しない

			HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pWVPBuffer);
			if (FAILED(hr))
				return;
		}
		

		//定数バッファの書き込み.
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_pD3DDeviceContext->Map(m_pWVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CopyMemory(mappedResource.pData, &(wvp.world), sizeof(mat44f));
		CopyMemory((mat44f*)mappedResource.pData + 1, &(wvp.view), sizeof(mat44f));
		CopyMemory((mat44f*)mappedResource.pData + 2, &(wvp.projection), sizeof(mat44f));
		//CopyMemory((mat44f*)mappedResource.pData + 3, &m_pPointSize, sizeof(vec2f));
		m_pD3DDeviceContext->Unmap(m_pWVPBuffer, 0);
		
	}
#else

	{
		if (!m_pWVPBuffer) {
			D3D11_BUFFER_DESC bd;
			bd.ByteWidth = sizeof(mat44f) * 3;	//16の倍数である必要がある//
			bd.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			//SUBRESOURCEによる初期値は設定しない

			HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pWVPBuffer);
			if (FAILED(hr))
				return;
		}

		m_pD3DDeviceContext->UpdateSubresource(m_pWVPBuffer, 0, NULL, &wvp, 0, 0);
	}
#endif
}
#else

void BasicRenderer::SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix) {
	for (int i = 0; i < 16; ++i) {
		m_ViewMatrix.elements[i] = (float)(pViewMatrix[i]);
	}
	for (int i = 0; i < 16; ++i) {
		m_ProjectionMatrix.elements[i] = (float)(pProjectionMatrix[i]);
	}

	{
		if (!m_pWVPBuffer) {
			D3D11_BUFFER_DESC bd;
			bd.ByteWidth = sizeof(mat44f) * 3 + sizeof(float) * 4;	//16の倍数である必要がある//
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			//SUBRESOURCEによる初期値は設定しない

			HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pWVPBuffer);
			if (FAILED(hr))
				return;
		}

		//変換行列
		mat44f world_matrix;
		world_matrix.SetIdentity();


		//定数バッファの書き込み.
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_pD3DDeviceContext->Map(m_pConstantBuffers[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CopyMemory(mappedResource.pData, &world_matrix, sizeof(mat44f));
		CopyMemory((mat44f*)mappedResource.pData + 1, &m_ViewMatrix, sizeof(mat44f));
		CopyMemory((mat44f*)mappedResource.pData + 2, &m_ProjectionMatrix, sizeof(mat44f));
		//CopyMemory((mat44f*)mappedResource.pData + 3, &m_pPointSize, sizeof(vec2f));
		m_pD3DDeviceContext->Unmap(m_pConstantBuffers[0], 0);
	}
}
#endif
/*
void BasicRenderer::SetViewAndProjectionMatrix(const double* pViewMatrix, const double* pProjectionMatrix){
	for (int i = 0; i < 16; ++i){
		m_ViewMatrix.elements[i] = (float)(pViewMatrix[i]);
	}
	for (int i = 0; i < 16; ++i) {
		m_ProjectionMatrix.elements[i] = (float)(pProjectionMatrix[i]);
	}
}
*/


void BasicRenderer::UpdateInstanceBuffer(LPVOID pBuffer, UINT count, UINT stride) {



	if (count * stride > m_instanceBufSz) {
		//インスタンスバッファの作成・サイズ変更


		//すでにバッファがある場合は記憶しておき,
		//新しいバッファをGPUにセットしてから古いほうをRelease	
		ID3D11Buffer* m_pOldBuffer = NULL;
		if (m_pInstanceBuffer) {
			m_pOldBuffer = m_pInstanceBuffer;
		}

		m_instanceBufSz = count * stride;

		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = stride * count;
		bd.Usage = D3D11_USAGE_DEFAULT;		//D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;				//D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pInstanceBuffer);
		if (FAILED(hr))
			return;

		if (m_pOldBuffer) m_pOldBuffer->Release();

	}///バッファの作成終わり

	if (pBuffer == NULL) {//NULLの場合はバッファ作成だけして終了.
		return;
	}

	
	UINT offset = 0;
	//m_dx_com->pd3dDC->IASetInputLayout( m_pVertexLayout );
	m_pD3DDeviceContext->IASetVertexBuffers(1, 1, &m_pInstanceBuffer, &stride, &offset);


	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = stride * count;
	destRegion.top = 0;
	destRegion.bottom = 1;
	destRegion.front = 0;
	destRegion.back = 1;
	m_pD3DDeviceContext->UpdateSubresource(m_pInstanceBuffer, 0, &destRegion, pBuffer, 0, 0);

	m_instance_stride = stride;

	return;

}






//exeと同じフォルダにあるファイルパスへ直す.
int GetAttachedFileName(TCHAR* filepath, const TCHAR *filename){
	//TCHAR filepath[MAX_PATH];
	int length = GetModuleFileName(NULL, filepath, MAX_PATH);
	if(length <= 0){ return -1;}

	TCHAR* p = _tcsrchr(filepath, _T('\\'));
	if(! p ) { return -1;}

	_tcscpy(p + 1, filename);
	return 0;
}
