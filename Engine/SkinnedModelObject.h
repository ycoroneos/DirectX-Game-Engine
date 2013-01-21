class SkinnedModelObject : public ModelObject
{
private:
	Skeleton bones;
	BoneFrame* pRootBone;
	int numBones;
public:
	SkinnedModelObject(BoneFrame* in) : ModelObject()
	{
		pRootBone=new BoneFrame();
		CopyBones(pRootBone, in);
		in=NULL;
		//pRootBone->name;
	}
	~SkinnedModelObject()
	{
	}
	
	void LoadTexture(ID3D11Device* device)
	{
			if (!D3DX11CreateShaderResourceViewFromFile(device, texture, NULL, NULL, &Tex, NULL) || texture==NULL)
				D3DX11CreateShaderResourceViewFromFile(device, L"water.jpg", NULL, NULL, &Tex, NULL);
	}

	void init(ID3D11Device* device, ID3D11DeviceContext* dcon, SkinnedVertex* vertices, WORD* indices, int numIndex, int numVertex)
	{
		numIndexes=numIndex;
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( SkinnedVertex ) * numVertex;
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
	
	void RecalcMatrix(XMMATRIX in)
	{
		//CalculateWorldMatrices(&pRootBone, in);
	}
	
	void LoadMatrices(XMMATRIX* in)
	{
		GetMats(pRootBone, &(*in), 0);
	}
	
	void Draw(ID3D11DeviceContext* dcon)
	{
		dcon->PSSetShaderResources(0, 1, &Tex);
		UINT stride = sizeof( SkinnedVertex );
		UINT offset = 0;
		dcon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		dcon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		dcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dcon->DrawIndexed(numIndexes, 0, 0);
		//dcon->PSSetShaderResources(0, 1, NULL);
	}
};