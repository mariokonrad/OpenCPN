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

#ifndef __CHART__GSHHS__GSHHSPOINT__H__
#define __CHART__GSHHS__GSHHSPOINT__H__

//==========================================================================
// GSHHS file format:
//
// int id;           /* Unique polygon id number, starting at 0 */
// int n;            /* Number of points in this polygon */
// int flag;             /* level + version << 8 + greenwich << 16 + source << 24 */
// int west, east, south, north; /* min/max extent in micro-degrees */
// int area;             /* Area of polygon in 1/10 km^2 */
//
// Here, level, version, greenwhich, and source are
// level:   1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
// version: Set to 4 for GSHHS version 1.4
// greenwich:   1 if Greenwich is crossed
// source:  0 = CIA WDBII, 1 = WVS
//==========================================================


class GshhsPoint
{
	public:
		double lon;
		double lat;

	public:
		GshhsPoint(double lon, double lat)
			: lon(lon)
			, lat(lat)
		{}
};

#endif
