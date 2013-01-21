
class CharacterController
{
private:
	SkinnedModelObject* body;
	PxController* controller;

	PxVec3 DispX;
	PxVec3 DispY;
	PxVec3 DispZ;
	PxVec3 Gravity;

	float screenWidth;
	float screenHeight;
public:
	CharacterController()
	{ }
	~CharacterController()
	{
		controller->release();
		delete[] body;
	}
	void init(PhysxSDK* PxSDK, SkinnedModelObject* its_models, int width, int height)
	{
		screenWidth=width;
		screenHeight=height;
		body=its_models;
		Gravity=PxSDK->GetGravity();
		controller=PxSDK->MakeCharacterController(XMFLOAT3(0, 500, 0), 10, 3, body);
	}
	void ClearDisp()
	{
		DispX=PxVec3(0, 0, 0);
		DispY=PxVec3(0, 0, 0);
		DispZ=PxVec3(0, 0, 0);
	}
	void OnW()
	{
		DispZ.x=0;
		DispZ.y=0;
		DispZ.z=0.1;
	}
	void OnS()
	{
		DispZ.x=0;
		DispZ.y=0;
		DispZ.z=-0.1;
	}
	void OnA()
	{
		DispZ.x=-0.1;
		DispZ.y=0;
		DispZ.z=0;
	}
	void OnD()
	{
		DispZ.x=0.1;
		DispZ.y=0;
		DispZ.z=0;
	}
	void OnSPC()
	{
		DispY.x=0;
		DispY.y=10.0f;
		DispY.z=0;
	}
	void UpdatePos()
	{
		PxU32 flags;
		controller->move(DispX+DispY+DispZ+Gravity, NULL, 0.01f/10.0f, flags, 0.2f, NULL, NULL);
	}
	CameraObject GetCamera()
	{
		CameraObject out;
		out.init(XMFLOAT3(controller->getPosition().x, controller->getPosition().y+50, controller->getPosition().z-50), XMFLOAT3(controller->getPosition().x, controller->getPosition().y, controller->getPosition().z), XMFLOAT3(0, 1, 0), .1, 1000, screenWidth, screenHeight);
		return out;
	}
	//PxController::move(const PxVec3& disp, PxU32 activeGroups, PxF32 minDist, PxU32& collisionFlags, PxF32 sharpness=1.0f, const PxFilterData* filterData=NULL, PxSceneQueryFilterCallback* filterCallback = NULL);
};