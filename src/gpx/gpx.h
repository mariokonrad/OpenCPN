/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#ifndef __GPX__GPX__H__
#define __GPX__GPX__H__

#include <wx/colour.h>
#include <wx/string.h>

namespace gpx {

// The last color defined by Garmin is transparent - we ignore it
const wxString GpxxColorNames[] =
{
	_("Black"),
	_("DarkRed"),
	_("DarkGreen"),
	_("DarkYellow"),
	_("DarkBlue"),
	_("DarkMagenta"),
	_("DarkCyan"),
	_("LightGray"),
	_("DarkGray"),
	_("Red"),
	_("Green"),
	_("Yellow"),
	_("Blue"),
	_("Magenta"),
	_("Cyan"),
	_("White")
};

const wxColour GpxxColors[] =
{
	wxColour(0x00, 0x00, 0x00),
	wxColour(0x60, 0x00, 0x00),
	wxColour(0x00, 0x60, 0x00),
	wxColour(0x80, 0x80, 0x00),
	wxColour(0x00, 0x00, 0x60),
	wxColour(0x60, 0x00, 0x60),
	wxColour(0x00, 0x80, 0x80),
	wxColour(0xC0, 0xC0, 0xC0),
	wxColour(0x60, 0x60, 0x60),
	wxColour(0xFF, 0x00, 0x00),
	wxColour(0x00, 0xFF, 0x00),
	wxColour(0xF0, 0xF0, 0x00),
	wxColour(0x00, 0x00, 0xFF),
	wxColour(0xFE, 0x00, 0xFE),
	wxColour(0x00, 0xFF, 0xFF),
	wxColour(0xFF, 0xFF, 0xFF)
};

const int StyleValues[] =
{
	-1,
	wxSOLID,
	wxDOT,
	wxLONG_DASH,
	wxSHORT_DASH,
	wxDOT_DASH
};

const int WidthValues[] =
{
	-1,
	 1,
	 2,
	 3,
	 4,
	 5,
	 6,
	 7,
	 8,
	 9,
	10
};

}

#endif
