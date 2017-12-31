#pragma once

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11DeviceContext;

class RectDrawer
{
public:
	struct ShaderConstants
	{
		FLOAT		transformMatrix[16];
		FLOAT		color[4];
	};
	RectDrawer();
	~RectDrawer();
	bool Init(ID3D11Device *pDevice);
	void DrawRect(ID3D11DeviceContext *pContext, float left, float top, float right, float bottom, UINT32 color);

private:
	bool							m_inited;
	ID3D11Buffer *					m_pBuffer;
	ID3D11PixelShader *				m_pPixelShader;
	ID3D11VertexShader *			m_pVertexShader;
};
