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

#include "PolyTessGeoTrap.h"

#include <geo/triangulate.h>
#include <chart/geometry/PolyTrapGroup.h>
#include <chart/geometry/ExtendedGeometry.h>

#include <wx/geometry.h>

namespace chart {
namespace geometry {

// Build Trapezoidal PolyTessGeoTrap Object from ExtendedGeometry
PolyTessGeoTrap::PolyTessGeoTrap(ExtendedGeometry* pxGeom)
{
	m_bOK = false;

	m_ptg_head = new PolyTrapGroup(pxGeom);
	m_nvertex_max = pxGeom->n_max_vertex; // record the maximum number of segment vertices

	// All allocated buffers are owned now by the m_ptg_head
	// And will be freed on dtor of this object
	delete pxGeom; // FIXME: at this point, data of pxGeom will be freed, later again with 'delete
				   // m_ptg_head'
}

PolyTessGeoTrap::~PolyTessGeoTrap()
{
	delete m_ptg_head;
}

void PolyTessGeoTrap::BuildTess()
{
	// Flip the passed vertex array, contour-by-contour
	int offset = 1;
	for (int ict = 0; ict < m_ptg_head->nContours; ict++) {
		int nvertex = m_ptg_head->pn_vertex[ict];
		wxPoint2DDouble* pa = &m_ptg_head->ptrapgroup_geom[offset];
		wxPoint2DDouble* pb = &m_ptg_head->ptrapgroup_geom[(nvertex - 1) + offset];

		for (int iv = 0; iv < nvertex / 2; iv++) {

			wxPoint2DDouble a = *pa;
			*pa = *pb;
			*pb = a;

			pa++;
			pb--;
		}

		offset += nvertex;
	}

	itrap_t* itr;
	isegment_t* iseg;
	int n_traps;

	int trap_err
		= int_trapezate_polygon(m_ptg_head->nContours, m_ptg_head->pn_vertex,
								(double(*)[2])m_ptg_head->ptrapgroup_geom, &itr, &iseg, &n_traps);

	m_ptg_head->m_trap_error = trap_err;

	if (0 != n_traps) {
		// Now the Trapezoid Primitives

		// Iterate thru the trapezoid structure counting valid, non-empty traps

		int nvtrap = 0;
		for (int it = 1; it < n_traps; it++) {
			if (itr[it].inside == 1)
				nvtrap++;
		}

		m_ptg_head->ntrap_count = nvtrap;

		// Allocate enough memory
		m_ptg_head->trap_array = (trapz_t*)malloc(nvtrap * sizeof(trapz_t));

		// Iterate again and capture the valid trapezoids
		trapz_t* prtrap = m_ptg_head->trap_array;
		for (int i = 1; i < n_traps; i++) {
			if (itr[i].inside == 1) {

				// Fix up the trapezoid segment indices to account for ring closure points in the
				// input vertex array
				int i_adjust = 0;
				int ic = 0;
				int pcount = m_ptg_head->pn_vertex[0] - 1;
				while (itr[i].lseg > pcount) {
					i_adjust++;
					ic++;
					if (ic >= m_ptg_head->nContours)
						break;
					pcount += m_ptg_head->pn_vertex[ic] - 1;
				}
				prtrap->ilseg = itr[i].lseg + i_adjust;

				i_adjust = 0;
				ic = 0;
				pcount = m_ptg_head->pn_vertex[0] - 1;
				while (itr[i].rseg > pcount) {
					i_adjust++;
					ic++;
					if (ic >= m_ptg_head->nContours)
						break;
					pcount += m_ptg_head->pn_vertex[ic] - 1;
				}
				prtrap->irseg = itr[i].rseg + i_adjust;

				// Set the trap y values

				prtrap->hiy = itr[i].hi.y;
				prtrap->loy = itr[i].lo.y;

				prtrap++;
			}
		}
	} else {
		m_nvertex_max = 0;
	}

	// Free the trapezoid structure array
	free(itr);
	free(iseg);

	// Always OK, even if trapezator code faulted....
	// Contours should be OK, anyway, and    m_ptg_head->ntrap_count will be 0;

	m_bOK = true;
}

double PolyTessGeoTrap::Get_xmin() const
{
	return xmin;
}

double PolyTessGeoTrap::Get_xmax() const
{
	return xmax;
}

double PolyTessGeoTrap::Get_ymin() const
{
	return ymin;
}

double PolyTessGeoTrap::Get_ymax() const
{
	return ymax;
}

PolyTrapGroup* PolyTessGeoTrap::Get_PolyTrapGroup_head()
{
	return m_ptg_head;
}

int PolyTessGeoTrap::GetnVertexMax() const
{
	return m_nvertex_max;
}

bool PolyTessGeoTrap::IsOk() const
{
	return m_bOK;
}

}}

