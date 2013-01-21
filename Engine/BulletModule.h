#include <btBulletDynamicsCommon.h>
#include <btBulletWorldImporter.h>
#pragma comment(lib, "BulletCollision.lib")
#pragma comment(lib, "BulletDynamics.lib")
#pragma comment(lib, "LinearMath.lib")
#pragma comment(lib, "BulletFileLoader.lib")
#pragma comment(lib, "BulletWorldImporter.lib")

class DebugDrawer : public btIDebugDraw
{
private:
	ID3D11Buffer* VertexBuffer;
	ID3D11Device* device;
	ID3D11DeviceContext* dcon;
	GBuffDebugShader shader;
public:
	struct DebugVertex
	{
		XMFLOAT3 pos;
		XMFLOAT4 Color;
	};
	DebugDrawer(ID3D11Device* Device, ID3D11DeviceContext* context) : btIDebugDraw()
	{
		dcon=context;
		device=Device;
		shader.init(device, L"GBufferDebug.fx");
	}

	void setmatrix(XMFLOAT4X4* camera, XMFLOAT4X4* projection)
	{
		shader.mats.projectionMatrix=XMLoadFloat4x4(projection);
		shader.mats.viewMatrix=XMLoadFloat4x4(camera);
	}

	void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		//dcon->CopySubresourceRegion(VertexBuffer, 0, 0, 0, 0, 
		DebugVertex vertices[2]=
		{
			{XMFLOAT3(from.x(), from.y(), from.z()), XMFLOAT4(color.x(), color.y(), color.z(), 1.0f)},
			{XMFLOAT3(to.x(), to.y(), to.z()), XMFLOAT4(color.x(), color.y(), color.z(), 1.0f)}
		};
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( DebugVertex ) * 2;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData; 
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		device->CreateBuffer( &bd, &InitData, &VertexBuffer );

		shader.mats.worldMatrix=XMMatrixTranspose(XMMatrixIdentity());
		shader.CommitUniforms(dcon);

		UINT stride = sizeof( DebugVertex );
		UINT offset = 0;
		dcon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		dcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		dcon->Draw(ARRAYSIZE(vertices), 0);
	}

	void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
	}

	void	reportErrorWarning(const char* warningString)
	{
	}

	void	draw3dText(const btVector3& location,const char* textString)
	{
	}

	void	setDebugMode(int debugMode)
	{
	}
	
	int		getDebugMode() const
	{
		return 1;
	}
};

class MyMotionState : public btMotionState {
private:
    ModelObject* object;
	GBuffShader* shader;
	ID3D11DeviceContext* dcon;
    btTransform mPos1;
public:
    MyMotionState(const btTransform &initialpos, ModelObject* obj, GBuffShader* shade, ID3D11DeviceContext* decon) : shader(shade), dcon(decon), mPos1(initialpos), object(obj)
	{
	  
    }

    virtual ~MyMotionState() 
	{
    }

    virtual void getWorldTransform(btTransform &worldTrans) const 
	{
        worldTrans = mPos1;
    }

    virtual void setWorldTransform(const btTransform &worldTrans) 
	{
		if (object!=NULL)
		{
			//btVector3 pos;
			float transform[16];
			worldTrans.getOpenGLMatrix(transform);
			//pos=worldTrans.getOrigin();
			shader->mats.worldMatrix=XMMatrixTranspose(XMMATRIX(transform));
			shader->CommitUniforms(dcon);
			object->Draw(dcon);
		}
    }
};

class WorldImporter : public btBulletWorldImporter
{
private:
	MyMotionState* motionstate;
	int numberObjects;
	ModelObject** objects;
	GBuffShader* gbuf;
	ID3D11DeviceContext* dcon;
public:
	
	WorldImporter(btDiscreteDynamicsWorld* World, ObjectLibrary* library, GBuffShader* shade, ID3D11DeviceContext* decon) : btBulletWorldImporter(World), gbuf(shade), dcon(decon)
	{
		objects = new ModelObject*(library->objects);
		numberObjects=library->numberObjects;
	}

