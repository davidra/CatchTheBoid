/***************************************************************************************************
aabb.h
 
by David Ramos
***************************************************************************************************/
#pragma once

//----------------------------------------------------------------------------
class cAABB2D
{
public:
	cAABB2D(const cVector2& p);
	cAABB2D(const cVector2& aabb_min, const cVector2& aabb_max);

	cVector2 mMin;
	cVector2 mMax;
};

//----------------------------------------------------------------------------
class cAABB
{
public:
	cAABB() {}
	cAABB(const cVector3& p);
	cAABB(const cVector3& aabb_min, const cVector3& aabb_max);

	cAABB2D GetXZ() const;

	cVector3 mMin;
	cVector3 mMax;
};

//----------------------------------------------------------------------------
inline cAABB::cAABB(const cVector3& p)
	: mMin(p)
	, mMax(p)
{
}

//----------------------------------------------------------------------------
inline cAABB::cAABB(const cVector3& aabb_min, const cVector3& aabb_max)
	: mMin(aabb_min)
	, mMax(aabb_max)
{
	CPR_assert((mMin.x < mMax.x)
		&& (mMin.y < mMax.y)
		&& (mMin.z < mMax.z), "incorrect min/max vectors provided");
}

//----------------------------------------------------------------------------
inline cAABB2D cAABB::GetXZ() const
{
	return cAABB2D(cVector2(mMin.x, mMin.z), cVector2(mMax.x, mMax.z));
}

//----------------------------------------------------------------------------
inline cAABB2D::cAABB2D(const cVector2& p)
	: mMin(p)
	, mMax(p)
{
}

//----------------------------------------------------------------------------
inline cAABB2D::cAABB2D(const cVector2& aabb_min, const cVector2& aabb_max)
	: mMin(aabb_min)
	, mMax(aabb_max)
{
}
