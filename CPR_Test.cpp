#include "stdafx.h"

#include "CPR_Framework.h"
#include "game/world.h"
#include "game/player.h"
#include "game/bullet.h"
#include "debugutils/debugrenderer.h"


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
	cWorld::InitInstance("resources/city.txt");

	// Register our game object classes
	cGameObjectManager::InitInstance();
	cBullet::RegisterInManager();
	cPlayer::RegisterInManager();

	cGameObjectManager::GetInstance()->CreateGameObject<cPlayer>(sDefaultPlayerDef, cPlayerState(cVector3(2.0f, sDefaultPlayerDef.mRadius, -12.0f)));
}

//----------------------------------------------------------------------------
void OnShutdown()
{
	cGameObjectManager::GetInstance()->DestroyAllGameObjects();
}

//----------------------------------------------------------------------------
void OnUpdate( float _deltaTime )
{
	// TODO: Some update times are coming with 0, investigate what this means to the actual framerate
	if (_deltaTime > 0.0f)
	{
		// The debug renderer update will clear debug entries, so make sure it happens before we can add any this frame
		Debug::cRenderer::Get().Update(_deltaTime);
	}

	cGameObjectManager::GetInstance()->Update(_deltaTime);
}

//----------------------------------------------------------------------------
void OnRender()
{
	cWorld::GetInstance()->Render();
	cGameObjectManager::GetInstance()->Render();

	Debug::cRenderer::Get().Render();
}