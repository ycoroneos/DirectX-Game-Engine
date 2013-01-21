#include <dinput.h>
#include <time.h>

class DXINPUT
{
private:
	LPDIRECTINPUT8 din;    // the pointer to our DirectInput interface
	LPDIRECTINPUTDEVICE8 dinkeyboard;    // the pointer to the keyboard device
	LPDIRECTINPUTDEVICE8 dinmouse;    // the pointer to the mouse device
	DIMOUSESTATE mousestate;    // the storage for the mouse-information
	BYTE keystate[256];    // the storage for the key-information

	double old_time;
public:
	~DXINPUT()
	{
    dinkeyboard->Unacquire();    // make sure the keyboard is unacquired
	dinmouse->Unacquire();
    din->Release();    // close DirectInput before exiting
	}

	bool KeyState(int key)
	{
		if(keystate[key] & 0x80)
			return true;
		else
			return false;
	}

void initDInput(HINSTANCE hInstance, HWND hWnd)
{
    // create the DirectInput interface
    DirectInput8Create(hInstance,    // the handle to the application
                       DIRECTINPUT_VERSION,    // the compatible version
                       IID_IDirectInput8,    // the DirectInput interface version
                       (void**)&din,    // the pointer to the interface
                       NULL);    // COM stuff, so we'll set it to NULL

    // create the keyboard device
    din->CreateDevice(GUID_SysKeyboard,    // the default keyboard ID being used
                      &dinkeyboard,    // the pointer to the device interface
                      NULL);    // COM stuff, so we'll set it to NULL

	 din->CreateDevice(GUID_SysMouse,
                      &dinmouse,
                      NULL);

    // set the data format to keyboard format
    dinkeyboard->SetDataFormat(&c_dfDIKeyboard);
	dinmouse->SetDataFormat(&c_dfDIMouse);

    // set the control you will have over the keyboard
    dinkeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dinmouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	old_time=0;

}

	void UpdateFreeCamera(CameraObject* Camera)
{
    // get access if we don't have it already
    dinkeyboard->Acquire();
	dinmouse->Acquire();

    // get the input data
    dinkeyboard->GetDeviceState(256, (LPVOID)keystate);
	dinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mousestate);

	XMFLOAT3 pos=Camera->GetPosition();
	
	XMMATRIX Translation, Rotation;
	Translation=XMMatrixIdentity();
	Rotation=XMMatrixRotationRollPitchYaw(mousestate.lY*.01*-1, mousestate.lX*.01*-1, 0.0f);

	float speed=0;

	if(keystate[DIK_LSHIFT] & 0x80)
	{
		speed=0.5;
	}

	if(keystate[DIK_W] & 0x80)
	{
		Translation=XMMatrixMultiply(Translation, XMMatrixTranslation(0.0f, 0.0f, -0.2f-speed));
	}
	if(keystate[DIK_S] & 0x80)
	{
		Translation=XMMatrixMultiply(Translation, XMMatrixTranslation(0.0f, 0.0f, 0.2f+speed));
	}
	if(keystate[DIK_A] & 0x80)
	{
		Translation=XMMatrixMultiply(Translation, XMMatrixTranslation(0.2f+speed, 0.0f, 0.0f));
	}
	if(keystate[DIK_D] & 0x80)
	{
		Translation=XMMatrixMultiply(Translation, XMMatrixTranslation(-0.2f-speed, 0.0f, 0.0f));
	}
	
	XMFLOAT4X4 Transform;
	XMStoreFloat4x4(&Transform, XMMatrixMultiply(Translation, Rotation));
	Camera->Update(Transform);
	static bool pressed=false;
	if(keystate[DIK_K] & 0x80 && pressed!=true)
	{
		Camera->ShootBox();
		pressed=true;
	}
	else
	{
		pressed=false;
	}
}
	void UpdateController(CharacterController* controller)
	{
		// get access if we don't have it already
    dinkeyboard->Acquire();
	dinmouse->Acquire();

    // get the input data
    dinkeyboard->GetDeviceState(256, (LPVOID)keystate);
	dinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mousestate);
	
	XMMATRIX Translation, Rotation;
	Translation=XMMatrixIdentity();
	Rotation=XMMatrixRotationRollPitchYaw(mousestate.lY*.01*-1, mousestate.lX*.01*-1, 0.0f);

	if (clock()-old_time>=1/100)
		{
		old_time=clock();
		controller->ClearDisp();
		if(KeyState(DIK_W))
		{
			controller->OnW();
		}
		if(KeyState(DIK_S))
		{
			controller->OnS();
		}

		if(KeyState(DIK_A))
		{
			controller->OnA();
		}
		if(KeyState(DIK_D))
		{
			controller->OnD();
		}
		if(KeyState(DIK_SPACE))
		{
			controller->OnSPC();
		}
	
		controller->UpdatePos();
		}
	}

};