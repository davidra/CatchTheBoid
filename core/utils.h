/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Several general-purpose Core utilities
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "core/base.h"

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
