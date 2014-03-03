/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef __DYCHART__H__
#define __DYCHART__H__

// Microsoft compiler warning suppression
#ifdef __MSVC__
#pragma warning(disable:4114)
#pragma warning(disable:4284)  // This one is to do with "reverse iterator UDT..." Huh?
#endif


// Following definition required by GDAL
#define notdef 1


// Windows specific
#ifdef __WXMSW__
    #include "wx/msw/private.h"
#endif

#ifdef __MSVC__66
        #ifdef _DEBUG
            #define _CRTDBG_MAP_ALLOC
            #include <crtdbg.h>
        #endif
#endif

#ifdef __MSVC__
	#define _putenv _putenv  // This is for MSVC
#else
	#define _putenv putenv   // This is for other Windows compiler
#endif

// Use the CPL Portability library only if S57 is enabled
#ifdef USE_S57
	#define USE_CPL
	#include "cpl_port.h"
#endif


// FIXME: not sure if this is necessary
// Define __POSIX__ to imply posix thread model compatibility
// Especially used for communication port multithreading.
//
// Posix thread model is available on selected platforms, see code.
#ifdef __POSIX__
	#undef __POSIX__
#endif

#ifdef  __WXOSX__
	#define __POSIX__
#endif

#ifdef  __WXGTK__
	#define __POSIX__
#endif


// Enable GTK Display Optimization
// Note this requires libgtk+2-devel
// which is not often available on basic systems.
// On standard linux platforms, configure will set
// ocpnUSE_GTK_OPTIMIZE if possible, i.e. if libgtk+2-devel is installed
#ifdef __WXGTK__
	#ifdef ocpnUSE_GTK_OPTIMIZE
		#include <gtk/gtk.h>
	#endif
#endif

#ifndef OCPN_GL_INCLUDES
	#define OCPN_GL_INCLUDES 1
	#ifdef __WXMSW__
		#include "GL/gl.h"  // local copy for Windows
		#include "GL/glu.h"
		#include "GL/glext.h"
		#include <windows.h>
	#else
		#include <GL/gl.h>
		#include <GL/glu.h>
		#include <GL/glext.h>
	#endif
#endif

#endif
