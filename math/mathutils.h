/***************************************************************************************************
mathutils.h

math-related constants and utilities
 
by David Ramos
***************************************************************************************************/
#pragma once

// TODO: review if/when I get the VS2015 libs to make template typedefs and have constexpr instead of preprocessor macros
#define PI 3.14159265358979323846f
#define METERS 1.0f
#define SECONDS 1.0f
#define TO_RADIANS(deg) (deg * PI/180.0f)
#define TO_DEGREES(rad)	(rad * 180.0f/PI)
#define HALF 0.5f
#define EPSILON 1e-5f
#define INVALID_INTERSECT_RESULT FLT_MAX

//----------------------------------------------------------------------------
inline bool IsSimilar(float lhs, float rhs, float epsilon = EPSILON)
{
	return fabsf(lhs - rhs) < epsilon;
}

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