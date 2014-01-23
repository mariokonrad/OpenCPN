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

#ifndef __GEO__POLYTRIGROUP__H__
#define __GEO__POLYTRIGROUP__H__

namespace geo {

class ExtendedGeometry;
class TriPrim;

/// Used for describing/rendering tesselated polygons
class PolyTriGroup
{
public:
	PolyTriGroup();
	PolyTriGroup(ExtendedGeometry* pxGeom);
	~PolyTriGroup();

	int nContours; // pointer to array of poly vertex counts
	int* pn_vertex; // pointer to array of poly vertex counts
	float* pgroup_geom; // pointer to Raw geometry, used for contour line drawing

	TriPrim* tri_prim_head; // head of linked list of TriPrims
	bool m_bSMSENC;

private:
	int my_bufgets(char* buf, int buf_len_max);
};

}

#endif
