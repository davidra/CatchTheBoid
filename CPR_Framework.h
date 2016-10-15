#pragma once

#include <d3dx9.h>
#include <vector>

//----------------------------------------------------------------------------
//	Simple class for handling mouse input
//----------------------------------------------------------------------------
class Mouse
{
public:
	static D3DXVECTOR2 GetPosition();
	static bool LeftMouseButton();
	static bool RightMouseButton();
	// TODO: Delete this if not used
/*
	static D3DXVECTOR2 GetMouseDelta()
	{
		// If the resolution changes after this is called for the first time, this will no longer be correct, but oh well
		static const int reset_x = GetSystemMetrics(SM_CXSCREEN) / 2;
		static const int reset_y = GetSystemMetrics(SM_CYSCREEN) / 2;

		auto mouse_delta = GetPosition() - D3DXVECTOR2(static_cast<float>(reset_x), static_cast<float>(reset_y));

		SetCursorPos(reset_x, reset_y);

		return mouse_delta;
	}
*/
};

//----------------------------------------------------------------------------
//	Simple class for handling keyboard input
//----------------------------------------------------------------------------
class Keyboard
{
public:
	enum Key
	{
		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_SPACE,
		KEY_RETURN,
		KEY_W,
		KEY_A,
		KEY_S,
		KEY_D
	};

	static bool IsKeyPressed( Key key );
};

//----------------------------------------------------------------------------
//	A class for controlling the camera
//----------------------------------------------------------------------------
class Camera
{
public:
	static void LookAt( const D3DXVECTOR3& _eye, const D3DXVECTOR3& _target );
};

//----------------------------------------------------------------------------
//	Mesh resource handling class
//----------------------------------------------------------------------------
class Mesh
{
protected:
	Mesh();

public:
	~Mesh();

	static Mesh* LoadFromFile( char filename[] );

	void Render( const D3DXVECTOR3& _position, const D3DXVECTOR3& _rotation, const D3DXVECTOR3& _scale, D3DXVECTOR4 _color );

private:
	ID3DXMesh*			m_mesh;
	int					m_numSubsets;
};