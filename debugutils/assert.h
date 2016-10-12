/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Custom implementation of assert that prints some message
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "debugutils\debug.h"

#define CPR_assert(pp_expr, ...) \
		do																		\
		{                                                                       \
			if (!(pp_expr)) {                                                   \
				Debug::ErrorMsg(__FILE__, __LINE__, #pp_expr, __VA_ARGS__);		\
				__debugbreak();                                                 \
			}                                                                   \
		}                                                                       \
		while (false)
