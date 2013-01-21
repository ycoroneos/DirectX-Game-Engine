#include <pxphysicsapi.h>
#include <PxExtensionsAPI.h>
#include <pxdefaulterrorcallback.h>
#include <pxdefaultallocator.h>
#include <PxDefaultSimulationFilterShader.h>
#include <PxDefaultCpuDispatcher.h>
#include <PxShapeExt.h>
#include <PxSimpleFactory.h>
#include <PxStringTable.h>
#include <PxStringTableExt.h>
#include <RepXUtility.h>
#include <PxScene.h>
#include <PxToolkit.h>
#include <PxCharacter.h>
#include <PxCapsuleController.h>
#include <PxControllerManager.h>

/*
Foundation
PhysX3Common
PhysX3CharacterDynamic
PhysX3CharacterKinematic
PhysX3Vehicle
PhysX3Cooking
PhysX3
PhysX3Extensions
LowLevel
GeomUtils
SceneQuery
SimulationController
PvdRuntime
RepX3
*/ //-lPhysX3Cooking -lPhysX3 -lSimulationController -lLowLevel -lPhysXProfileSDK -lPhysX3Extensions -lFoundation -lSceneQuery -lGeomUtils -lPhysX3Common -lPhysX3CharacterKinematic -lPvdRuntime -lPhysX3MetaData -lRepX3 -lPxTask -lPhysX3Vehicle

#pragma comment(lib, "PhysX3Cooking_x64.lib")
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "SimulationController.lib") 
#pragma comment (lib, "LowLevel.lib")
#pragma comment (lib, "PhysXProfileSDK.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "SceneQuery.lib") 
#pragma comment(lib, "GeomUtils.lib") 
#pragma comment (lib, "PhysX3Common.lib")
#pragma comment (lib, "PhysX3CharacterKinematic.lib")
#pragma comment(lib, "PvdRuntime.lib") 
#pragma comment (lib, "PhysX3MetaData.lib")
#pragma comment(lib, "RepX3.lib") 
#pragma comment(lib, "PxTask.lib")
#pragma comment (lib, "PhysX3Vehicle.lib")
#pragma comment (lib, "PxToolkit.lib")
//
//
/*
class RepXIdMap : public physx::repx::RepXIdToRepXObjectMap
{
private:
public:
	void destroy()
	{
	}
	void addLiveObject( physx::repx::RepXObject inLiveObject )
	{
	 
	}
	physx::repx::RepXObject getLiveObjectFromId( const void* inId )
	{
		physx::repx::
	}
};
class RepXResultHandler : public physx::repx::RepXInstantiationResultHandler
{
private:
public:
	void addInstantiationResult( physx::repx::RepXInstantiationResult inResult )
	{
		inResult.mLiveObject;
	}
};
*/

void getColumnMajor(PxMat33 m, PxVec3 t, float* mat) {
   mat[0] = m.column0[0];
   mat[1] = m.column0[1];
   mat[2] = m.column0[2];
   mat[3] = 0;

   mat[4] = m.column1[0];
   mat[5] = m.column1[1];
   mat[6] = m.column1[2];
   mat[7] = 0;

   mat[8] = m.column2[0];
   mat[9] = m.column2[1];
   mat[10] = m.column2[2];
   mat[11] = 0;

   mat[12] = t[0];
   mat[13] = t[1];
   mat[14] = t[2];
   mat[15] = 1;
}

