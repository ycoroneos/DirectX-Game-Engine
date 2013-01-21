#ifndef _LIGHT_H_
#define _LIGHT_H_
#define ATTENUATION_ASSUMPTION 0.03
#define POINT_LIGHT 2
#define DIRECTIONAL_LIGHT 3
#define SPOT_LIGHT 4
#define AMBIENT_LIGHT 5
#include "GeometryHelper.h"
#include <vector>
#include <algorithm>
using namespace std;
////////////////////////////////
/*struct LightVertex
{
    XMFLOAT3 Pos;  // Position
	XMFLOAT3 Normal;
};*/

////////////////////////////////

class LightMesh
{
private:
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT numIndexes;
public:
	string name;
	LightMesh()
	{
		VertexBuffer=NULL;
		IndexBuffer=NULL;
		numIndexes=0;
	}
	///////////////////////////////////////////////////////////////////
	void init(ID3D11Device* device, ID3D11DeviceContext* dcon, LightVertex* vertices, WORD* indices, int numIndex, int numVertex)
	{
		numIndexes=numIndex;
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( LightVertex ) * numVertex;
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
	/////////////////////////////////////////////////////////////////////////////
	void Draw(ID3D11DeviceContext* dcon)
	{
		UINT stride = sizeof( LightVertex );
		UINT offset = 0;
		dcon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		dcon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		dcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dcon->DrawIndexed(numIndexes, 0, 0);
	}
	///////////////////////////////////////////////////////////////////////////
	~LightMesh()
	{
		VertexBuffer->Release();
		IndexBuffer->Release();
	}
};
//////////////////////////////////////////

//////////////////////////////////////////
class Light
{
private:
	LightMesh *mesh;
	XMFLOAT3 color;
	XMFLOAT3 pos;
	float linearatten;
	float constantatten;
	float quadatten;
	float aradius;
	float bradius;
public:
	Light()
	{
	}

	void init(XMFLOAT3 position, float attconstant, float attlinear, float attquad, int type, ID3D11Device* device, ID3D11DeviceContext* dcon)
	{
		pos=position;
		linearatten=attlinear;
		constantatten=attconstant;
		quadatten=attquad;
		if (type==POINT_LIGHT)
		{
			bradius=0;
			aradius=attlinear/ATTENUATION_ASSUMPTION;
			WORD* pwIndices=NULL;
			LightVertex* pVertices=NULL;
			UINT cFaces = 2 * ( 8 - 1 ) * 8;
			UINT cVertices = ( 8 - 1 ) *8 + 2;

			pVertices = new LightVertex[ cVertices ];
			pwIndices = new WORD[ cFaces * 3 ];
		
			MakeSphere( pVertices, pwIndices, aradius, 8, 8 );
			mesh = new LightMesh;
			mesh->init(device, dcon, pVertices, pwIndices, cFaces*3, cVertices);
			
			delete [] pVertices;
			delete [] pwIndices;
		}
		if (type==DIRECTIONAL_LIGHT)
		{
		}
	}

	void Draw(ID3D11DeviceContext* dcon)
	{
		mesh->Draw(dcon);
	}

	XMFLOAT3 getPos()
	{
		return pos;
	}
};
///////////////////////////////
class Sorter
{
private:
	Light* lights;
	DeferredRTShader shader;
	vector<Light> important;
	vector<Light> lessimportant;
	vector<Light> dontrender;
	int numberLights;
	////////////////////////////////////
	ID3D11RenderTargetView* LightAccumBuffer;
	ID3D11Texture2D* LightBuf; ID3D11ShaderResourceView* LightBufView;
public:
	
	Sorter()
	{
		LightAccumBuffer=NULL;
		LightBuf=NULL;
		LightBufView=NULL;
	}
	////////////////////////////////////////////////////////
	void LoadLightFile(const char* file, ID3D11Device* device, ID3D11DeviceContext* dcon)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, 
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_OptimizeMeshes |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FlipUVs);
		
