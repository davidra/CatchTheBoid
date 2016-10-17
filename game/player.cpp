#include "stdafx.h"

#include "player.h"
#include "CPR_Framework.h"
#include "game/world.h"
#include "debugutils/debugrenderer.h"

// TODO: find a better place for these constants, or some kind of definition scheme
namespace
{
	static /*const*/ float PLAYER_RADIUS = 0.5f;
	static /*const*/ float PLAYER_SPEED = 3.0f;
	static /*const*/ float PLAYER_HEIGHT = PLAYER_RADIUS * 2.0f;
	static /*const*/ float MOUSE_SENSITIVY = TO_RADIANS(0.2f); // 0.2 degrees per-pixel
}

//----------------------------------------------------------------------------
cPlayer::cPlayer()
	: mYaw(TO_RADIANS(-90.0f))
	, mPitch(0.0f)
	, mPos(cVector3::ZERO())
	, mLookAt(cVector3::ZERO())
	, mPrevMousePos(0.0f, 0.0f)
{
}

//----------------------------------------------------------------------------
void cPlayer::Init()
{
	const cWorld* const world = cWorld::GetInstance();
	CPR_assert(world != nullptr, "World has not been initialized yet!");

	const cAABB& world_boundaries = cWorld::GetInstance()->GetWorldBoundaries();
	mPos = world_boundaries.GetCentroid();
	mPos.y = 0.0f;

	// Test: Delete me
	mPos = cVector3(2.0f, 0.0f, -12.0f);
}

//----------------------------------------------------------------------------
void cPlayer::Update(float elapsed)
{
	// Debug::WriteLine("Player pos (%f, %f, %f)", mPos.x, mPos.y, mPos.z);

	mPos = cWorld::GetInstance()->StepPlayerCollision(mPos, ComputeLinearVelocity(), PLAYER_RADIUS, elapsed);

	mLookAt = ComputeLookAt();

	const cVector3 forward_dir = GetForwardDir();
	
	if (Keyboard::IsKeyPressed(Keyboard::KEY_UP))
	{
		const cVector3 eye_pos = mPos + cVector3(0.0f, PLAYER_HEIGHT, 0.0f);
		const cVector3 aim_dir = Normalize(mLookAt - eye_pos);
		cVector3 coll_pos;
		cVector3 coll_normal;
		if (cWorld::GetInstance()->CastSphereAgainstWorld(eye_pos, eye_pos + (aim_dir * 10000), PLAYER_RADIUS, coll_pos, coll_normal))
		{
			Debug::cRenderer::Get().AddSphere(coll_pos, 1.0f, TCOLOR_RED);
			Debug::WriteLine("Collision: (%f, %f, %f). Normal: (%f, %f, %f)", coll_pos.x, coll_pos.y, coll_pos.z, coll_normal.x, coll_normal.y, coll_normal.z);
		}
	}

	const cVector3 height(0.0f, PLAYER_HEIGHT, 0.0f);
	Camera::LookAt(mPos + height, mLookAt);
}

//----------------------------------------------------------------------------
void cPlayer::Render()
{
}

//----------------------------------------------------------------------------
cVector3 cPlayer::ComputeLinearVelocity() const
{
	cVector3 player_speed(cVector3::ZERO());
	if (Keyboard::IsKeyPressed(Keyboard::KEY_A))
	{
		player_speed.x -= 1.0f;
	}

	if (Keyboard::IsKeyPressed(Keyboard::KEY_W))
	{
		player_speed.z += 1.0f;
	}

	if (Keyboard::IsKeyPressed(Keyboard::KEY_S))
	{
		player_speed.z -= 1.0f;
	}

	if (Keyboard::IsKeyPressed(Keyboard::KEY_D))
	{
		player_speed.x += 1.0f;
	}

	player_speed.SetNormalized();
	player_speed *= PLAYER_SPEED;

	return player_speed.RotateAroundY(mYaw);
}

//----------------------------------------------------------------------------
cVector3 cPlayer::ComputeLookAt()
{
	auto new_mouse_pos = Mouse::GetPosition();
	auto mouse_delta = new_mouse_pos - mPrevMousePos;
	mPrevMousePos = new_mouse_pos;
	mouse_delta *= MOUSE_SENSITIVY;

	mYaw += mouse_delta.x;
	mPitch -= mouse_delta.y;

	static const float MAX_PITCH = TO_RADIANS(85.0f);
	mPitch = Clamp(-MAX_PITCH, mPitch, MAX_PITCH);

	const float lookat_y = sin(mPitch) * PLAYER_RADIUS * 0.9f;
	const float lookat_z = cos(mPitch) * PLAYER_RADIUS * 0.9f;

	return mPos + cVector3(0.0f, mPos.y + PLAYER_HEIGHT + lookat_y, lookat_z).RotateAroundY(mYaw);
}

//----------------------------------------------------------------------------
cVector3 cPlayer::GetForwardDir() const
{
	return cVector3::ZAXIS().RotateAroundY(mYaw);
}