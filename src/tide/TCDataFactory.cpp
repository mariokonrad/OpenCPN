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

#include "TCDataFactory.h"

namespace tide {

TCDataFactory::TCDataFactory()
{
	// Build the units array
	known_units[0].name = "feet";
	known_units[0].abbrv = "ft";
	known_units[0].type = LENGTH;
	known_units[0].conv_factor = 0.3048;

	known_units[1].name = "meters";
	known_units[1].abbrv = "m";
	known_units[1].type = LENGTH;
	known_units[1].conv_factor = 1.0;

	known_units[2].name = "knots";
	known_units[2].abbrv = "ky";
	known_units[2].type = VELOCITY;
	known_units[2].conv_factor = 1.0;

	known_units[3].name = "knots^2";
	known_units[3].abbrv = "kt^2";
	known_units[3].type = BOGUS;
	known_units[3].conv_factor = 1.0;
}

TCDataFactory::~TCDataFactory()
{
}

/// Find a unit; returns -1 if not found.
int TCDataFactory::findunit(const char* unit) const
{
	for (int a = 0; a < sizeof(known_units) / sizeof(known_units[0]); ++a) {
		if ((known_units[a].name == unit) || (known_units[a].abbrv == unit))
			return a;
	}
	return -1;
}

/// Returns a reference to the unit information at the specified index.
///
/// @note This method does not check if the specified index is in range.
const TCDataFactory::UnitInfo& TCDataFactory::get_unit(int index) const
{
	return known_units[index];
}

}

