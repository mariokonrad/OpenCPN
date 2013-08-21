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
#include <geo/ExtendedGeometry.h>
#include <cstdlib>

namespace geo {

PolyTrapGroup::PolyTrapGroup()
{
	pn_vertex = NULL;             // pointer to array of poly vertex counts
	ptrapgroup_geom = NULL;           // pointer to Raw geometry, used for contour line drawing
	trap_array = NULL;            // pointer to trapz_t array

	ntrap_count = 0;
}

PolyTrapGroup::PolyTrapGroup(ExtendedGeometry *pxGeom)
{
	m_trap_error = 0;

	nContours = pxGeom->n_contours;

	pn_vertex = pxGeom->contour_array;             // pointer to array of poly vertex counts
	pxGeom->contour_array = NULL;

	ptrapgroup_geom = pxGeom->vertex_array;
	pxGeom->vertex_array = NULL;

	ntrap_count = 0;                                // provisional
	trap_array = NULL;                              // pointer to generated trapz_t array
}

PolyTrapGroup::~PolyTrapGroup()
{
	free(pn_vertex);
	free(ptrapgroup_geom);
	free(trap_array);
}

}

