
class DeferredRender
{
private:
	ID3D11RenderTargetView* GBuffer[2];
	ID3D11Texture2D* Color; ID3D11ShaderResourceView* ColorView;
	ID3D11Texture2D* Normal; ID3D11ShaderResourceView* NormalView;
	//ID3D11Texture2D* Depth; ID3D11ShaderResourceView* DepthView;

	ID3D11Buffer* VertexBuffer;
	QuadShader shader;
public:
	struct QuadVertex
		{
			XMFLOAT3 Pos;
		};
	~DeferredRender()
	{
		Color->Release();
		Normal->Release();
		//Depth->Release();
		ColorView->Release();
		NormalView->Release();
		//DepthView->Release();
		GBuffer[0]->Release();
		GBuffer[1]->Release();
		//GBuffer[2]->Release();
	}
	ID3D11ShaderResourceView* GetColorView()
	{
		return ColorView;
	}
	ID3D11ShaderResourceView* GetNormalView()
	{
		return NormalView;
	}
	/*
	ID3D11ShaderResourceView* GetDepthView()
	{
		//return DepthView;
	}
	*/
	HRESULT makequad(ID3D11Device* device, int width, int height)
	{
		
		QuadVertex quad[]=
		{
			XMFLOAT3(-1, 1, 0)
		};
		VertexBuffer=NULL;

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( QuadVertex ) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = quad;
		HRESULT hr=device->CreateBuffer( &bd, &InitData, &VertexBuffer );
		return hr;
	}
	HRESULT init(ID3D11Device* device, int width, int height)
	{
		HRESULT hr;
		Color=NULL;
		Normal=NULL;
		//Depth=NULL;
		//DepthView=NULL;
		ZeroMemory(GBuffer, sizeof(GBuffer));
		/////////////////////////////////////////////////////////////////////////////////
		D3D11_TEXTURE2D_DESC tdesc;
		ZeroMemory( &tdesc, sizeof(tdesc) );
		tdesc.Width = width;
		tdesc.Height = height;
		tdesc.MipLevels = 1;
		tdesc.ArraySize = 1;
		tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tdesc.SampleDesc.Count = 1;
		tdesc.SampleDesc.Quality = 0;
		tdesc.Usage = D3D11_USAGE_DEFAULT;
		tdesc.BindFlags =D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		tdesc.CPUAccessFlags = 0;
		tdesc.MiscFlags = 0;
		hr = device->CreateTexture2D( &tdesc, NULL, &Color );
		if( FAILED(hr) )
        return hr;

		D3D11_RENDER_TARGET_VIEW_DESC vdesc;
		vdesc.Format=tdesc.Format;
		vdesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipSlice = 0;
		hr=device->CreateRenderTargetView(Color, &vdesc, &GBuffer[0]);
		if( FAILED(hr) )
        return hr;

		D3D11_SHADER_RESOURCE_VIEW_DESC rvdesc;
		ZeroMemory(&rvdesc, sizeof(rvdesc));
		rvdesc.Format = tdesc.Format;
		rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		rvdesc.Texture2D.MostDetailedMip = 0;
		rvdesc.Texture2D.MipLevels = 1;
		hr=device->CreateShaderResourceView(Color, &rvdesc, &ColorView);
		if( FAILED(hr) )
        return hr;
		///////////////////////////////////////////////////////////////////////////
		ZeroMemory( &tdesc, sizeof(tdesc) );
		tdesc.Width = width;
		tdesc.Height = height;
		tdesc.MipLevels = 1;
		tdesc.ArraySize = 1;
		tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tdesc.SampleDesc.Count = 1;
		tdesc.SampleDesc.Quality = 0;
		tdesc.Usage = D3D11_USAGE_DEFAULT;
		tdesc.BindFlags =D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		tdesc.CPUAccessFlags = 0;
		tdesc.MiscFlags = 0;
		hr = device->CreateTexture2D( &tdesc, NULL, &Normal );
		if( FAILED(hr) )
        return hr;

		ZeroMemory(&rvdesc, sizeof(rvdesc));
		vdesc.Format=tdesc.Format;
		vdesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipSlice = 0;
		hr=device->CreateRenderTargetView(Normal, &vdesc, &GBuffer[1]);
		if( FAILED(hr) )
        return hr;

		ZeroMemory(&rvdesc, sizeof(rvdesc));
		rvdesc.Format = tdesc.Format;
		rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		rvdesc.Texture2D.MostDetailedMip = 0;
		rvdesc.Texture2D.MipLevels = 1;
		hr=device->CreateShaderResourceView(Normal, &rvdesc, &NormalView);
		if( FAILED(hr) )
        return hr;

		hr=makequad(device, width, height);
		shader.init(device, L"QuadShader.fx");

		if( FAILED(hr))
			return hr;
		////////////////////////////////////////////////////////////////////////////
		/*
		ZeroMemory( &tdesc, sizeof(tdesc) );
		tdesc.Width = width;
		tdesc.Height = height;
		tdesc.MipLevels = 1;
		tdesc.ArraySize = 1;
		tdesc.Format = DXGI_FORMAT_R8_UNORM;
		tdesc.SampleDesc.Count = 1;
		tdesc.SampleDesc.Quality = 0;
		tdesc.Usage = D3D11_USAGE_DEFAULT;
		tdesc.BindFlags =D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		tdesc.CPUAccessFlags = 0;
		tdesc.MiscFlags = 0;
		hr = device->CreateTexture2D( &tdesc, NULL, &Depth );
		if( FAILED(hr) )
        return hr;

		ZeroMemory(&rvdesc, sizeof(rvdesc));
		vdesc.Format=tdesc.Format;
		vdesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipSlice = 0;
		hr=device->CreateRenderTargetView(Depth, &vdesc, &GBuffer[2]);
		if( FAILED(hr) )
        return hr;

		ZeroMemory(&rvdesc, sizeof(rvdesc));
		rvdesc.Format = tdesc.Format;
		rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		rvdesc.Texture2D.MostDetailedMip = 0;
		rvdesc.Texture2D.MipLevels = 1;
		hr=device->CreateShaderResourceView(Depth, &rvdesc, &DepthView);
		if( FAILED(hr) )
        return hr;
		*/
	}