	btRigidBody* createRigidBody(bool isDynamic, btScalar mass, const btTransform& startTransform,btCollisionShape* shape,const char* bodyName) //get motion state into here
	{
		btVector3 localInertia;
		localInertia.setZero();

		if (mass)
			shape->calculateLocalInertia(mass,localInertia);

		ModelObject* thisone;
	for (int i=0; i<numberObjects; ++i)
	 {
		// ModelObject* object= new ModelObject(*objects[numberObjects]);
		 string name=string(objects[i]->name+"_bullet");
		 name="pCube_bulletShape";
		 if (strncmp(name.c_str(), bodyName, 20)==0)
		   {
			   thisone=objects[i];
			   break;
		   }
	  }

		MyMotionState* motionstate=new MyMotionState(startTransform, thisone, gbuf, dcon);
		btRigidBody* body = new btRigidBody(mass,motionstate,shape,localInertia);	//put motion state here
		//body->setWorldTransform(startTransform);

		if (m_dynamicsWorld)
		{
			body->setActivationState(DISABLE_DEACTIVATION);
			m_dynamicsWorld->addRigidBody(body);
			body->setActivationState(DISABLE_DEACTIVATION);
		}
	
		if (bodyName)
		{
			char* newname = duplicateName(bodyName);
			m_objectNameMap.insert(body,newname);
			m_nameBodyMap.insert(newname,body);
		}
		m_allocatedRigidBodies.push_back(body);
		return body;

	}

};


class BulletSDK
{
private:
	 btBroadphaseInterface* broadphase;
	 btDefaultCollisionConfiguration* collisionConfiguration;
     btCollisionDispatcher* dispatcher;
	 btSequentialImpulseConstraintSolver* solver;
	 btDiscreteDynamicsWorld* dynamicsWorld;
	 DebugDrawer* debugger;
public:
	BulletSDK()
	{
		broadphase = new btDbvtBroadphase();
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		solver = new btSequentialImpulseConstraintSolver;
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0,-10,0));
		
	}
	~BulletSDK()
	{
		delete broadphase;
		delete collisionConfiguration;
		delete dispatcher;
		delete solver;
		delete dynamicsWorld;
	}

	void EnableDebugDraw(ID3D11Device* device, ID3D11DeviceContext* dcon)
	{
		debugger = new DebugDrawer(device, dcon);
		dynamicsWorld->setDebugDrawer(debugger);
		dynamicsWorld->getDebugDrawer()->setDebugMode(1);
	}

	void Simulate(static XMFLOAT4X4* camera, XMFLOAT4X4* projection)
	{
		dynamicsWorld->stepSimulation(1.0f/60.0f);
		debugger->setmatrix(camera, projection);
		dynamicsWorld->debugDrawWorld();
	}
	
	void LoadFile(const char* file, ObjectLibrary* lib, GBuffShader* shader, ID3D11DeviceContext* dcon)
	{
		WorldImporter* fileLoader = new WorldImporter(dynamicsWorld, lib, shader, dcon);
		
		fileLoader->loadFile(file);
	}

	void MakeTestBox(GBuffShader* shader, ID3D11DeviceContext* dcon, ModelObject* object)
	{
		btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
		btCollisionShape* fallShape = new btSphereShape(1);
		MyMotionState* groundMotionState = new MyMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)), object, shader, dcon);
		btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
		groundRigidBody->setActivationState(DISABLE_DEACTIVATION);
		dynamicsWorld->addRigidBody(groundRigidBody);
		MyMotionState* fallMotionState = new MyMotionState(btTransform(btQuaternion(0.43f,0,0,1),btVector3(0,500,0)), object, shader, dcon);
		btScalar mass = 1;
        btVector3 fallInertia(0,0,0);
        fallShape->calculateLocalInertia(mass,fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,fallShape,fallInertia);
        btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
		fallRigidBody->setActivationState(DISABLE_DEACTIVATION);
        dynamicsWorld->addRigidBody(fallRigidBody);
		//fallRigidBody->setActivationState(DISABLE_DEACTIVATION);
	}
};