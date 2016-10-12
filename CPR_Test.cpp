#include "stdafx.h"

#include "CPR_Framework.h"
#include "game/world.h"

cWorld gWorld;
float	g_angle = 0.0f;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	This file contains the main loop of the program and works as follows:

		OnInit();

		while( !shouldShutDown )
		{
			OnUpdate( deltaTime );
			OnRender();
		}

		OnShutdown();

	For vector & matrix math we're using the D3DX Library. 
	Here are some useful classes & functions (that may or may not be handy):

		D3DXVECTOR3		- x, y, z (floats)
		D3DXMATRIX		- 16 float values
		D3DXQUATERNION	- x, y, z, w (floats)

		D3DXVECTOR3*	D3DXVec3Normalize( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV );
		FLOAT			D3DXVec3Length( const D3DXVECTOR3 *pV );
		FLOAT			D3DXVec3Dot( const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );
		D3DXVECTOR3*	D3DXVec3Cross( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

	You can find these and more in the DX SDK documentation.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//----------------------------------------------------------------------------
void OnInit()
{
	gWorld.Init("resources/city.txt");
}

//----------------------------------------------------------------------------
void OnShutdown()
{
}

//----------------------------------------------------------------------------
void OnUpdate( float _deltaTime )
{
	// update camera
	g_angle += _deltaTime;
	Camera::LookAt( D3DXVECTOR3( cos( g_angle ) * 50.0f, 5.0f, sin( g_angle ) * 50.0f ), D3DXVECTOR3( 0.0f, 0.5f, 0.0f ) );
}

//----------------------------------------------------------------------------
void OnRender()
{
	gWorld.Render();
}