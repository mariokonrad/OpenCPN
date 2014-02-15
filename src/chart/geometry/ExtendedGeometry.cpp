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

#include "ExtendedGeometry.h"
#include <cstring>
#include <cstdlib>

namespace chart {
namespace geometry {

ExtendedGeometry::ExtendedGeometry()
	: pogrGeom(NULL)
	, n_vector_indices(0)
	, pvector_index(NULL)
	, n_contours(0)
	, contour_array(NULL)
	, n_max_vertex(0)
	, pointx(0)
	, pointy(0)
	, vertex_array(NULL)
	, xmin(0)
	, xmax(0)
	, ymin(0)
	, ymax(0)
	, n_max_edge_points(0)
	, x_rate(0.0)
	, x_offset(0.0)
	, y_rate(0.0)
	, y_offset(0.0)
{
}

ExtendedGeometry::~ExtendedGeometry()
{
	if (vertex_array) {
		free(vertex_array);
		vertex_array = NULL;
	}
	delete [] contour_array;
	contour_array = NULL;
}

void ExtendedGeometry::set_contour_array(const int* data, int n)
{
	// FIXME: check if the memory is already allocated

	n_contours = n;
	contour_array = new int[n];
	memcpy(contour_array, data, n * sizeof(int));
}

}}

