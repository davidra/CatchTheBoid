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

	void		Extend(float amount);

	cAABB2D		GetXZ() const;
	cVector3	GetCentroid() const;

	bool IsInside(const cVector3& point) const;

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
inline void cAABB::Extend(float amount)
{
	const cVector3 extension(amount);
	mMin -= extension;
	mMax += extension;
}

//----------------------------------------------------------------------------
inline cAABB2D cAABB::GetXZ() const
{
	return cAABB2D(cVector2(mMin.x, mMin.z), cVector2(mMax.x, mMax.z));
}

//----------------------------------------------------------------------------
inline cVector3 cAABB::GetCentroid() const
{
	return (mMin + mMax) * HALF;
}

//----------------------------------------------------------------------------
inline bool cAABB::IsInside(const cVector3& point) const
{
	return (point.x >= mMin.x) && (point.x <= mMax.x)
		&& (point.y >= mMin.y) && (point.y <= mMax.y)
		&& (point.z >= mMin.z) && (point.z <= mMax.z);
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
