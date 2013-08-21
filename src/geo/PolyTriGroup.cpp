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

#include "PolyTriGroup.h"
#include "geo/TriPrim.h"
#include <cstdlib>

namespace geo {

PolyTriGroup::PolyTriGroup()
{
	pn_vertex = NULL;             // pointer to array of poly vertex counts
	pgroup_geom = NULL;           // pointer to Raw geometry, used for contour line drawing
	tri_prim_head = NULL;         // head of linked list of TriPrims
	m_bSMSENC = false;

}

PolyTriGroup::~PolyTriGroup()
{
	free(pn_vertex);
	free(pgroup_geom);
	//Walk the list of TriPrims, deleting as we go
	TriPrim *tp_next;
	TriPrim *tp = tri_prim_head;
	while(tp)
	{
		tp_next = tp->p_next;
		delete tp;
		tp = tp_next;
	}
}

}

