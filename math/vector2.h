/***************************************************************************************************
vector2.h
 
2D Vector class, a wrapper over D3DXVECTOR2

by David Ramos
***************************************************************************************************/
#pragma once

#include <D3dx9math.h>

//----------------------------------------------------------------------------
class cVector2 : public D3DXVECTOR2
{
public:
	cVector2() : D3DXVECTOR2() {}
	cVector2(float x, float y) : D3DXVECTOR2(x, y) {}
	cVector2(const D3DXVECTOR2& other) : D3DXVECTOR2(other) {}

	float	Length() const;
	void	SetNormalized();
	bool	IsNormalized() const;

	bool	IsZero() const;

	static cVector2 ZERO() { return cVector2(0.0f, 0.0f); }
};

inline cVector2 Normalize(const cVector2& v);

//----------------------------------------------------------------------------
inline float cVector2::Length() const
{
	return D3DXVec2Length(this);
}

//----------------------------------------------------------------------------
inline void cVector2::SetNormalized()
{
	D3DXVec2Normalize(this, this);
}

//----------------------------------------------------------------------------
inline bool cVector2::IsNormalized() const
{
	return IsSimilar(Length(), 1.0f);
};

//----------------------------------------------------------------------------
inline bool cVector2::IsZero() const
{
	return !!(*this == cVector2::ZERO());
}

//----------------------------------------------------------------------------
inline cVector2 Normalize(const cVector2& v)
{
	cVector2 result;
	return static_cast<cVector2&>(*D3DXVec2Normalize(&result, &v));
}

