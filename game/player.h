/***************************************************************************************************
player.h
 
Player class

by David Ramos
***************************************************************************************************/
#pragma once

#include "gameobject.h"
#include "GameObjectManager.h"

//----------------------------------------------------------------------------
class cPlayerDef : public IGameObjectDef
{
public:
	cPlayerDef(float radius, float speed, float height, float mouse_sensitivity, float fire_period)
		: mRadius(radius)
		, mSpeed(speed)
		, mHeight(height)
		, mMouseSensitivity(mouse_sensitivity)
		, mFirePeriod(fire_period)
	{
	}

	float mRadius;
	float mSpeed;
	float mHeight;
	float mMouseSensitivity;
	float mFirePeriod;
};

//----------------------------------------------------------------------------
class cPlayerState : public IGameObjectState
{
public:
	cPlayerState() {}
	cPlayerState(const cVector3& pos) : mPos(pos) {}

	virtual void Init(const IGameObjectState& game_object_state)
	{
		*this = static_cast<const cPlayerState&>(game_object_state);
	}

	cVector3 mPos;
};

//----------------------------------------------------------------------------
class cPlayer : IGameObject
{
	REGISTER_GAMEOBJECT(cPlayer, cPlayerDef, cPlayerState);

public:
	cPlayer();

	bool Init(const IGameObjectDef* def, IGameObjectState*&& initial_state) override;
	void Update(float elapsed) override;
	void Render() override;

private:
	cVector3	ComputeLinearVelocity() const;
	cVector3	ComputeEyePos() const;
	cVector3	ComputeLookAt();
	cVector3	GetForwardDir() const;


	float		mYaw;
	float		mPitch;
	cVector3	mLookAt;
	cVector2	mPrevMousePos;
	float		mLastShot;

	Mesh*		mCrosshair;
};

extern const cPlayerDef sDefaultPlayerDef;