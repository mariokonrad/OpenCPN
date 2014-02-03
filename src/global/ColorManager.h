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

#ifndef __GLOBAL__COLORMANAGER__H__
#define __GLOBAL__COLORMANAGER__H__

#include <global/ColorScheme.h>
#include <wx/colour.h>

namespace global {

/// Manages the color scheme.
class ColorManager
{
public:
	virtual ~ColorManager()
	{
	}

	/// Returns the color corresponding to the specified name.
	virtual wxColour get(const wxString& color_name) const = 0;

	/// Sets the current color scheme.
	virtual void set_current(ColorScheme scheme) = 0;

	/// Returns the current color scheme.
	virtual ColorScheme get_current() const = 0;
};

}

#endif
