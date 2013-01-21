#include <string>
using namespace std;

class ModelObject
{
private:
public:

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	ID3D11ShaderResourceView* Tex;
	UINT numIndexes;
	std::string name;
	WCHAR* texture;
	ModelObject()
	{
		VertexBuffer=NULL;
		IndexBuffer=NULL;
		Tex=NULL;
		numIndexes=0;
		texture=NULL;
	}

	~ModelObject()
	{
		if (VertexBuffer!=NULL)
		VertexBuffer->Release();
		if (IndexBuffer!=NULL)
		IndexBuffer->Release();
	}
	virtual void LoadTexture(ID3D11Device* device)
	{
			if (!D3DX11CreateShaderResourceViewFromFile(device, texture, NULL, NULL, &Tex, NULL) || texture==NULL)
				D3DX11CreateShaderResourceViewFromFile(device, L"water.jpg", NULL, NULL, &Tex, NULL);
	}
	virtual void init(ID3D11Device* device, ID3D11DeviceContext* dcon, SimpleVertex* vertices, WORD* indices, int numIndex, int numVertex)
	{
		numIndexes=numIndex;
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( SimpleVertex ) * numVertex;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		device->CreateBuffer( &bd, &InitData, &VertexBuffer );

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( WORD ) * numIndexes;        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		device->CreateBuffer( &bd, &InitData, &IndexBuffer );
	}
	virtual void Draw(ID3D11DeviceContext* dcon)
	{
		dcon->PSSetShaderResources(0, 1, &Tex);
		UINT stride = sizeof( SimpleVertex );
		UINT offset = 0;
		dcon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		dcon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		dcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dcon->DrawIndexed(numIndexes, 0, 0);
		//dcon->PSSetShaderResources(0, 1, NULL);
	}
};