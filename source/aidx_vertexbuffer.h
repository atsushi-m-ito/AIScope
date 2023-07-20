#pragma once

#include <d3d11.h>
#include "aidx_saferelease.h"

namespace AIDX11 {


	class VertexBuffer {
	private:
		ID3D11Device*           m_pD3DDevice;
		ID3D11DeviceContext*    m_pD3DDeviceContext;

		ID3D11Buffer* m_pVertexBuffer;
		UINT m_size ;
		UINT m_stride;
		UINT m_slot;

	public:
		VertexBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT slot) :
			m_pD3DDevice(device),
			m_pD3DDeviceContext(deviceContext),
			m_pVertexBuffer(nullptr),
			m_size(0),
			m_slot(slot)
		{

		}

		virtual ~VertexBuffer() {
			SafeRelease(m_pVertexBuffer);
		}

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;


		void Update(LPCVOID pBuffer, UINT count, UINT stride) {

			if (count * stride > m_size) {
				//バッファの作成・サイズ変更

				//すでにバッファがある場合は記憶しておき,
				//新しいバッファをGPUにセットしてから古いほうをRelease	
				ID3D11Buffer* m_pOldBuffer = NULL;
				if (m_pVertexBuffer) {
					m_pOldBuffer = m_pVertexBuffer;
					m_pOldBuffer->Release();
				}

				m_size = count * stride;

				D3D11_BUFFER_DESC bd;
				bd.ByteWidth = stride * count;
				bd.Usage = D3D11_USAGE_DEFAULT;		//D3D11_USAGE_DYNAMIC;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;				//D3D11_CPU_ACCESS_WRITE;
				bd.MiscFlags = 0;
				bd.StructureByteStride = 0;

				HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);
				//if (FAILED(hr)){
				if (hr != S_OK) {
					m_pVertexBuffer = NULL;
					return;
				}

				//if(m_pOldBuffer) m_pOldBuffer->Release();

			}///バッファの作成終わり

			if (pBuffer == NULL) {//NULLの場合はバッファ作成だけして終了.
				return;
			}

			UINT offset = 0;
			m_stride = stride;
			m_pD3DDeviceContext->IASetVertexBuffers(m_slot, 1, &m_pVertexBuffer, &m_stride, &offset);
			

			D3D11_BOX destRegion;
			destRegion.left = 0;
			destRegion.right = count * stride;
			destRegion.top = 0;
			destRegion.bottom = 1;
			destRegion.front = 0;
			destRegion.back = 1;
			m_pD3DDeviceContext->UpdateSubresource(m_pVertexBuffer, 0, &destRegion, pBuffer, 0, 0);
			
		}

		void Attach() {
			UINT offset = 0;
			m_pD3DDeviceContext->IASetVertexBuffers(m_slot, 1, &m_pVertexBuffer, &m_stride, &offset);
		}

	};
};

