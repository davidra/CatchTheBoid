#include "stdafx.h"

#include "player.h"
#include "CPR_Framework.h"
#include "game/world.h"
#include "game/bullet.h"
#include "debugutils/debugrenderer.h"


static const cPlayerDef sDefaultPlayerDef(0.5f, 5.0f, 0.5f * 2.0f, TO_RADIANS(0.2f), 0.8f);

namespace
{
	static bool sTestCollisionsOnLookAt = true;
	static bool sMoveUpAndDown = true;
	static bool sOverrideSpawnPos = true;
	static bool sUseWorldCentroid = false;
}

//----------------------------------------------------------------------------
cPlayer::cPlayer()
	: mYaw(TO_RADIANS(-90.0f))
	, mPitch(0.0f)
	, mLookAt(cVector3::ZERO())
	, mPrevMousePos(0.0f, 0.0f)
	, mLastShot(0.0f)
	, mCrosshair(nullptr)
{
}

//----------------------------------------------------------------------------
bool cPlayer::Init(const IGameObjectDef* def, IGameObjectState*&& initial_state)
{
	bool success = IGameObject::Init(def, std::move(initial_state));
	if (success)
	{
		const cWorld* const world = cWorld::GetInstance();
		CPR_assert(world != nullptr, "World has not been initialized yet!");

		if (sUseWorldCentroid)
		{
			const cAABB& world_boundaries = cWorld::GetInstance()->GetWorldBoundaries();
			State().mPos = world_boundaries.GetCentroid();
			State().mPos.y = Def().mRadius;
		}
	}

	// mCrosshair = ModelRepo::GetModel(MID_BOX);
	mCrosshair = ModelRepo::GetModel(MID_SPHERE);

	return success;
}

//----------------------------------------------------------------------------
void cPlayer::Update(float elapsed)
{
	// Debug::WriteLine("Player pos (%f, %f, %f)", State().mPos.x, State().mPos.y, State().mPos.z);

	State().mPos = cWorld::GetInstance()->StepPlayerCollision(State().mPos, ComputeLinearVelocity(), Def().mRadius, elapsed);

	mLookAt = ComputeLookAt();
	const cVector3 eye_pos = ComputeEyePos();

	if (sTestCollisionsOnLookAt && Keyboard::IsKeyPressed(Keyboard::KEY_UP))
	{
		const cVector3 aim_dir = Normalize(mLookAt - eye_pos);
		cVector3 coll_pos;
		cVector3 coll_normal;
		if (cWorld::GetInstance()->CastSphereAgainstWorld(eye_pos, eye_pos + (aim_dir * 10000), Def().mRadius, false, coll_pos, coll_normal))
		{

			Debug::cRenderer::Get().AddSphere(coll_pos + (coll_normal * Def().mRadius), Def().mRadius, TCOLOR_RED);
			Debug::WriteLine("Collision: (%f, %f, %f). Normal: (%f, %f, %f)", coll_pos.x, coll_pos.y, coll_pos.z, coll_normal.x, coll_normal.y, coll_normal.z);
		}
	}

	if (Mouse::LeftMouseButton())
	{
		cGameObjectManager* const game_obj_mgr = cGameObjectManager::GetInstance();
		const float current_time = game_obj_mgr->GetCurrTime();

		if ((current_time - mLastShot) > Def().mFirePeriod)
		{
			const cVector3 eye_pos = ComputeEyePos();
			const cVector3 aim_dir = Normalize(mLookAt - eye_pos);

			game_obj_mgr->CreateGameObject<cBullet>(gPlayerBullets, cBulletState(eye_pos, aim_dir));

			mLastShot = current_time;
		}
	}

	const cVector3 height(0.0f, Def().mHeight, 0.0f);
	Camera::LookAt(State().mPos + height, mLookAt);
}

