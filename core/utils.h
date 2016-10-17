/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Several general-purpose Core utilities
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "core/base.h"

//----------------------------------------------------------------------------
template <class T>
static T Clamp(const T& low, const T& value, const T& high)
{
	return (value < low)
		? low
		: (value > high)
			? high
			: value;
}

//----------------------------------------------------------------------------
// Similar to Clamp but does not care about min/max and is really intended only for numerical values (or the swap could hurt)
template <class T>
static T Limit(T a, T value, T b)
{
	if (a > b)
		std::swap(a, b);

	return Clamp(a, value, b);
}


//----------------------------------------------------------------------------
// Checks if value is inside the range [range_start, range_end], i.e., both ends are included 
template <class T>
static bool IsWithinRange(const T& range_start, const T& value, const T& range_end)
{
	return (value >= range_start) && (value <= range_end);
}

template <typename T> void ignore_expression(const T&) {}
#define ignore_expr(expression) ignore_expression((expression))
