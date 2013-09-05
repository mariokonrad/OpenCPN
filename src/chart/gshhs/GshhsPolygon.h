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

#ifndef __CHART__GSHHS__GSHHSPOLYGON__H__
#define __CHART__GSHHS__GSHHSPOLYGON__H__

#include <cstdio>
#include <vector>

class GshhsPoint;

class GshhsPolygon
{
	public:
		GshhsPolygon( FILE *file );

		virtual ~GshhsPolygon();

		int getLevel() { return flag & 255; }
		int isGreenwich() { return greenwich; }
		int isAntarctic() { return antarctic; }
		bool isOk() { return ok; }
		int readInt4();
		int readInt2();

		int id; /* Unique polygon id number, starting at 0 */
		int n; /* Number of points in this polygon */
		int flag; /* level + version << 8 + greenwich << 16 + source << 24 */
		double west, east, south, north; /* min/max extent in DEGREES */
		int area; /* Area of polygon in 1/10 km^2 */
		int areaFull, container, ancestor;

		std::vector<GshhsPoint *> lsPoints;

	protected:
		FILE *file;
		bool ok;
		bool greenwich;
		bool antarctic;
};

#endif
