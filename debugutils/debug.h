/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Debug-related functions
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "core\base.h"
#include "core\utils.h"

class Mesh;
class cVector3;

namespace Debug
{
	bool HasDebuggerAttached();

	void ErrorMsg(const char* file, int line, const char* expr, const char* format, ...);

	void WriteLine(const char* fmt, ...);
};