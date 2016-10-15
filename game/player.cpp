#include "stdafx.h"

#include "player.h"
#include "CPR_Framework.h"
#include "game/world.h"
#include "debugutils/debugrenderer.h"

// TODO: find a better place for these constants, or some kind of definition scheme
namespace
{
	static /*const*/ float PLAYER_SPEED = 3.0f;
	static /*const*/ float PLAYER_HEIGHT = 1.8f;
	static /*const*/ float MOUSE_SENSITIVY = TO_RADIANS(0.2f); // 0.2 degrees per-pixel
	static /*const*/ float PLAYER_RADIUS = 0.5f;
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
void cPlayer::Update(float elapsed)
{
	mPos += ComputeLinearVelocity() * elapsed;

	const cVector3 height(0.0f, PLAYER_HEIGHT, 0.0f);

	// Test
	const cVector3 forward_dir = cVector3::ZAXIS().RotateAroundY(mYaw);
	// Debug::cRenderer::Get().AddSphere(mPos + height + (forward_dir * 2.0f), PLAYER_RADIUS, TCOLOR_YELLOW);

	// Debug::WriteLine("Player pos: (%f, %f, %f). Forward dir: (%f, %f, %f)", mPos.x, mPos.y, mPos.z, forward_dir.x, forward_dir.y, forward_dir.z);

	const cVector3 collision = cWorld::GetInstance()->StepPlayerCollision(mPos + cVector3(0.0f, PLAYER_HEIGHT, 0.0f), forward_dir * 10000, PLAYER_RADIUS, elapsed);
	Debug::cRenderer::Get().AddSphere(collision, 1.0f, TCOLOR_RED);
	Debug::WriteLine("collision: (%f, %f, %f). ", collision.x, collision.y, collision.z);

	mLookAt = ComputeLookAt();

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

	return mPos + cVector3(0.0f, mPos.y + sin(mPitch) + PLAYER_HEIGHT, cos(mPitch)).RotateAroundY(mYaw);
}