//----------------------------------------------------------------------------
void cPlayer::Render()
{
	// WIP Render "crosshair" . I Have no idea why I can't get this to work...
/*
	static float separation = 0.01f;
	static float distance_to_crosshair_plane = 0.4f;

	cVector3 crosshair_up = cVector3(0.0f, separation, distance_to_crosshair_plane);
	cVector3 crosshair_down = cVector3(0.0f, -separation, distance_to_crosshair_plane);
	cVector3 crosshair_left = cVector3(-separation, 0.0f, distance_to_crosshair_plane);
	cVector3 crosshair_right = cVector3(separation, 0.0f, distance_to_crosshair_plane);

	static float long_side = 0.01f;
	static float short_side = 0.005f;

/ *
	const cVector3 horizontal_boxes_scale = cVector3(long_side, short_side, short_side);
	const cVector3 vertical_boxes_scale = cVector3(short_side, long_side, short_side);
* /
	const cVector3 horizontal_boxes_scale = cVector3::ONE() * long_side;
	const cVector3 vertical_boxes_scale = cVector3::ONE() * long_side;

	const cVector3 rotation(-mPitch, mYaw, 0.0f);

	const cVector3 eye_pos = ComputeEyePos();
	cMatrix44 look_at_matrix = BuildLookAtMatrix(eye_pos, mLookAt, cVector3::YAXIS());
	look_at_matrix.SetTranslation(eye_pos);

	const cVector3 crosshair_up_pos = look_at_matrix.RotateCoord(crosshair_up);
	const cVector3 crosshair_up_down = look_at_matrix.RotateCoord(crosshair_down);
	const cVector3 crosshair_up_left = look_at_matrix.RotateCoord(crosshair_left);
	const cVector3 crosshair_up_right = look_at_matrix.RotateCoord(crosshair_right);

	mCrosshair->Render(crosshair_up_pos, rotation, vertical_boxes_scale, TCOLOR_BLACK);
	mCrosshair->Render(crosshair_up_down, rotation, vertical_boxes_scale, TCOLOR_BLACK);
	mCrosshair->Render(crosshair_up_left, rotation, horizontal_boxes_scale, TCOLOR_BLACK);
	mCrosshair->Render(crosshair_up_right, rotation, horizontal_boxes_scale, TCOLOR_BLACK);
*/
}

//----------------------------------------------------------------------------
cVector3 cPlayer::ComputeEyePos() const
{
	return (State().mPos + cVector3(0.0f, Def().mHeight, 0.0f));
}

//----------------------------------------------------------------------------
cVector3 cPlayer::ComputeLinearVelocity() const
{
	// For now, let's have infinite acceleration to max speed, and infinite friction

	cVector3 linear_velocity(cVector3::ZERO());
	if (Keyboard::IsKeyPressed(Keyboard::KEY_A))
	{
		linear_velocity.x -= 1.0f;
	}

	if (Keyboard::IsKeyPressed(Keyboard::KEY_W))
	{
		linear_velocity.z += 1.0f;
	}

	if (Keyboard::IsKeyPressed(Keyboard::KEY_S))
	{
		linear_velocity.z -= 1.0f;
	}

	if (Keyboard::IsKeyPressed(Keyboard::KEY_D))
	{
		linear_velocity.x += 1.0f;
	}

	if (sMoveUpAndDown)
	{
		// For debugging purposes mainly
		if (Keyboard::IsKeyPressed(Keyboard::KEY_SPACE))
		{
			linear_velocity.y += 1.0f;
		}
		if (Keyboard::IsKeyPressed(Keyboard::KEY_DOWN))
		{
			linear_velocity.y -= 1.0f;
		}
	}

	linear_velocity.SetNormalized();
	linear_velocity *= Def().mSpeed;

	return linear_velocity.RotateAroundY(mYaw);
}

//----------------------------------------------------------------------------
cVector3 cPlayer::ComputeLookAt()
{
	auto new_mouse_pos = Mouse::GetPosition();
	auto mouse_delta = new_mouse_pos - mPrevMousePos;
	mPrevMousePos = new_mouse_pos;
	mouse_delta *= Def().mMouseSensitivity;

	mYaw += mouse_delta.x;
	mPitch -= mouse_delta.y;

	static const float MAX_PITCH = TO_RADIANS(85.0f);
	mPitch = Clamp(-MAX_PITCH, mPitch, MAX_PITCH);

	const float lookat_y = sin(mPitch) * Def().mRadius * 0.9f;
	const float lookat_z = cos(mPitch) * Def().mRadius * 0.9f;

	return State().mPos + cVector3(0.0f, Def().mHeight + lookat_y, lookat_z).RotateAroundY(mYaw);
}

//----------------------------------------------------------------------------
cVector3 cPlayer::GetForwardDir() const
{
	return cVector3::ZAXIS().RotateAroundY(mYaw);
}