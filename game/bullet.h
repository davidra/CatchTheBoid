/***************************************************************************************************
bullet.h

by David Ramos
***************************************************************************************************/
#pragma once

#include "game/gameobject.h"
#include "game/GameObjectManager.h"

#include "modelrepository.h"

//----------------------------------------------------------------------------
class cBulletDef : public IGameObjectDef
{
public:
	cBulletDef(float radius, float speed, const cColor& color, float duration)
		: mRadius(radius)
		, mSpeed(speed)
		, mColor(color)
		, mDuration(duration)
	{
	}

	float			GetRadius() const { return mRadius; }
	float			GetSpeed() const { return mSpeed;  }
	float			GetDuration() const { return mDuration;  }
	const cColor&	GetColor() const { return mColor;  }

private:
	float		mRadius;
	float		mSpeed;
	cColor		mColor;
	float		mDuration;
};

//----------------------------------------------------------------------------
class cBulletState : public IGameObjectState
{
public:
	cBulletState() {}

	cBulletState(const cVector3& pos, const cVector3& linear_vel)
		: mPos(pos)
		, mLinearVelocity(linear_vel)
	{
	}

	void Init(const IGameObjectState& game_object_state) override
	{
		*this = static_cast<const cBulletState&>(game_object_state);
	}

	cVector3 mPos;
	cVector3 mLinearVelocity;
};

//----------------------------------------------------------------------------
class cBullet : public IGameObject
{
	REGISTER_GAMEOBJECT(cBullet, cBulletDef, cBulletState)
public:

	cBullet() 
		: mModel(nullptr)
		, mLifeTime(0.0f)
	{}

	bool Init(const IGameObjectDef* def, IGameObjectState*&& initial_state) override;
	void Update(float elapsed) override;
	void Render() override;

private:
	Mesh* mModel;

	float mLifeTime;
};

extern cBulletDef gPlayerBullets;

