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

#ifndef __GEO__GEODESIC__H__
#define __GEO__GEODESIC__H__

#include <cstdlib>

namespace geo {

class Position;

/// Find the distance (meters) and bearings between two Lon/Lat pairs (given in degrees)
/// Results are in meters and degrees as appropriate
class Geodesic
{
public:
	static double GreatCircleDistBear(const Position& pos1, const Position& pos2,
									  double* Dist = NULL, double* Bear1 = NULL,
									  double* Bear2 = NULL);

	static void GreatCircleTravel(const Position& pos1, double Dist, double Bear1,
								  double* Lon2 = NULL, double* Lat2 = NULL, double* Bear2 = NULL);
};

}

#endif
