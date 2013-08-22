#ifndef __MICROSOFTCOMPATIBILITY__H__
#define __MICROSOFTCOMPATIBILITY__H__

#ifdef __MSVC__

#include <windows.h>

long  __stdcall MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS * ExceptionInfo);

// __MSVC__ randomly does not link snprintf, or _snprintf
// Replace it with a local version, code is in cutil.c
#define snprintf mysnprintf
int mysnprintf( char *buffer, int count, const char *format, ... );

#define round round_msvc
double round_msvc(double flt);

#ifndef fmin
double fmin(double, double);
#endif

#ifndef fmax
double fmax(double, double);
#endif


#endif

#endif
