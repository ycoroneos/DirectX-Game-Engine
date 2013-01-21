
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    //DWORD dwShaderFlags = D3DSHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
   // dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        NULL, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

class SamplerState
{
private:
	ID3D11SamplerState* Linear;
public:
	SamplerState()
	{
		Linear=NULL;
	}
	~SamplerState()
	{
		Linear->Release();
	}
	void Init(ID3D11Device* device, ID3D11DeviceContext* dcon)
	{
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState( &sampDesc, &Linear );
		dcon->PSSetSamplers(0, 1, &Linear);
	}
};

class FlatShader
{
private:
public:
	const char* name;

	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gConstantBuffer;
	
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

	ConstantBuffer cb1;
	FlatShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gConstantBuffer=NULL;
	}

	~FlatShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gConstantBuffer->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "VS", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "PS", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gConstantBuffer );
		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gConstantBuffer, 0, NULL, &cb1, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gConstantBuffer );
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};

class BlinnShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gMatrix;
	ID3D11Buffer* gCamera;
	ID3D11Buffer* gLight;

struct MatrixBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};
struct CameraBuffer
{
	XMVECTOR cameraPosition;
};
struct LightBuffer
{
	XMFLOAT4 lightDirection;
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specularColor;
	XMFLOAT4 specularPower;
};

	MatrixBuffer mats;
	CameraBuffer cam;
	LightBuffer light;

	BlinnShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gMatrix=NULL;
		gCamera=NULL;
		gLight=NULL;
	}

	~BlinnShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gMatrix->Release();
		gCamera->Release();
		gLight->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "LightVertexShader", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "LightPixelShader", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gMatrix);

		bd.ByteWidth=sizeof(CameraBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gCamera);

		bd.ByteWidth=sizeof(LightBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gLight);

		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gMatrix, 0, NULL, &mats, 0, 0);
		dcon->UpdateSubresource(gLight, 0, NULL, &light, 0, 0);
		dcon->UpdateSubresource(gCamera, 0, NULL, &cam, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gMatrix );
		dcon->VSSetConstantBuffers(1, 1, &gCamera);
		dcon->PSSetConstantBuffers(2, 1, &gLight);
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};


class RTShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gMatrix;
	ID3D11Buffer* gCamera;
	ID3D11Buffer* gLight;
	ID3D11Buffer* gObject;

struct MatrixBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};
struct CameraBuffer
{
	XMVECTOR cameraPosition;
};
struct LightBuffer
{
	XMFLOAT4 lightDirectionIntensity[9];
};
struct ObjectBuffer
{
	XMFLOAT4 DiffuseRoughness;
	XMFLOAT4 Specular;
};

	MatrixBuffer mats;
	CameraBuffer cam;
	LightBuffer light;
	ObjectBuffer object;

	RTShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gMatrix=NULL;
		gCamera=NULL;
		gLight=NULL;
		gObject=NULL;
	}

	~RTShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gMatrix->Release();
		gCamera->Release();
		gLight->Release();
		gObject->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "LightVertexShader", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "LightPixelShader", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gMatrix);

		bd.ByteWidth=sizeof(CameraBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gCamera);

		bd.ByteWidth=sizeof(LightBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gLight);

		bd.ByteWidth=sizeof(ObjectBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gObject);

		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gMatrix, 0, NULL, &mats, 0, 0);
		dcon->UpdateSubresource(gLight, 0, NULL, &light, 0, 0);
		dcon->UpdateSubresource(gCamera, 0, NULL, &cam, 0, 0);
		dcon->UpdateSubresource(gObject, 0, NULL, &object, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gMatrix );
		dcon->VSSetConstantBuffers(1, 1, &gCamera);
		dcon->PSSetConstantBuffers(2, 1, &gLight);
		dcon->PSSetConstantBuffers(3, 1, &gObject);
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};
class GBuffShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gMatrix;
	ID3D11Buffer* gCamera;
	ID3D11Buffer* gObject;

struct MatrixBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};
struct CameraBuffer
{
	XMVECTOR cameraPosition;
};
struct ObjectBuffer
{
	XMFLOAT4 DiffuseRoughness;
	XMFLOAT4 Specular;
};

	MatrixBuffer mats;
	CameraBuffer cam;
	ObjectBuffer object;

	GBuffShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gMatrix=NULL;
		gCamera=NULL;
		gObject=NULL;
	}

	~GBuffShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gMatrix->Release();
		gCamera->Release();
		gObject->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "LightVertexShader", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "LightPixelShader", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gMatrix);

		bd.ByteWidth=sizeof(CameraBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gCamera);

		bd.ByteWidth=sizeof(ObjectBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gObject);

		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gMatrix, 0, NULL, &mats, 0, 0);
		dcon->UpdateSubresource(gCamera, 0, NULL, &cam, 0, 0);
		dcon->UpdateSubresource(gObject, 0, NULL, &object, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gMatrix );
		dcon->VSSetConstantBuffers(1, 1, &gCamera);
		//dcon->PSSetShaderResources(1, 1, &ColorView);
		//dcon->PSSetShaderResources(2, 1, &NormalView);
		//dcon->PSSetShaderResources(3, 1, &DepthView);
		dcon->PSSetConstantBuffers(3, 1, &gObject);
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};
class SkinShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gMatrix;
	ID3D11Buffer* gCamera;
	ID3D11Buffer* gObject;

