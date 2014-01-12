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

#ifndef __GLOBAL__COLORSCHEME__H__
#define __GLOBAL__COLORSCHEME__H__

namespace global {

/// Enumeration of all possible color schemes. Used to present the GUI
/// differently, depending on the desired brightness.
///
/// This enumeration must be gapless.
enum ColorScheme {
	GLOBAL_COLOR_SCHEME_INVALID = -1,
	GLOBAL_COLOR_SCHEME_RGB = 0,
	GLOBAL_COLOR_SCHEME_DAY = 1,
	GLOBAL_COLOR_SCHEME_DUSK = 2,
	GLOBAL_COLOR_SCHEME_NIGHT = 3,
	GLOBAL_COLOR_SCHEME_MAX
};

}

#endif
