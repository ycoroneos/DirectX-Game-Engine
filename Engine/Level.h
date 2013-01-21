#include "definitions.h"
#include "Shaders.h"
#include "Deferred.h"
#include "ModelObject.h"
#include "Skeleton.h"
#include "SkinnedModelObject.h"
#include "OBJLib.h"
#include "mesh_loader.h"
#include "Light.h"
#include "HeightMap.h"
#include "PhysxModule.h"
#include "Camera.h"
#include "Controllable_Character.h"
#include "Input.h"

class Level
{
private:
	ObjectLibrary lib;
	DeferredRender renderer;
	Sorter LightSorter;
	CameraObject Camera;
	CharacterController Character;
	DXINPUT InputDevices;
	PhysxSDK PxSDK;

	HeightMap Map;

	XMMATRIX World, View, Projection;
public:
	~Level()
	{
		PxSDK.Release();
	}
	bool LoadLevel(std::string level, std::string basedir, ID3D11Device* device, ID3D11DeviceContext* dcon, int width, int height, DXINPUT devices, bool debug=false)
	{
		InputDevices=devices;

		ifstream infile;
		infile.open(basedir+"\\"+level);
		//infile.open("C:\\Users\\Rula\\Documents\\Visual Studio 2010\\Projects\\Engine\\Engine\\Level1\\Level_1.txt");
		if (!infile.good())
			return false;

		std::string object_dir, light_dir, physics_dir, HeightName, HeightTex, ObjectName, PhysicsName, LightFile;
		char temp[256];
		float HeightFieldScale;
		std::string line;
		std::vector<const char*> meshes;
		while (infile.good())
		{
			line.clear();
			getline(infile, line);
			
			if (sscanf(line.c_str(), "Object Dir = %s", object_dir.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Light Dir = %s", light_dir.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Physics Dir = %s", physics_dir.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Heightfield = %s", HeightName.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Heightfield Texture = %s", HeightTex.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Heightfield Scale = %f", &HeightFieldScale, 60))
				continue;
			if (sscanf(line.c_str(), "Object Library = %s", ObjectName.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Mesh = %s", temp, 60)>0)
			{
				meshes.push_back(temp);
				continue;
			}
			if (sscanf(line.c_str(), "Physics File = %s", PhysicsName.c_str(), 60))
				continue;
			if (sscanf(line.c_str(), "Lighting Description = %s", LightFile.c_str(), 60))
				continue;
		}
		infile.close();
		
		


		char* input=new char[basedir.size()+HeightTex.size()+2];
		sprintf(input, "%s\\%s", basedir.c_str(), HeightTex.c_str());
		WCHAR* out= new WCHAR[basedir.size()+HeightTex.size()];
		MultiByteToWideChar
		(
			CP_ACP,         
			0,         
			input, 
			-1,       
			out,  
			256        
		);
		ZeroMemory(input, sizeof(input));

		input= new char[basedir.size()+object_dir.size()+ObjectName.size()+2];
		sprintf(input, "%s%s\\%s", basedir.c_str(), object_dir.c_str(), ObjectName.c_str());
		//lib.LoadOBJ(input, device, dcon);

		for (int i=0; i<meshes.size(); ++i)
		{
			LoadMesh(meshes[i], &lib, device, dcon);
		}

		ZeroMemory(input, sizeof(input));
		input= new char[basedir.size()+HeightName.size()+2];
		sprintf(input, "%s\\%s", basedir.c_str(), HeightName.c_str());
		Map.create(device, dcon, input, out, HeightFieldScale);

		LightSorter.init(device, width, height);

		ZeroMemory(input, sizeof(input));
		input= new char[basedir.size()+light_dir.size()+2+LightFile.size()];
		sprintf(input, "%s%s\\%s", basedir.c_str(), light_dir.c_str(), LightFile.c_str());
		LightSorter.LoadLightFile(input, device, dcon);

		PxSDK.MakeScene();
		if (debug==true)
			PxSDK.EnableVisualDebug();
		PxSDK.MakeHeightFieldTMesh(Map.GetVertices(), Map.GetIndices(), &Map, Map.GetNumVerts(), Map.GetNumIndices());
		//PxSDK.MakeHeightField(heights, &Map);
		//PxSDK.MakeHeightFieldConvex(heights, &Map, Map.GetNumVerts());

		ZeroMemory(input, sizeof(input));
		input= new char[basedir.size()+physics_dir.size()+2+PhysicsName.size()];
		sprintf(input, "%s%s\\%s", basedir.c_str(), physics_dir.c_str(), PhysicsName.c_str());
		//PxSDK.LoadFile(input, &lib);
		
		ZeroMemory(input, sizeof(input));

		Camera.init(XMFLOAT3(0.0f, 0.0f, 10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), 0.1f, 10000.0f, width, height);
		Camera.SetBox_Physx(&lib, &PxSDK);
		//Character.init(&PxSDK, lib.skinnedObjects[0], width, height);
	}

	XMFLOAT4X4 GetView()
	{
		return Camera.GetView();
	}

	XMFLOAT4X4 GetProjection()
	{
		return Camera.GetProjection();
	}

	XMFLOAT3 GetPosition()
	{
		return Camera.GetPosition();
	}

	PhysxSDK GetPhysx()
	{
		return PxSDK;
	}

	void PreRender()
	{
		InputDevices.UpdateFreeCamera(&Camera);
		//InputDevices.UpdateController(&Character);
		PxSDK.Simulate();
	}

	void Render(GBuffShader* shader, SkinShader* skinning, ID3D11DeviceContext* dcon)
	{
		//CameraObject temp_cam=Character.GetCamera();
		shader->cam.cameraPosition=XMLoadFloat3(&Camera.GetPosition());
		//shader->cam.cameraPosition=XMLoadFloat3(&temp_cam.GetPosition());
		//skinning->cam.cameraPosition=XMLoadFloat3(&temp_cam.GetPosition());
		shader->mats.viewMatrix = XMMatrixTranspose( XMLoadFloat4x4(&Camera.GetView()) );
		//shader->mats.viewMatrix = XMMatrixTranspose( XMLoadFloat4x4(&temp_cam.GetView()) );
		//skinning->mats.viewMatrix = XMMatrixTranspose( XMLoadFloat4x4(&temp_cam.GetView()) );
		shader->mats.projectionMatrix = XMMatrixTranspose( XMLoadFloat4x4(&Camera.GetProjection()) );
		//shader->mats.projectionMatrix = XMMatrixTranspose( XMLoadFloat4x4(&temp_cam.GetProjection()));
		shader->object.DiffuseRoughness=XMFLOAT4(0.0f, 0.0f, 0.0f, 1.43);
		shader->object.Specular=XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

		//skinning->mats.projectionMatrix = XMMatrixTranspose( XMLoadFloat4x4(&temp_cam.GetProjection()));
		skinning->object.DiffuseRoughness=XMFLOAT4(0.0f, 0.0f, 0.0f, 1.43);
		skinning->object.Specular=XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

		//shader->mats.worldMatrix = XMMatrixTranspose( XMMatrixIdentity() );
		//shader->CommitUniforms(dcon);
		//lib.objects[0].Draw(dcon);
		//Map.Draw(dcon);

		PxSDK.PxDraw(dcon, shader, skinning);
	}

	void RenderLighting(ID3D11DeviceContext* dcon)
	{
		LightSorter.LightScene(dcon, &Camera.GetView(), &Camera.GetProjection(), &Camera.GetPosition());
	}

	void WhileWaitingOnPhysics()
	{
	}
};