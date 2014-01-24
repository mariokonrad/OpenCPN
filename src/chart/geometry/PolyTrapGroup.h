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

#ifndef __CHART__GEOMETRY__POLYTRAPGROUP__H__
#define __CHART__GEOMETRY__POLYTRAPGROUP__H__

class wxPoint2DDouble;

namespace chart {
namespace geometry {

class ExtendedGeometry;

struct trapz_t
{
	int ilseg;
	int irseg;
	double loy;
	double hiy;
};

/// Used for describing/rendering tesselated polygons
class PolyTrapGroup
{
public:
	PolyTrapGroup();
	PolyTrapGroup(ExtendedGeometry* pxGeom);
	~PolyTrapGroup();

	int nContours;
	int* pn_vertex; // pointer to array of poly vertex counts
	wxPoint2DDouble* ptrapgroup_geom; // pointer to Raw geometry, used for contour line drawing

	int ntrap_count;
	trapz_t* trap_array; // pointer to trapz_t array
	int m_trap_error;
};

}}

#endif
