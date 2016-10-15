/***************************************************************************************************
intersect_tests.h
 
by David Ramos
***************************************************************************************************/
#pragma once

//----------------------------------------------------------------------------
// Axis-aligned-2D-lines related tests
inline float IntersectRayWithXAxisAlignedLine2D(const cVector2& org, const cVector2& dir, float line_y)
{
	CPR_assert(dir.IsNormalized(), "dir is not normalized!");

	if (dir.y == 0.0f)
	{
		// Parallel
		return INVALID_INTERSECT_RESULT;
	}

	const float result = (line_y - org.y) / dir.y;
	if (result < 0.0f)
	{
		// not coincidental
		return INVALID_INTERSECT_RESULT;
	}

	return result;

}

inline float IntersectRayWithYAxisAlignedLine2D(const cVector2& org, const cVector2& dir, float line_x)
{
	CPR_assert(dir.IsNormalized(), "dir is not normalized!");

	if (dir.x == 0.0f)
	{
		// Parallel
		return INVALID_INTERSECT_RESULT;
	}

	const float result = (line_x - org.x) / dir.x;
	if (result < 0.0f)
	{
		// not coincidental
		return INVALID_INTERSECT_RESULT;
	}

	return result;
}

inline float DistanceToXAxisAlignedLine2D(const cVector2& point, float line_y)
{
	return fabsf(line_y - point.y);
}

inline float DistanceToYAxisAlignedLine2D(const cVector2& point, float line_x)
{
	return fabsf(line_x - point.x);
}