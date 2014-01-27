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

#include "LatLonBoundingBox.h"

namespace geo {

// Is the given LL point in the boundingbox ??
bool LatLonBoundingBox::PointInBox(double Lon, double Lat, double Marge) const
{
	double x = Lon;
	double y = Lat;

	// Box is centered in East lon, crossing IDL
	if (m_maxx > 180.0) {
		if (x < m_maxx - 360.0)
			x += 360.0;

		if (x >= (m_minx - Marge) && x <= (m_maxx + Marge) && y >= (m_miny - Marge)
			&& y <= (m_maxy + Marge))
			return true;
		return false;
	}

	// Box is centered in Wlon, crossing IDL
	if (m_minx < -180.0) {
		if (x > m_minx + 360.0)
			x -= 360.0;

		if (x >= (m_minx - Marge) && x <= (m_maxx + Marge) && y >= (m_miny - Marge)
			&& y <= (m_maxy + Marge))
			return true;
		return false;
	}

	if (x >= (m_minx - Marge) && x <= (m_maxx + Marge) && y >= (m_miny - Marge)
		&& y <= (m_maxy + Marge))
		return true;
	return false;
}

}

