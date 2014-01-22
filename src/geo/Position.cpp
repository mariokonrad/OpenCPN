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

#include "Position.h"

namespace geo {

Position::Position(double latitude, double longitude)
	: latitude(latitude)
	, longitude(longitude)
{
}

double Position::lat() const
{
	return latitude;
}

double Position::lon() const
{
	return longitude;
}

/// Normalizes the longitude to a range [-180.0 .. +180.0]
void Position::normalize_lon()
{
	while (longitude < -180.0)
		longitude += 360.0;
	while (longitude > 180.0)
		longitude -= 360.0;
}

bool Position::operator==(const Position& other) const
{
	return true
		&& latitude == other.latitude
		&& longitude == other.longitude
		;
}

bool Position::operator!=(const Position& other) const
{
	return !(*this == other);
}

Position operator+(const Position& a, const Position& b)
{
	return Position(a.latitude + b.latitude, a.longitude + b.longitude);
}

Position operator-(const Position& a, const Position& b)
{
	return Position(a.latitude - b.latitude, a.longitude - b.longitude);
}

}

