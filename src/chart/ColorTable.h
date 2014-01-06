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

#ifndef __CHART__COLORTABLE__H__
#define __CHART__COLORTABLE__H__

#include <wx/string.h>
#include <wx/colour.h>

#include <chart/S52Color.h>

namespace chart {

WX_DECLARE_STRING_HASH_MAP(wxColour, wxColorHashMap);
WX_DECLARE_STRING_HASH_MAP(S52color, colorHashMap);

class ColorTable
{
public:
	ColorTable(const wxString& tableName)
		: tableName(tableName)
	{
	}

	wxString tableName;
	wxString rasterFileName;
	wxArrayPtrVoid* color;
	colorHashMap colors;
	wxColorHashMap wxColors;
};

}

#endif
