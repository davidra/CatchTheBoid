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
template <typename T>
int Sign(const T& val)
{
	return (T(0) < val) - (val < T(0));
}
