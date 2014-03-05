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

#ifndef __POSITIONCONVERT__H__
#define __POSITIONCONVERT__H__

#include <geo/Position.h>
#include <wx/string.h>

/// Class which provides functionality to convert position from and to strings.
///
/// FIXME: position conversion and parsing (see PositionParser) does almost the
///        same. they have to be merged.
class PositionConvert
{
public:
	static wxString lat(double a, bool high_precision = true);
	static wxString lon(double a, bool high_precision = true);

	static geo::Position pos(const wxString& lat_str, const wxString& lon_str);

	static double lat(const wxString&);
	static double lon(const wxString&);

private:
	static wxString toSDMM(int NEflag, double a, bool hi_precision);
	static double fromDMM(wxString);
};

#endif

