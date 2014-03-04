#ifndef __WINDOWS__COMPATIBILITY__H__
#define __WINDOWS__COMPATIBILITY__H__

#ifdef __MSVC__

#ifdef __MSVC__66
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
	#endif
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795028841972)
#endif

#include <windows.h>

long __stdcall MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS * ExceptionInfo);

// __MSVC__ randomly does not link snprintf, or _snprintf
// Replace it with a local version
#define snprintf mysnprintf
int mysnprintf(char* buffer, int count, const char* format, ...);

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
