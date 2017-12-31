#include "RectDrawer.h"
#include <D3D11.h>
#include <d3dcompiler.h>

RectDrawer::RectDrawer() : m_pBuffer(nullptr), m_pPixelShader(nullptr), m_pVertexShader(nullptr), m_inited(false)
{

}


RectDrawer::~RectDrawer()
{
	if (m_pBuffer != nullptr) 			m_pBuffer->Release();
	if (m_pPixelShader != nullptr)		m_pPixelShader->Release();
	if (m_pVertexShader != nullptr)		m_pVertexShader->Release();
}


bool RectDrawer::Init(ID3D11Device *pDevice)
{
	pD3DCompile fnD3DCompile = nullptr;
	HMODULE hD3DCompiler = LoadLibrary(D3DCOMPILER_DLL);
	if (hD3DCompiler != nullptr)
	{
		fnD3DCompile = reinterpret_cast<pD3DCompile>(GetProcAddress(hD3DCompiler, "D3DCompile"));
		if (fnD3DCompile != nullptr)
		{
			const char vsQuad[] =
				"cbuffer ShaderConstants : register(b0) {"
				"	float4x4 TransformMatrix : packoffset(c0);"
				"};"
				"float4 VS(uint VertexIndex : SV_VertexID) : SV_Position{"
				"	const float2 corners[4] = {"
				"		float2(0.0f, 1.0f),"
				"		float2(0.0f, 0.0f),"
				"		float2(1.0f, 1.0f),"
				"		float2(1.0f, 0.0f)"
				"	};"
				"	return mul(TransformMatrix, float4(corners[VertexIndex].xy, 0.0f, 1.0f));"
				"}";
			ID3DBlob * pCode = nullptr;
			HRESULT hr = fnD3DCompile(vsQuad, sizeof(vsQuad), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, &pCode, NULL);
			if (FAILED(hr))
				return false;
			hr = pDevice->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &m_pVertexShader);
			pCode->Release();
			if (FAILED(hr))
				return false;

			const char psColor[] =
				"cbuffer ShaderConstants : register(b0) {"
				"	float4 Color : packoffset(c4);"
				"};"
				"float4 PS() : SV_Target {"
				"	return Color;"
				"}";
			hr = fnD3DCompile(psColor, sizeof(psColor), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, &pCode, NULL);
			if (FAILED(hr))
				return false;
			hr = pDevice->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &m_pPixelShader);
			pCode->Release();
			if (FAILED(hr))
				return false;

			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.ByteWidth = sizeof(ShaderConstants);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			hr = pDevice->CreateBuffer(&bufferDesc, NULL, &m_pBuffer);
			if (FAILED(hr))
				return false;

			m_inited = true;

			return true;
		}
	}
	return false;
}

void RectDrawer::DrawRect(ID3D11DeviceContext *pContext, float left, float top, float right, float bottom, UINT32 color) 
{
	if (!m_inited)
		return;

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pContext->PSSetShader(m_pPixelShader, NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, &m_pBuffer);
	pContext->VSSetShader(m_pVertexShader, NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, &m_pBuffer);
	pContext->GSSetShader(NULL, NULL, 0);

	ShaderConstants constants = { 0 };

	D3D11_VIEWPORT vp;
	UINT nvp = 1;
	pContext->RSGetViewports(&nvp, &vp);
	constants.transformMatrix[0] = (right - left) * 2.0f / vp.Width;
	constants.transformMatrix[12] = -1.0f + left * 2.0f / vp.Width;
	constants.transformMatrix[5] = (bottom - top) * -2.0f / vp.Height;
	constants.transformMatrix[13] = 1.0f + top * -2.0f / vp.Height;
	constants.transformMatrix[10] = 1.0f;
	constants.transformMatrix[15] = 1.0f;

	for (int i = 0; i<4; ++i)
		constants.color[i] = static_cast<FLOAT>((color >> (i << 3)) & 0xff) / 255.0f;

	pContext->UpdateSubresource(m_pBuffer, 0, NULL, &constants, 0, 0);

	pContext->Draw(4, 0);
}