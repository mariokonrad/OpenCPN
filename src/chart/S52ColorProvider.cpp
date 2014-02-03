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

#include "S52ColorProvider.h"

#ifdef USE_S57 // FIXME: remove this preprocessor stuff
	#include <chart/s52s57.h>
	#include <chart/s52plib.h>
	#include <chart/ColorTable.h>
	extern chart::s52plib* ps52plib;
#endif

namespace chart {

S52ColorProvider::~S52ColorProvider()
{
}

wxColour S52ColorProvider::get_color(const wxString& color_name) const
{
	// Use the S52 Presentation library if present
	if (ps52plib) {
		return ps52plib->getwxColour(color_name);
	}

	return wxColour();
}

}

