/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __TIDE__STATION_DATA_H__
#define __TIDE__STATION_DATA_H__

#include <wx/defs.h>
#include <wx/wxchar.h>
#include <string>
#include <vector>

namespace tide {

class Station_Data
{
public:
	Station_Data();
	~Station_Data();

	std::string station_name;
	wxChar station_type; // T or C
	std::vector<double> amplitude;
	std::vector<double> epoch;
	double DATUM;
	int meridian; // **UNUSED**
	double zone_offset;
	char tzfile[40];
	char unit[40];
	char units_conv[40]; // printable converted units
	char units_abbrv[20]; // and abbreviation
	int have_BOGUS;
};

}

#endif
