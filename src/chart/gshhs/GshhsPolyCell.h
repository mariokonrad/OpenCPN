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

#ifndef __CHART__GSHHS__GSHHSPOLYCELL__H__
#define __CHART__GSHHS__GSHHSPOLYCELL__H__

#include <chart/gshhs/QLineF.h>
#include <vector>
#include <cstdio>
#include <wx/colour.h>

class Projection;
class ocpnDC;
struct PolygonFileHeader;

typedef std::vector<wxRealPoint> contour;
typedef std::vector<contour> contour_list;

class GshhsPolyCell
{
	public:

		GshhsPolyCell( FILE *fpoly, int x0, int y0, Projection *proj, PolygonFileHeader *header );
		~GshhsPolyCell();

		void drawMapPlain( ocpnDC &pnt, double dx, Projection *proj, wxColor seaColor,
				wxColor landColor );

		void drawSeaBorderLines( ocpnDC &pnt, double dx, Projection *proj );
		std::vector<QLineF> * getCoasts() { return &coasts; }
		contour_list &getPoly1() { return poly1; }

	private:
		int nbpoints;
		int x0cell, y0cell;

		FILE *fpoly;

		std::vector<QLineF> coasts;
		Projection *proj;
		PolygonFileHeader *header;
		contour_list poly1, poly2, poly3, poly4, poly5;

		void DrawPolygonFilled( ocpnDC &pnt, contour_list * poly, double dx, Projection *proj,
				wxColor color );
		void DrawPolygonContour( ocpnDC &pnt, contour_list * poly, double dx, Projection *proj );

		void ReadPolygonFile( FILE *polyfile, int x, int y, int pas_x, int pas_y, contour_list *p1,
				contour_list *p2, contour_list *p3, contour_list *p4, contour_list *p5 );

};

#endif
