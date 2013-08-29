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

#ifndef __CHART_CM93GEOMETRY__H__
#define __CHART_CM93GEOMETRY__H__

// This constant was developed empirically by looking at a
// representative cell, comparing the cm93 point transform coefficients
// to the stated lat/lon bounding box.
// This value corresponds to the semi-major axis for the "International 1924" geo-standard
// For WGS84, it should be 6378137.0......
static const double CM93_semimajor_axis_meters = 6378388.0; // CM93 semimajor axis

struct cm93_point
{
	unsigned short x;
	unsigned short y;
};

struct cm93_point_3d
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
};

struct geometry_descriptor
{
	unsigned short n_points;
	unsigned short x_min;
	unsigned short y_min;
	unsigned short x_max;
	unsigned short y_max;
	int index;
	cm93_point * p_points;
};

#endif