struct MatrixBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX skinmatrix[100];
	//XMMATRIX* skinmatrix;
};
struct CameraBuffer
{
	XMVECTOR cameraPosition;
};
struct ObjectBuffer
{
	XMFLOAT4 DiffuseRoughness;
	XMFLOAT4 Specular;
};

	MatrixBuffer mats;
	CameraBuffer cam;
	ObjectBuffer object;

	SkinShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gMatrix=NULL;
		gCamera=NULL;
		gObject=NULL;
	}

	~SkinShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gMatrix->Release();
		gCamera->Release();
		gObject->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "VSMAIN", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHTS"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{  "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "PSMAIN", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gMatrix);

		bd.ByteWidth=sizeof(CameraBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gCamera);

		bd.ByteWidth=sizeof(ObjectBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gObject);

		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gMatrix, 0, NULL, &mats, 0, 0);
		//dcon->UpdateSubresource(gCamera, 0, NULL, &cam, 0, 0);
		//dcon->UpdateSubresource(gObject, 0, NULL, &object, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gMatrix );
		//dcon->VSSetConstantBuffers(1, 1, &gCamera);
		//dcon->PSSetShaderResources(1, 1, &ColorView);
		//dcon->PSSetShaderResources(2, 1, &NormalView);
		//dcon->PSSetShaderResources(3, 1, &DepthView);
		dcon->PSSetConstantBuffers(3, 1, &gObject);
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};
class HeightMapShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gMatrix;
	ID3D11Buffer* gObject;

struct MatrixBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};
struct CameraBuffer
{
	XMVECTOR cameraPosition;
};

	MatrixBuffer mats;

	HeightMapShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gMatrix=NULL;
	}

	~HeightMapShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gMatrix->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "LightVertexShader", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "LightPixelShader", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gMatrix);

		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gMatrix, 0, NULL, &mats, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gMatrix );
		//dcon->PSSetShaderResources(1, 1, &ColorView);
		//dcon->PSSetShaderResources(2, 1, &NormalView);
		//dcon->PSSetShaderResources(3, 1, &DepthView);
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};
class DeferredRTShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* gMatrix;
	ID3D11Buffer* gCamera;

struct MatrixBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};
struct CameraBuffer
{
	XMVECTOR cameraPosition;
};

	MatrixBuffer mats;
	CameraBuffer cam;

	DeferredRTShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		gMatrix=NULL;
		gCamera=NULL;
	}

	~DeferredRTShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		VertexLayout->Release();
		gMatrix->Release();
		gCamera->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "LightVertexShader", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE( layout );
		device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &VertexLayout);
		pVSBlob->Release();

		CompileShaderFromFile( file, "LightPixelShader", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr=device->CreateBuffer( &bd, NULL, &gMatrix);

		bd.ByteWidth=sizeof(CameraBuffer);
		hr=device->CreateBuffer(&bd, NULL, &gCamera);

		if (FAILED(hr))
			return false;
		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)			//in future this will have to be split up into many smaller commit functions to improve constant buffer efficiency
	{
		dcon->IASetInputLayout(VertexLayout);
		dcon->UpdateSubresource(gMatrix, 0, NULL, &mats, 0, 0);
		dcon->UpdateSubresource(gCamera, 0, NULL, &cam, 0, 0);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->VSSetConstantBuffers( 0, 1, &gMatrix );
		dcon->VSSetConstantBuffers(1, 1, &gCamera);
		//dcon->PSSetShaderResources(1, 1, &ColorView);
		//dcon->PSSetShaderResources(2, 1, &NormalView);
		//dcon->PSSetShaderResources(3, 1, &DepthView);
		dcon->PSSetShader( PixelShader, NULL, 0 );
		return true;
	}
};
class QuadShader
{
private:
public:
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	ID3DBlob* pGSBlob;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11GeometryShader* GeometryShader;
	ID3D11InputLayout* VertexLayout;

	QuadShader()
	{
		pVSBlob=NULL;
		pPSBlob=NULL;
		pGSBlob=NULL;
		VertexShader=NULL;
		PixelShader=NULL;
		VertexLayout=NULL;
		GeometryShader=NULL;
	}

	~QuadShader()
	{
		VertexShader->Release();
		PixelShader->Release();
		GeometryShader->Release();
	}

	bool init(ID3D11Device* device, WCHAR* file)
	{
		CompileShaderFromFile(file, "QuadVertexShader", "vs_4_0", &pVSBlob );
		device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader);
		pVSBlob->Release();

		CompileShaderFromFile( file, "QuadPixelShader", "ps_4_0", &pPSBlob );
		device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
		pPSBlob->Release();

		CompileShaderFromFile( file, "GSMainQuad", "gs_4_0", &pGSBlob );
		device->CreateGeometryShader( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &GeometryShader );
		pGSBlob->Release();

		return true;
	}
	
	bool CommitUniforms(ID3D11DeviceContext* dcon)
	{
		//dcon->IASetInputLayout(VertexLayout);
		dcon->VSSetShader( VertexShader, NULL, 0 );
		dcon->PSSetShader( PixelShader, NULL, 0 );
		dcon->GSSetShader(GeometryShader, NULL, 0);
		return true;
	}
	bool CleanUp(ID3D11DeviceContext* dcon)
	{
		dcon->GSSetShader(NULL, NULL, 0);
		return true;
	}
};