/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Custom implementation of assert that prints some message
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "debugutils\debug.h"

#if defined _DEBUG
	#define CPR_assert(expr, ...) \
			do																		\
			{                                                                       \
				if (!(expr)) {														\
					Debug::ErrorMsg(__FILE__, __LINE__, #expr, __VA_ARGS__);		\
					__debugbreak();                                                 \
				}                                                                   \
			}                                                                       \
			while (false)
#else
	#define CPR_assert(expr, ...) ignore_expr(expr)
#endif