/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Simple Vector3 class mainly destined to hide the ugliness of D3DXVECTOR3 while still being interchangeable
/// with it
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include <D3dx9math.h>

class cVector3 : public D3DXVECTOR3
{
public:
	// Inherit constructors (not working in VS2012)
	// using D3DXVECTOR3::D3DXVECTOR3;

	cVector3() {}
	cVector3(const D3DXVECTOR3& other) : D3DXVECTOR3(other) {}
	cVector3(float x, float y, float z) : D3DXVECTOR3(x, y, z) {}

	cVector3& RotateAroundY(float angle);
	cVector3& RotateAroundX(float angle);

	void SetNormalized();

	cVector2 GetXZ() const;

	bool IsZero() const;

	// In newer compiler versions these could be constexpr, here we rely on RVO
	static cVector3 XAXIS() { return cVector3(1.0f, 0.0f, 0.0f); }
	static cVector3 YAXIS() { return cVector3(0.0f, 1.0f, 0.0f); }
	static cVector3 ZAXIS() { return cVector3(0.0f, 0.0f, 1.0f); }
	static cVector3 ZERO()	{ return cVector3(0.0f, 0.0f, 0.0f); }
	static cVector3 ONE()	{ return cVector3(1.0f, 1.0f, 1.0f); }
};

//----------------------------------------------------------------------------	
inline bool IsSimilar(const cVector3& lhs, const cVector3& rhs, float epsilon = EPSILON)
{
	return IsSimilar(lhs.x, rhs.x, epsilon)
		&& IsSimilar(lhs.y, rhs.y, epsilon)
		&& IsSimilar(lhs.z, rhs.z, epsilon);
}

//----------------------------------------------------------------------------	
inline cVector3& cVector3::RotateAroundY(float angle)
{
	// Negate angle to be more left-handedness-correct
	const float cosine = cos(-angle);
	const float sine = sin(-angle);

	*this = cVector3(
		(x * cosine) - (z * sine)
		, y
		, (x * sine) + (z * cosine));

	return *this;
}

//----------------------------------------------------------------------------	
inline cVector3& cVector3::RotateAroundX(float angle)
{
	// Negate angle to be more left-handedness-correct
	const float cosine = cos(angle);
	const float sine = sin(angle);

	*this = cVector3(
		x
		, (y * cosine) - (z * sine)
		, (y * sine) + (z * cosine));

	return *this;
}

//----------------------------------------------------------------------------
inline void cVector3::SetNormalized()
{
	D3DXVec3Normalize(this, this);
}

//----------------------------------------------------------------------------
inline cVector2 cVector3::GetXZ() const
{
	return cVector2(x, z);
}

//----------------------------------------------------------------------------
inline bool cVector3::IsZero() const
{
	return !!(*this == cVector3::ZERO());
}