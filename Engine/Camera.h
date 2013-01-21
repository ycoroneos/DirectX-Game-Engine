class CameraObject
{
private:
	XMVECTOR Position;
	XMVECTOR At;
	XMVECTOR Up;
	XMMATRIX View;

	XMMATRIX Projection;

	PhysxSDK* SDK;
	ModelObject* box;
public:
	void init(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up, float near_view, float far_view, float screenwidth, float screenheight)
	{
		Position=XMVectorSet(eye.x, eye.y, eye.z, 1);
		At=XMVectorSet(at.x, at.y, at.z, 1);
		Up=XMVectorSet(up.z, up.y, up.z, 1);
		View = XMMatrixLookAtLH( Position, At, Up );

		Projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, screenwidth / screenheight, near_view, far_view );
	}
	void SetBox_Physx(ObjectLibrary* in, PhysxSDK* PxSDK)
	{
		box=in->objects[0];
		SDK=PxSDK;
	}
	void Update(XMFLOAT4X4 Transform)
	{
		View=XMMatrixMultiply(View, XMLoadFloat4x4(&Transform));
	}
	void ShootBox()
	{
		
		SDK->MakeTestBox(box, this->GetPosition());
	}
	XMFLOAT3 GetPosition()
	{
		XMMATRIX inv=XMMatrixInverse(&XMMatrixDeterminant(View), View);
		XMFLOAT3 out(inv._41, inv._42, inv._43);
		return out;
	}
	XMFLOAT3 GetViewDirection()
	{
		XMFLOAT3 out; 
		XMStoreFloat3(&out, At);
		return out;
	}
	XMFLOAT3 GetUp()
	{
		XMFLOAT3 out; 
		XMStoreFloat3(&out, Up);
		return out;
	}
	XMFLOAT4X4 GetView()
	{
		XMFLOAT4X4 ViewX;
		XMStoreFloat4x4(&ViewX, View); 
		return ViewX;
	}
	XMFLOAT4X4 GetProjection()
	{
		XMFLOAT4X4 ProjectionX;
		XMStoreFloat4x4(&ProjectionX, Projection);
		return ProjectionX;
	}
};