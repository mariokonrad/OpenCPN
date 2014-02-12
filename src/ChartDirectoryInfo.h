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

#ifndef __CHARTDIRECTORYINFO__H__
#define __CHARTDIRECTORYINFO__H__

#include <vector>
#include <wx/string.h>

/// Holds information about a chart directory.
class ChartDirectoryInfo
{
public:
	ChartDirectoryInfo(const wxString& fullpath, const wxString& magic_number = wxString())
		: fullpath(fullpath)
		, magic_number(magic_number)
	{}

	wxString fullpath;
	wxString magic_number;
};

typedef std::vector<ChartDirectoryInfo> ChartDirectories;

#endif
