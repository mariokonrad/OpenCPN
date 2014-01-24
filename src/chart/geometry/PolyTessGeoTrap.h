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

#ifndef __CHART__GEOMETRY__POLYTESSGEOTRAP__H__
#define __CHART__GEOMETRY__POLYTESSGEOTRAP__H__

namespace chart {
namespace geometry {

class ExtendedGeometry;
class PolyTrapGroup;

/// Trapezoid Tesselator
class PolyTessGeoTrap
{
public:
	PolyTessGeoTrap();
	~PolyTessGeoTrap();

	PolyTessGeoTrap(ExtendedGeometry* pxGeom); // Build this from Extended Geometry

	void BuildTess();

	double Get_xmin() const;
	double Get_xmax() const;
	double Get_ymin() const;
	double Get_ymax() const;
	PolyTrapGroup* Get_PolyTrapGroup_head();
	int GetnVertexMax() const;
	bool IsOk() const;

	int ErrorCode;

private:
	bool m_bOK;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	PolyTrapGroup* m_ptg_head; // PolyTrapGroup
	int m_nvertex_max; // computed max vertex count used by drawing primitives as optimization for
					   // malloc
	int m_ncnt;
	int m_nwkb;
};

}}

#endif
