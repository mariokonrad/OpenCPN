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

#include "PolyTrapGroup.h"
#include <chart/geometry/ExtendedGeometry.h>
#include <cstdlib>

namespace chart {
namespace geometry {

PolyTrapGroup::PolyTrapGroup()
	: nContours(0)
	, pn_vertex(NULL)
	, ptrapgroup_geom(NULL)
	, ntrap_count(0)
	, trap_array(NULL)
	, m_trap_error(0)
{
}

PolyTrapGroup::PolyTrapGroup(ExtendedGeometry* pxGeom)
	: nContours(0)
	, pn_vertex(NULL)
	, ptrapgroup_geom(NULL)
	, ntrap_count(0)
	, trap_array(NULL)
	, m_trap_error(0)
{
	// transfering ownership

	nContours = pxGeom->n_contours;
	pn_vertex = pxGeom->contour_array;
	ptrapgroup_geom = pxGeom->vertex_array;

	pxGeom->contour_array = NULL;
	pxGeom->vertex_array = NULL;
}

PolyTrapGroup::~PolyTrapGroup()
{
	if (pn_vertex) {
		free(pn_vertex);
		pn_vertex = NULL;
	}
	if (ptrapgroup_geom) {
		free(ptrapgroup_geom);
		ptrapgroup_geom = NULL;
	}
	if (trap_array) {
		free(trap_array);
		trap_array = NULL;
	}
}

}}