class PhysxSDK
{
private:
	PxPhysics* SDK;
	PxCooking* Cook;
	PxDefaultErrorCallback ErrorCallback;
	PxDefaultAllocator AllocatorCallback;
	PxSimulationFilterShader FilterShader;
	PxScene* gScene;
	PxReal Timestep;
	PxMaterial* dMaterial;
	PxFoundation* Foundation;
	PxControllerManager* manager;
public:
	PhysxSDK()
	{
		SDK=NULL;
		SDK = PxCreatePhysics(PX_PHYSICS_VERSION, AllocatorCallback, ErrorCallback, PxTolerancesScale() );
		PxInitExtensions(*SDK);
		Cook = PxCreateCooking(PX_PHYSICS_VERSION, &SDK->getFoundation(), PxCookingParams());
		FilterShader=PxDefaultSimulationFilterShader;
		manager = PxCreateControllerManager(SDK->getFoundation());
		Timestep=1.0f/60.0f;
	}
	void Release()
	{
		gScene->release();
		Cook->release();
		SDK->release();
	}
	void EnableVisualDebug()
	{
		PxExtensionVisualDebugger::connect(SDK->getPvdConnectionManager(),"localhost",5425, 10000, true); 
	}
	void MakeScene()
	{
		PxSceneDesc sceneDesc(SDK->getTolerancesScale());  
		sceneDesc.gravity=PxVec3(0.0f, -9.8f, 0.0f); 
		//sceneDesc.upAxis=2;
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);    
		sceneDesc.cpuDispatcher = mCpuDispatcher;    
		sceneDesc.filterShader  = FilterShader;     
		gScene = SDK->createScene(sceneDesc);
		dMaterial = SDK->createMaterial(0.5,0.5,0.5); 
	}
	void Simulate()
	{
		gScene->simulate(Timestep);
		manager->updateControllers();
	}
	bool FetchResults()
	{
		return gScene->fetchResults(true);
	}
	void LoadFile(const char* name, ObjectLibrary* lib)
	{
		physx::PxStringTable* mStringTable( &PxStringTableExt::createStringTable( AllocatorCallback ) );
		physx::repx::RepXCollection* collection=physx::repx::createCollection( name );
		addObjectsToScene( collection, SDK, Cook, gScene, mStringTable, lib );
		collection->destroy();
		mStringTable->release();
		mStringTable = NULL;
	}
	PxController* MakeCharacterController(XMFLOAT3 Position, float height, float radius, SkinnedModelObject* mesh)
	{
		PxCapsuleControllerDesc desc;
		desc.setToDefault();
		desc.position=PxExtendedVec3(Position.x, Position.y, Position.z);
		desc.height=height;
		desc.radius=radius;
		desc.userData=mesh;
		desc.material=dMaterial;
		//desc.slopeLimit=cosf(XMConvertToRadians(76));
		desc.slopeLimit=0;

		if (desc.isValid())
			return manager->createController(*SDK, gScene, desc);
		else
			return NULL;
	}
	void MakeTestBox(ModelObject* object, XMFLOAT3 Position)
	{  
		PxReal density = 1.0f; 
		PxTransform transform(PxVec3(Position.x, Position.y, Position.z), PxQuat::createIdentity()); 
		PxTransform trans;
		PxVec3 dimensions(10,10,10); 
		PxBoxGeometry geometry(dimensions);
		PxRigidDynamic *actor = PxCreateDynamic(*SDK, transform, geometry, *dMaterial, density);    
		actor->setAngularDamping(0.75);    
		actor->setLinearVelocity(PxVec3(1,0,0)); 
		actor->userData=object;
		gScene->addActor(*actor);
	}
	
	void MakeHeightFieldTMesh(SimpleVertex* samples, int* indices, HeightMap* the_map, int num_verts, int numIndices)
	{
		PxVec3* verts = new PxVec3[num_verts];
		for (int i=0; i<num_verts; ++i)
		{
			verts[i].x=samples[i].Pos.x; verts[i].y=samples[i].Pos.y; verts[i].z=samples[i].Pos.z;
		}

		PxTriangleMeshDesc meshDesc;
		meshDesc.points.count           = num_verts;
		meshDesc.triangles.count        = numIndices/3;
		meshDesc.points.stride          = sizeof(PxVec3);
		meshDesc.triangles.stride       = sizeof(int)*3;
		meshDesc.points.data            = verts;
		meshDesc.triangles.data         = indices;
		
		if (meshDesc.isValid())
		{
		PxToolkit::MemoryWriteBuffer buf;
		bool status = Cook->cookTriangleMesh(meshDesc, buf);
		PxTriangleMesh* triangleMesh = SDK->createTriangleMesh(PxToolkit::MemoryReadBuffer(buf.data));

		PxRigidStatic* aTriMeshActor = SDK->createRigidStatic(PxTransform(PxVec3(0,0,0), PxQuat(0, 1, 0, 0)));
		aTriMeshActor->userData=the_map;
		PxShape* aTriMeshShape = aTriMeshActor->createShape(PxTriangleMeshGeometry(triangleMesh), *dMaterial);
		gScene->addActor(*aTriMeshActor);
		}
	}

	void MakeHeightField(float samples[], HeightMap* the_map)
	{
		PxHeightFieldSample* samp = new PxHeightFieldSample[int(samples[0]*samples[1])];
		//samp->clearTessFlag();
		for (int i=0; i<(int)samples[0]*(int)samples[1]; ++i)
		{
			samp[i].height=PxI16(samples[3+i]*samples[3]/(65535.f / 2.f));
			samp[i].materialIndex0=1;
			samp[i].materialIndex1=1;
			//samp[i].setTessFlag();
		}

		PxHeightFieldDesc desc;
		desc.setToDefault();
		desc.format=PxHeightFieldFormat::eS16_TM;
		desc.nbColumns=(int)samples[1];
		desc.nbRows=(int)samples[0];
		desc.samples.data=samp;
		desc.samples.stride=sizeof(PxHeightFieldSample);
		//desc.thickness=-100;

		if (desc.isValid())
		{
			PxHeightField* aHeightField = SDK->createHeightField(desc);
			PxHeightFieldGeometry hfGeom(aHeightField, PxMeshGeometryFlags(), samples[3]/(65535.f / 2.f), 1, 1);
			PxRigidStatic* aHeightFieldActor = SDK->createRigidStatic(PxTransform(PxVec3(0,0,0), PxQuat(0, 1, 0, 0)));
			aHeightFieldActor->userData=the_map;
			PxShape* aHeightFieldShape = aHeightFieldActor->createShape(hfGeom, *dMaterial);
			if (!gScene->addActor(*aHeightFieldActor))
				return;
		}
	}

	void PxDraw_RigidDynamic(ID3D11DeviceContext* dcon, GBuffShader* shader)
	{
		PxActor** actors;
		int nbactors=gScene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC);
		actors= new PxActor*[nbactors];
		gScene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC, actors, nbactors);
		
		while (--nbactors>=0)
		{
			if (actors[nbactors]->userData)
			{
				ModelObject* object= (ModelObject*)actors[nbactors]->userData;
				PxRigidDynamic* shape=actors[nbactors]->isRigidDynamic();
				PxMat33 rotation(shape->getGlobalPose().q);
				float mat[16];
				getColumnMajor(rotation, shape->getGlobalPose().p, mat);
				
				shader->mats.worldMatrix = XMMatrixTranspose(XMMATRIX(mat));
				shader->CommitUniforms(dcon);
				object->Draw(dcon);
			}
		}
		delete []actors;
	}

	void PxDraw_RigidStatic(ID3D11DeviceContext* dcon, GBuffShader* shader)
	{
		PxActor** actors;
		int nbactors=gScene->getNbActors(PxActorTypeSelectionFlag::eRIGID_STATIC);
		actors= new PxActor*[nbactors];
		gScene->getActors(PxActorTypeSelectionFlag::eRIGID_STATIC, actors, nbactors);
		while (--nbactors>=0)
		{
			
			if (actors[nbactors]->userData)
			{
				ModelObject* object= (ModelObject*)actors[nbactors]->userData;
				PxRigidStatic* shape=actors[nbactors]->isRigidStatic();
				PxMat33 rotation(shape->getGlobalPose().q);
				float mat[16];
				getColumnMajor(rotation, shape->getGlobalPose().p, mat);
				
				shader->mats.worldMatrix = XMMatrixTranspose(XMMATRIX(mat));
				shader->CommitUniforms(dcon);
				object->Draw(dcon);
			}
		}
		delete []actors;
	}
	
	void PxDraw_CharacterControllers(ID3D11DeviceContext* dcon, SkinShader* shader)
	{
		for (int i=0; i<manager->getNbControllers(); ++i)
		{
			PxController* control=manager->getController(i);
			SkinnedModelObject* object= (SkinnedModelObject*)control->getUserData();
			XMMATRIX posMatrix=XMMatrixTranspose(XMMatrixTranslation(control->getPosition().x, control->getPosition().y, control->getPosition().z));
			shader->mats.worldMatrix = posMatrix;
			//shader->mats.skinmatrix=
			//object->RecalcMatrix(posMatrix);
			object->RecalcMatrix(posMatrix);
			//shader->mats.skinmatrix=object->LoadMatrices();
			object->LoadMatrices(&shader->mats.skinmatrix[0]);
			shader->CommitUniforms(dcon);
			object->Draw(dcon);
		}
	}

	void PxDraw(ID3D11DeviceContext* dcon, GBuffShader* shader, SkinShader* skin)
	{
		PxDraw_RigidDynamic(dcon, shader);
		PxDraw_RigidStatic(dcon, shader);
		PxDraw_CharacterControllers(dcon, skin);
	}
	PxVec3 GetGravity()
	{
		return gScene->getGravity();
	}
};

class CallBack : public PxSimulationEventCallback
{
private:
public:
};