	void BindTargets(ID3D11DeviceContext* dcon, ID3D11DepthStencilView* DepthStencilView)
	{
		ID3D11ShaderResourceView* pNullViews[3];
		pNullViews[0]=NULL; pNullViews[1]=NULL; pNullViews[2]=NULL;
		dcon->PSSetShaderResources(1, 3, pNullViews);
		dcon->OMSetRenderTargets(2, GBuffer, DepthStencilView);
	}

	void BindTextureTargets(ID3D11DeviceContext* dcon, ID3D11ShaderResourceView* DepthView)
	{
		ID3D11RenderTargetView* pNullRTV[3];
		pNullRTV[0]=NULL; pNullRTV[1]=NULL; pNullRTV[2]=NULL;
		dcon->OMSetRenderTargets(3, pNullRTV, NULL);
		dcon->PSSetShaderResources(0, 0, NULL);
		dcon->PSSetShaderResources(1, 1, &DepthView);
		//dcon->PSSetShaderResources(2, 1, &ColorView);
		dcon->PSSetShaderResources(3, 1, &NormalView);
		dcon->OMSetRenderTargets(1, &GBuffer[0], NULL);
	}

	void ClearGBuffer(ID3D11DeviceContext* dcon)
	{
		const float ccolor[4]={0.0f, 0.0f, 0.0f, 0.0f};
		const float cnormal[4]={0.5f, 0.5f, 0.5f, 0.0f}; 
		const float ddepth[1]={0};
		dcon->ClearRenderTargetView(GBuffer[0], ccolor);
		dcon->ClearRenderTargetView(GBuffer[1], cnormal);
		//dcon->ClearRenderTargetView(GBuffer[2], ddepth);
	}

	void PresentColorBuffer(ID3D11DeviceContext* dcon, ID3D11RenderTargetView* MainView)
	{
		dcon->OMSetRenderTargets(1, &MainView, NULL);
		//ID3D11ShaderResourceView* pNullViews[3];
		//pNullViews[0]=ColorView; pNullViews[1]=NULL; pNullViews[2]=NULL;
		dcon->PSSetShaderResources(1, 1, &ColorView);
		shader.CommitUniforms(dcon);
		UINT stride = sizeof( QuadVertex );
		UINT offset = 0;
		dcon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		dcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		dcon->Draw(1, 0);
		shader.CleanUp(dcon);
	}
};