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

// Is the given LL point in the boundingbox ??
bool LatLonBoundingBox::PointInBox(double Lon, double Lat, double Marge)
{
	double x = Lon;
	double y = Lat;

	//    Box is centered in East lon, crossing IDL
	if(m_maxx > 180.)
	{
		if( x < m_maxx - 360.)
			x +=  360.;

		if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
				y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
			return TRUE;
		return FALSE;
	}

	//    Box is centered in Wlon, crossing IDL
	else if(m_minx < -180.)
	{
		if(x > m_minx + 360.)
			x -= 360.;

		if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
				y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
			return TRUE;
		return FALSE;
	}

	else
	{
		if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
				y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
			return TRUE;
		return FALSE;
	}
}

