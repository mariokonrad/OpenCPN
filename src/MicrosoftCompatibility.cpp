#include "MicrosoftCompatibility.h"

#ifdef __MSVC__

#include <float.h>

extern long __stdcall MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	//    return EXCEPTION_EXECUTE_HANDLER ;        // terminates the app

	switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
			_clear87();
			return EXCEPTION_CONTINUE_EXECUTION ;     // retry

		default:
			return EXCEPTION_CONTINUE_SEARCH ;         // standard fatal dialog box
	}
}

/* Replacement for __MSVC__ in absence of snprintf or _snprintf  */
extern int mysnprintf( char *buffer, int count, const char *format, ...)
{
	int ret;

	va_list arg;
	va_start(arg, format);
	ret = _vsnprintf(buffer, count, format, arg);

	va_end(arg);
	return ret;
}

double round_msvc(double x)
{
	return(floor(x + 0.5));
}

#endif

