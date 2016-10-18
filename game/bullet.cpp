#include "stdafx.h"

#include "game/bullet.h"
#include "game/world.h"

cBulletDef gPlayerBullets(0.2f, 8.0f, TCOLOR_RED, 6.0f);

//----------------------------------------------------------------------------
bool cBullet::Init(const IGameObjectDef* def, IGameObjectState*&& initial_state)
{
	bool success = IGameObject::Init(def, std::move(initial_state));
	if (success)
	{
		mModel = ModelRepo::GetModel(MID_SPHERE);
		State().mLinearVelocity *= Def().GetSpeed();
	}

	return success;
}

//----------------------------------------------------------------------------
void cBullet::Update(float elapsed)
{
	mLifeTime += elapsed;
	if (mLifeTime > Def().GetDuration())
	{
		// We never go beyond our lifetime. Why so accurate? well, because we can and is easy
		elapsed = elapsed - Def().GetDuration();
		SetPendingDestroy();
		if (IsSimilar(elapsed, 0.0f))
		{
			return;
		}
	}

	auto& state = State();

	cVector3 movement = state.mLinearVelocity * elapsed;
	cVector3 new_pos = state.mPos + movement;
	cVector3 coll_pos;
	cVector3 coll_normal;
	if (cWorld::GetInstance()->CastSphereAgainstWorld(state.mPos, new_pos, Def().GetRadius(), true, coll_pos, coll_normal))
	{
		coll_pos += coll_normal * Def().GetRadius();
		const cVector3 reflecting_vector = ReflectVectorOntoPlane(new_pos - coll_pos, coll_normal);
		state.mLinearVelocity = Normalize(reflecting_vector) * Def().GetSpeed();
		new_pos = state.mPos + reflecting_vector;
	}
	
	state.mPos = new_pos;
}

//----------------------------------------------------------------------------
void cBullet::Render()
{
	mModel->Render(State().mPos, cVector3::ZERO(), cVector3::ONE() * Def().GetRadius(), Def().GetColor());
}


