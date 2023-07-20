#pragma once

#include <d3d11.h>
#include "aidx_saferelease.h"

namespace AIDX11 {


	class ConstantBuffer
	{
	private:
		ID3D11Device * m_pD3DDevice;
		ID3D11DeviceContext*    m_pD3DDeviceContext;

		ID3D11Buffer* m_pBuffers;
		UINT m_size;
		UINT m_stride;
		UINT m_slot;

	public:
		ConstantBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT slot) :
			m_pD3DDevice(device),
			m_pD3DDeviceContext(deviceContext),
			m_pBuffers(nullptr),
			m_size(0),
			m_slot(slot) {

		}

		virtual ~ConstantBuffer() {
			SafeRelease(m_pBuffers);
		}

		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(const ConstantBuffer&) = delete;


		void Update(LPCVOID pBuffer, UINT count, UINT stride) {

			if (count * stride > m_size) {
				//バッファの作成・サイズ変更

				if (m_pBuffers) {
					//ID3D11Buffer* m_pOldBuffer = m_pConstantBuffers;
					m_pBuffers->Release();
				}

				m_size = count * stride;

				D3D11_BUFFER_DESC bd;
				bd.ByteWidth = stride * count;
				bd.Usage = D3D11_USAGE_DEFAULT;		//D3D11_USAGE_DYNAMIC;
				bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				bd.CPUAccessFlags = 0;				//D3D11_CPU_ACCESS_WRITE;
				bd.MiscFlags = 0;
				bd.StructureByteStride = 0;

				HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pBuffers);
				if (FAILED(hr)){
					m_pBuffers = nullptr;
					return;
				}

				//if(m_pOldBuffer) m_pOldBuffer->Release();

			}///バッファの作成終わり

			if (pBuffer == nullptr) {//NULLの場合はバッファ作成だけして終了.
				return;
			}

			/*
			UINT offset = 0;
			m_stride = stride;
			m_pD3DDeviceContext->IASetVertexBuffers(m_slot, 1, &m_pConstantBuffers, &m_stride, &offset);
			*/

			//定数バッファーの場合は pDstBox を NULL に設定します。D3D11_BOXを指定すると失敗する//
			m_pD3DDeviceContext->UpdateSubresource(m_pBuffers, 0, NULL, pBuffer, 0, 0);

		}

		void AttachVS() {
			UINT offset = 0;
			m_pD3DDeviceContext->VSSetConstantBuffers(m_slot, 1, &m_pBuffers);
		}

	};
};

