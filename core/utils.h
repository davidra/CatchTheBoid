/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Several general-purpose Core utilities
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "core/base.h"

// TODO: Rethink this, probably the Core namespace is not needed, so remove it
namespace Core
{
	using std::move;
	using std::forward;
	using std::begin;
	using std::end;

	template <class T>
	static T Clamp(const T& low, const T& value, const T& high)
	{
		return (value < low)
			? low
			: (value > high)
				? high
				: value;
	}
}

//----------------------------------------------------------------------------
// Checks if value is inside the range [range_start, range_end), i.e., range_start is included 
template <class T>
static bool IsWithinRange(const T& range_start, const T& value, const T& range_end)
{
	return (value >= range_start) && (value < range_end);
}

template <typename T> void ignore_expression(const T&) {}
#define ignore_expr(expression) ignore_expression((expression))
