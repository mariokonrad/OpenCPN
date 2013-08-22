#ifndef __MICROSOFTCOMPATIBILITY__H__
#define __MICROSOFTCOMPATIBILITY__H__

#ifdef __MSVC__

#include <windows.h>

#ifdef __cplusplus

extern "C" long  __stdcall MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS * ExceptionInfo);
extern "C" int mysnprintf( char *buffer, int count, const char *format, ... );
extern "C" double round_msvc(double flt);

#else

extern long __stdcall MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS * ExceptionInfo);
extern int mysnprintf( char *buffer, int count, const char *format, ... );

#endif

#endif

#endif
