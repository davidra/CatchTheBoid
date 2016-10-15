/***************************************************************************************************
aabb.h
 
by David Ramos
***************************************************************************************************/
#pragma once

//----------------------------------------------------------------------------
class cAABB
{
public:
	cAABB(const cVector3& p);
	cAABB(const cVector3& aabb_min, const cVector3& aabb_max);

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
}


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
