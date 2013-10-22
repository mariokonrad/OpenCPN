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

#ifndef __GEO__TRIPRIM__H__
#define __GEO__TRIPRIM__H__

namespace geo {

class BoundingBox;

/// Used for describing/rendering tesselated polygons
class TriPrim
{
	public:
		//  nota bene  These definitions are identical to OpenGL prototypes
		enum Type
		{
			PTG_TRIANGLES      = 0x0004,
			PTG_TRIANGLE_STRIP = 0x0005,
			PTG_TRIANGLE_FAN   = 0x0006
		};

	public:
		TriPrim();
		~TriPrim();

		Type type;
		int nVert;
		double * p_vertex; //  Pointer to vertex array, x,y,x,y.....
		BoundingBox * p_bbox;
		TriPrim * p_next; // chain link
};

}

#endif
