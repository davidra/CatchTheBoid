/***************************************************************************************************
vector3.h

Simple Vector3 class mainly destined to hide the ugliness of D3DXVECTOR3 while still being interchangeable
with it

by David Ramos
***************************************************************************************************/
#pragma once

#include <D3dx9math.h>

class cVector3 : public D3DXVECTOR3
{
public:
	// Inherit constructors (not working in VS2012)
	// using D3DXVECTOR3::D3DXVECTOR3;

	cVector3() {}
	cVector3(float val) : D3DXVECTOR3(val, val, val) {}
	cVector3(const D3DXVECTOR3& other) : D3DXVECTOR3(other) {}
	cVector3(float x, float y, float z) : D3DXVECTOR3(x, y, z) {}

	cVector3& RotateAroundY(float angle);
	cVector3& RotateAroundX(float angle);

	void SetNormalized();
	bool IsNormalized() const;

	float	Length() const;
	float	LengthSqr() const;

	cVector2 GetXZ() const;

	bool IsZero() const;

	enum class eAxis
	{
		X,
		Y,
		Z
	};

	template <eAxis axis>
	float Get() const;

	// In newer compiler versions these could be constexpr, here we rely on RVO
	static cVector3 XAXIS() { return cVector3(1.0f, 0.0f, 0.0f); }
	static cVector3 YAXIS() { return cVector3(0.0f, 1.0f, 0.0f); }
	static cVector3 ZAXIS() { return cVector3(0.0f, 0.0f, 1.0f); }
	static cVector3 ZERO()	{ return cVector3(0.0f, 0.0f, 0.0f); }
	static cVector3 ONE()	{ return cVector3(1.0f, 1.0f, 1.0f); }
};

inline cVector3 Normalize(const cVector3& v);
inline float	Dot(const cVector3& lhs, const cVector3& rhs);
inline cVector3 Cross(const cVector3& lhs, const cVector3& rhs);
inline cVector3 ProjectVectorOntoPlane(const cVector3& vector, const cVector3& plane_normal);
inline cVector3 Multiply(const cVector3& lhs, const cVector3& rhs);

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
inline float cVector3::Length() const
{
	return D3DXVec3Length(this);
}

//----------------------------------------------------------------------------
inline float cVector3::LengthSqr() const
{
	return D3DXVec3LengthSq(this);
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

//----------------------------------------------------------------------------
template <>
inline float cVector3::Get<cVector3::eAxis::X>() const
{
	return x;
}

//----------------------------------------------------------------------------
template <>
inline float cVector3::Get<cVector3::eAxis::Y>() const
{
	return y;
}

//----------------------------------------------------------------------------
template <>
inline float cVector3::Get<cVector3::eAxis::Z>() const
{
	return z;
}

//----------------------------------------------------------------------------
inline cVector3 Normalize(const cVector3& v)
{
	cVector3 result;
	return *D3DXVec3Normalize(&result, &v);
}

//----------------------------------------------------------------------------
inline bool cVector3::IsNormalized() const
{
	return IsSimilar(Length(), 1.0f);
};

//----------------------------------------------------------------------------
inline float Dot(const cVector3& lhs, const cVector3& rhs)
{
	return D3DXVec3Dot(&lhs, &rhs);
}

//----------------------------------------------------------------------------
inline cVector3 Cross(const cVector3& lhs, const cVector3& rhs)
{
	cVector3 result;
	return *D3DXVec3Cross(&result, &lhs, &rhs);
}

//----------------------------------------------------------------------------
inline cVector3 ProjectVectorOntoPlane(const cVector3& vector, const cVector3& plane_normal)
{
	CPR_assert(plane_normal.IsNormalized(), "The plane normal must be normalized!");

	// V||N = N x (V x N)
	return Cross(plane_normal, Cross(vector, plane_normal));
}

//----------------------------------------------------------------------------
inline cVector3 ReflectVectorOntoPlane(const cVector3& vector, const cVector3& plane_normal)
{
	CPR_assert(plane_normal.IsNormalized(), "The plane normal must be normalized!");

	return (vector - (plane_normal * Dot(plane_normal, vector) * 2.0f));
}

//----------------------------------------------------------------------------
inline cVector3 Multiply(const cVector3& lhs, const cVector3& rhs)
{
	return cVector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}