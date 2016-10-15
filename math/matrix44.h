/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Simple Matrix wrapper class mainly destined to hide the ugliness of D3DXMatrix while still being 
/// interchangeable with it
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include <D3dx9math.h>

//----------------------------------------------------------------------------
class cMatrix44 : public D3DXMATRIX
{
public:
	cMatrix44() : D3DXMATRIX() {}
	cMatrix44(const cVector3& axis, float angle);
	cMatrix44(const cVector3& axis, float angle, const cVector3& translation);

	void				SetTranslation(const cVector3& vector);
	void				SetRotation(const cVector3& axis, float angle);

	cVector3			RotateVector(const cVector3& vector) const;
	cVector3			RotateCoord(const cVector3& coord) const;

	cVector3			XAxis() const;
	cVector3			YAxis() const;
	cVector3			ZAxis() const;
	cVector3			GetTranslation() const;

	static cMatrix44	IDENTITY();
};

//----------------------------------------------------------------------------
inline cMatrix44::cMatrix44(const cVector3& axis, float angle)
{
	SetRotation(axis, angle);
}

//----------------------------------------------------------------------------
inline cMatrix44::cMatrix44(const cVector3& axis, float angle, const cVector3& translation)
{
	SetRotation(axis, angle);
	SetTranslation(translation);
}

//----------------------------------------------------------------------------
inline void cMatrix44::SetRotation(const cVector3& axis, float angle)
{
	const cVector3 translation(GetTranslation());
	D3DXMatrixRotationAxis(this, &axis, angle);
	SetTranslation(translation);
}

//----------------------------------------------------------------------------
inline void cMatrix44::SetTranslation(const cVector3& vector)
{
	_41 = vector.x;
	_42 = vector.y;
	_43 = vector.z;
}

//----------------------------------------------------------------------------
inline cVector3	cMatrix44::RotateVector(const cVector3& vector) const
{
	cVector3 result;
	return static_cast<cVector3&>(*D3DXVec3TransformNormal(&result, &vector, this));
}

//----------------------------------------------------------------------------
inline cVector3	cMatrix44::RotateCoord(const cVector3& coord) const
{
	cVector3 result;
	return static_cast<cVector3&>(*D3DXVec3TransformCoord(&result, &coord, this));
}

//----------------------------------------------------------------------------
inline cVector3	cMatrix44::XAxis() const
{
	return cVector3(_11, _12, _13);
}

//----------------------------------------------------------------------------
inline cVector3	cMatrix44::YAxis() const
{
	return cVector3(_21, _22, _23);
}

//----------------------------------------------------------------------------
inline cVector3	cMatrix44::ZAxis() const
{
	return cVector3(_31, _32, _33);
}

//----------------------------------------------------------------------------
inline cVector3	cMatrix44::GetTranslation() const
{
	return cVector3(_41, _42, _43);
}

//----------------------------------------------------------------------------
inline cMatrix44 cMatrix44::IDENTITY()
{
	cMatrix44 identity;
	return static_cast<cMatrix44&>(*D3DXMatrixIdentity(&identity));
}
