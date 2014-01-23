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

#ifndef __GEO__EXTENDEDGEOMETRY__H__
#define __GEO__EXTENDEDGEOMETRY__H__

class wxPoint2DDouble;
class OGRGeometry;

namespace geo {

class ExtendedGeometry
{
public:
	ExtendedGeometry();
	~ExtendedGeometry();

	void set_contour_array(const int* data, int n);

	OGRGeometry* pogrGeom;
	int n_vector_indices;
	int* pvector_index;
	int n_contours;
	int* contour_array; // FIXME: use std::vector
	int n_max_vertex;
	int pointx;
	int pointy;
	wxPoint2DDouble* vertex_array;
	int xmin;
	int xmax;
	int ymin;
	int ymax;
	int n_max_edge_points;

	// Conversion parameters
	// for (assummed linear) convertions from vertex_array points to easting/northing, metres from
	// 0,0
	// To convert to lat/lon, use simple merctor equations
	double x_rate;
	double x_offset;
	double y_rate;
	double y_offset;
};

}

#endif
