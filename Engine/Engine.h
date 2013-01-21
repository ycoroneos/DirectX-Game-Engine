#ifndef _ENGINE_H_
#define _ENGINE_H_
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <xnamath.h>
#include "Level.h"
//#include "BulletModule.h"

void CleanupDevice();

ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11Texture2D*		g_pDepthStencil=NULL;
ID3D11ShaderResourceView* g_pDepthView=NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;

class Variables
{
public:
//RTShader light;
GBuffShader gbuf;
SkinShader skinning;
SamplerState Sampler;
ModelObject cube;
ObjectLibrary lib;
DeferredRender renderer;
Sorter LightSorter;
CameraObject Camera;
DXINPUT InputDevices;
//BulletSDK BtSDK;
PhysxSDK PxSDK;

HeightMap Map;

XMMATRIX World, View, Projection;
int width, height;
};
Variables variables;
Level TestLevel;

HRESULT InitDevice(HWND hwnd, HINSTANCE hInstance)
{
	HRESULT hr;
	RECT rt;
	GetClientRect(hwnd, &rt);
	variables.width= rt.right-rt.left;
	variables.height= rt.bottom-rt.top;

	DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = variables.width;
    sd.BufferDesc.Height = variables.height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

	UINT createDeviceFlags=0;
	#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numlevels=ARRAYSIZE(featureLevels);
	hr=D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numlevels, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
	if( FAILED( hr ) )
        return hr;

	ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

	// Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = variables.width;
    descDepth.Height = variables.height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED(hr) )
        return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC rvdesc;
	ZeroMemory(&rvdesc, sizeof(rvdesc));
	rvdesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rvdesc.Texture2D.MostDetailedMip = 0;
	rvdesc.Texture2D.MipLevels = 1;
	hr=g_pd3dDevice->CreateShaderResourceView(g_pDepthStencil, &rvdesc, &g_pDepthView);
	if( FAILED(hr) )
    return hr;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED(hr) )
        return hr;

	variables.renderer.init(g_pd3dDevice, variables.width, variables.height);
	

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)variables.width;
    vp.Height = (FLOAT)variables.height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

	variables.InputDevices.initDInput(hInstance, hwnd);

    return S_OK;
}

void SetupProg()
{
	if (variables.gbuf.init(g_pd3dDevice, L"GBuffer.fx")==false||variables.skinning.init(g_pd3dDevice, L"SkinGBuffer.fx")==false)
		CleanupDevice();
	variables.Sampler.Init(g_pd3dDevice, g_pImmediateContext);
	//XMFLOAT3 campos;
	TestLevel.LoadLevel("Level_1.con", "C:\\Users\\Rula\\Documents\\Visual Studio 2010\\Projects\\Engine\\Engine\\Level1", g_pd3dDevice, g_pImmediateContext, variables.width, variables.height, variables.InputDevices, true);
	///variables.lib.LoadOBJ("g36.3ds", g_pd3dDevice, g_pImmediateContext, NULL);
	///variables.Map.create(g_pd3dDevice, g_pImmediateContext, "Height_wake.raw", L"wake_map.dds", 100);
	///variables.LightSorter.init(g_pd3dDevice, variables.width, variables.height);
	///variables.LightSorter.LoadLightFile("box.3ds", g_pd3dDevice, g_pImmediateContext);
	//variables.BtSDK.MakeTestBox(&variables.gbuf, g_pImmediateContext, &variables.lib.objects[0]);
	//variables.BtSDK.LoadFile("box.bullet", &variables.lib, &variables.gbuf, g_pImmediateContext);
	//variables.BtSDK.EnableDebugDraw(g_pd3dDevice, g_pImmediateContext);
	///variables.PxSDK.MakeScene();
	///variables.PxSDK.EnableVisualDebug();
	//variables.PxSDK.MakeTestBox(&variables.lib.objects[0]);
	///variables.PxSDK.LoadFile("bf2.repx", &variables.lib);
	//variables.PxSDK.associate();

	//variables.World=XMMatrixIdentity();
	//variables.Camera.init(XMFLOAT3(0.0f, 0.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), 0.1f, 10000.0f, variables.width, variables.height);
	//variables.Projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, variables.width / (FLOAT)variables.height, 0.01f, 10000.0f );
}

void Render()
{
	/*
	variables.PxSDK.Simulate();
	
	variables.InputDevices.Update(&variables.Camera);
	*/

	TestLevel.PreRender();

	variables.renderer.ClearGBuffer(g_pImmediateContext);

	float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; //red,green,blue,alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	variables.renderer.BindTargets(g_pImmediateContext, g_pDepthStencilView);


	/*
	variables.gbuf.cam.cameraPosition=XMLoadFloat3(&variables.Camera.GetPosition());
	
	variables.gbuf.mats.viewMatrix = XMMatrixTranspose( XMLoadFloat4x4(&variables.Camera.GetView()) );
	variables.gbuf.mats.projectionMatrix = XMMatrixTranspose( XMLoadFloat4x4(&variables.Camera.GetProjection()) );
	variables.gbuf.object.DiffuseRoughness=XMFLOAT4(0.0f, 0.0f, 0.0f, 1.43);
	variables.gbuf.object.Specular=XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	
	
	
		variables.gbuf.mats.worldMatrix = XMMatrixTranspose( variables.World );
		variables.gbuf.CommitUniforms(g_pImmediateContext);
		//variables.lib.objects[0].Draw(g_pImmediateContext);
		variables.Map.Draw(g_pImmediateContext);
	
	//variables.BtSDK.Simulate(&variables.Camera.GetView(), &ProjectionX);
	
	
	variables.PxSDK.PxDraw(g_pImmediateContext, &variables.gbuf);
	*/
	TestLevel.Render(&variables.gbuf, &variables.skinning, g_pImmediateContext);

	variables.renderer.BindTextureTargets(g_pImmediateContext, g_pDepthView);

	///variables.LightSorter.LightScene(g_pImmediateContext, &variables.Camera.GetView(), &variables.Camera.GetProjection(), &variables.Camera.GetPosition());
	TestLevel.RenderLighting(g_pImmediateContext);

	variables.renderer.PresentColorBuffer(g_pImmediateContext, g_pRenderTargetView);

    g_pSwapChain->Present( 0, 0 );

	
	while (!TestLevel.GetPhysx().FetchResults())
	{
		TestLevel.WhileWaitingOnPhysics();
	}
	
}

void CleanupDevice()
{
	variables.PxSDK.Release();
	if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
	if (g_pDepthView) g_pDepthView->Release();
	if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}

#endif