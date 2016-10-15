#include "stdafx.h"

#include "debug.h"

//----------------------------------------------------------------------------
namespace Debug
{
	//----------------------------------------------------------------------------
	bool HasDebuggerAttached()
	{
		if (::IsDebuggerPresent())
			return true;

		BOOL remote_debugger_present = FALSE;
		if (FALSE == ::CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote_debugger_present))
			return false;

		return remote_debugger_present != FALSE;
	}

	//----------------------------------------------------------------------------
	void ErrorMsg(const char* file, int line, const char* expr, const char* format, ...)
	{
		const int large_enough = 1024;
		char buffer[large_enough] = {};

		va_list args;

		va_start(args, format);
		const int num_written = vsnprintf_s(buffer, _TRUNCATE, format, args);
		va_end(args);

		buffer[(std::min)(num_written + 1, large_enough) - 1] = 0;

		WriteLine("Error: (%s) - %s\n", expr, buffer);
	}

	//----------------------------------------------------------------------------
	void WriteLine(const char* fmt, ...)
	{
		const int large_enough = 1024;
		char buffer[large_enough] = {};

		va_list args;

		va_start(args, fmt);
		const int vsnprintf_result = vsnprintf_s(buffer, _TRUNCATE, fmt, args);
		const int num_written = vsnprintf_result >= 0 ? vsnprintf_result : large_enough - 1;
		va_end(args);

		const int room_for_cr_lf_and_terminating_nul = 3;
		const int end_of_str_fix_up_start = Clamp(0, num_written, large_enough - room_for_cr_lf_and_terminating_nul);

		const char cr = '\r';
		const char lf = '\n';
		const int prev_char = -1;

		char* ptr = buffer + end_of_str_fix_up_start;
		for (; ; --ptr)
			if (ptr <= buffer)
				break;
			else if ((prev_char[ptr] != cr) && (prev_char[ptr] != lf))
				break;
			else
				*ptr = 0;

		ptr[0] = cr;
		ptr[1] = lf;
		ptr[2] = '\0';

		::OutputDebugStringA(buffer);
		printf("%s", buffer);
	}
}
