/***************************************************************************************************
player.h
 
Player class

by David Ramos
***************************************************************************************************/
#pragma once

//----------------------------------------------------------------------------
class cPlayer
{
public:
	cPlayer();

	void Update(float elapsed);
	void Render();
private:
	cVector3	ComputeLinearVelocity() const;
	cVector3	ComputeLookAt();

	float		mYaw;
	float		mPitch;
	cVector3	mPos;
	cVector3	mLookAt;
	cVector2	mPrevMousePos;
};