		aiLight** scenelights = scene->mLights;
		numberLights=scene->mNumLights;
		lights=new Light[numberLights];

		LightVertex* verts;
		WORD* indices;
		
		for (int k=0; k<numberLights; ++k)
		{
			lights[k].init(aiVec3ToXMFLOAT3(scene->mLights[k]->mPosition),scene->mLights[k]->mAttenuationConstant,scene->mLights[k]->mAttenuationLinear, scene->mLights[k]->mAttenuationQuadratic, scene->mLights[k]->mType, device, dcon);
		}
	}
	////////////////////////////////////////////////////////
	HRESULT init(ID3D11Device* device, int width, int height)
	{
		HRESULT hr;
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
		hr = device->CreateTexture2D( &tdesc, NULL, &LightBuf );
		if( FAILED(hr) )
        return hr;

		D3D11_RENDER_TARGET_VIEW_DESC vdesc;
		vdesc.Format=tdesc.Format;
		vdesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipSlice = 0;
		hr=device->CreateRenderTargetView(LightBuf, &vdesc, &LightAccumBuffer);
		if( FAILED(hr) )
        return hr;

		D3D11_SHADER_RESOURCE_VIEW_DESC rvdesc;
		ZeroMemory(&rvdesc, sizeof(rvdesc));
		rvdesc.Format = tdesc.Format;
		rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		rvdesc.Texture2D.MostDetailedMip = 0;
		rvdesc.Texture2D.MipLevels = 1;
		hr=device->CreateShaderResourceView(LightBuf, &rvdesc, &LightBufView);
		if( FAILED(hr) )
        return hr;
		shader.init(device, L"LightAccum.fx");
	}
	
	double dist(XMFLOAT3 pos1, XMFLOAT3 pos2)
	{
		return sqrt((pos2.z-pos1.z)*(pos2.z-pos1.z)+(pos2.y-pos1.y)*(pos2.y-pos1.y)+(pos2.x+pos1.x)*(pos2.x+pos1.x));
	}

	void Sort(XMFLOAT3 campos)
	{
		//for (int k=0; k<sizeof(meshes); ++k)
		{
			//Light light;
			//if (meshes[k].name[0]=='P')
			{
			//	light.type=POINT_LIGHT;
			}
		}

		for (int i=0; i<dontrender.size(); ++i)
		{
			if (dist(campos, dontrender[i].getPos())<20)
			{
				important.push_back(dontrender[i]);
				dontrender.erase(dontrender.begin()+(i-1));
			}
			else if (dist(campos, dontrender[i].getPos())<80)
			{
				lessimportant.push_back(dontrender[i]);
				dontrender.erase(dontrender.begin()+(i-1));
			}
			else if (dist(campos, dontrender[i].getPos())>=80)
			{
				//do nothing, dont render it!
			}
		}
	}
	
	void LightScene(ID3D11DeviceContext* dcon, XMFLOAT4X4* View, XMFLOAT4X4* Projection, XMFLOAT3* camera)
	{
		XMMATRIX World=XMMatrixIdentity();
		//dcon->OMSetRenderTargets(1, &LightAccumBuffer, NULL);
		shader.mats.projectionMatrix=XMMatrixTranspose(XMLoadFloat4x4(Projection));
		shader.mats.viewMatrix=XMMatrixTranspose(XMLoadFloat4x4(View));
		shader.cam.cameraPosition=XMLoadFloat3(camera);
		for (int i=0; i<numberLights; ++i)
		{
			XMFLOAT3 pos=lights[i].getPos();
			World*=XMMatrixTranslation(pos.x, pos.y, pos.z);
			shader.mats.worldMatrix=XMMatrixTranspose(World);
			shader.CommitUniforms(dcon);
			lights[i].Draw(dcon);
			World=XMMatrixIdentity();
		}
	}

	~Sorter()
	{
		important.clear();
		lessimportant.clear();
		LightAccumBuffer->Release();
		LightBuf->Release();
		LightBufView->Release();
	}
};

#endif