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

#include <wx/wx.h>

//#ifdef __MSVC__
//	#define _putenv _putenv  // This is for MSVC
//#else
//	#define _putenv putenv   // This is for other Windows compiler
//#endif

// Use the CPL Portability library only if S57 is enabled
#ifdef USE_S57
	#define USE_CPL
	#include "cpl_port.h"
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

#endif
