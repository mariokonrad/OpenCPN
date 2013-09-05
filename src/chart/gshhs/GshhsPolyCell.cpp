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

#include "GshhsPolyCell.h"
#include <chart/gshhs/Projection.h>
#include <chart/gshhs/PolygonFileHeader.h>
#include <ocpnDC.h>
#include "gshhs.h"

#define GSHHS_SCL    1.0e-6    /* Convert micro-degrees to degrees */

GshhsPolyCell::GshhsPolyCell(
		FILE * fpoly_,
		int x0_,
		int y0_,
		Projection * proj_,
		PolygonFileHeader * header_)
{
	proj = proj_;
	header = header_;
	fpoly = fpoly_;
	x0cell = x0_;
	y0cell = y0_;

	ReadPolygonFile(fpoly, x0cell, y0cell, header->pasx, header->pasy, &poly1, &poly2, &poly3, &poly4, &poly5);

	// FIXME: what good is this for?
	int cnt = 0;
	for( unsigned int i = 0; i < poly1.size(); i++ )
		cnt += poly1.at( i ).size();
}

GshhsPolyCell::~GshhsPolyCell()
{}

// FIXME: move this into a function/method
#define READ_POLY(POLY) { \
	double X,Y; \
	contour tmp_contour; \
	int num_vertices,num_contours; \
	int value; \
	POLY->clear(); \
	fread(&(num_contours), sizeof(int), 1, polyfile); \
	for (int c= 0; c < num_contours; c++) \
	{ \
		fread(&(value), sizeof(int), 1, polyfile); /* discarding hole value */ \
		fread(&(value), sizeof(int), 1, polyfile); \
		num_vertices=value; \
		tmp_contour.clear(); \
		for (int v= 0; v < num_vertices; v++) \
		{ \
			fread(&(X), sizeof(double), 1, polyfile); \
			fread(&(Y), sizeof(double), 1, polyfile); \
			tmp_contour.push_back(wxRealPoint(X*GSHHS_SCL,Y*GSHHS_SCL)); \
		} \
		POLY->push_back(tmp_contour); \
	} \
}

void GshhsPolyCell::ReadPolygonFile( FILE *polyfile, int x, int y, int pas_x, int pas_y,
		contour_list *p1, contour_list *p2, contour_list *p3, contour_list *p4, contour_list *p5 )
{
	int pos_data;
	int tab_data;

	tab_data = ( x / pas_x ) * ( 180 / pas_y ) + ( y + 90 ) / pas_y;
	fseek( polyfile, sizeof(PolygonFileHeader) + tab_data * sizeof(int), SEEK_SET );
	fread( &pos_data, sizeof(int), 1, polyfile );

	fseek( polyfile, pos_data, SEEK_SET );

	READ_POLY( p1 )
		READ_POLY( p2 )
		READ_POLY( p3 )
		READ_POLY( p4 )
		READ_POLY( p5 )
}

void GshhsPolyCell::DrawPolygonFilled( ocpnDC &pnt, contour_list * p, double dx, Projection *proj,
		wxColor color )
{
	int x, y;
	unsigned int c, v;
	int pointCount;
	wxPoint* poly_pt;

	wxPen myPen = wxNullPen;
	pnt.SetPen( myPen );
	pnt.SetBrush( color );

	int x_old = 0;
	int y_old = 0;

	for( c = 0; c < p->size(); c++ ) {
		if( !p->at( c ).size() ) continue;

		poly_pt = new wxPoint[ p->at(c).size() ];
		pointCount = 0;

		for( v = 0; v < p->at( c ).size(); v++ ) {
			proj->map2screen( p->at( c ).at( v ).x + dx, p->at( c ).at( v ).y, &x, &y );

			if( v == 0 || x != x_old || y != y_old ) {
				poly_pt[pointCount].x = x;
				poly_pt[pointCount].y = y;
				pointCount++;
				x_old = x;
				y_old = y;
			}
		}

		if(pointCount>1)
			pnt.DrawPolygonTessellated( pointCount, poly_pt, 0, 0 );
		delete[] poly_pt;
	}
}

#define DRAW_POLY_FILLED(POLY,COL) if(POLY) DrawPolygonFilled(pnt,POLY,dx,proj,COL);

void GshhsPolyCell::drawMapPlain( ocpnDC &pnt, double dx, Projection *proj, wxColor seaColor,
		wxColor landColor )
{
	DRAW_POLY_FILLED( &poly1, landColor )
	DRAW_POLY_FILLED( &poly2, seaColor )
	DRAW_POLY_FILLED( &poly3, landColor )
	DRAW_POLY_FILLED( &poly4, seaColor )
	DRAW_POLY_FILLED( &poly5, landColor )
}

void GshhsPolyCell::DrawPolygonContour( ocpnDC &pnt, contour_list * p, double dx, Projection *proj )
{
	double x1, y1, x2, y2;
	double long_max, lat_max, long_min, lat_min;

	long_min = (double) x0cell;
	lat_min = (double) y0cell;
	long_max = ( (double) x0cell + (double) header->pasx );
	lat_max = ( (double) y0cell + (double) header->pasy );

	//qWarning()  << long_min << "," << lat_min << long_max << "," << lat_max;

	for( unsigned int i = 0; i < p->size(); i++ ) {
		if( !p->at( i ).size() ) continue;
		unsigned int v;
		for( v = 0; v < ( p->at( i ).size() - 1 ); v++ ) {
			x1 = p->at( i ).at( v ).x;
			y1 = p->at( i ).at( v ).y;
			x2 = p->at( i ).at( v + 1 ).x;
			y2 = p->at( i ).at( v + 1 ).y;

			// Elimination des traits verticaux et horizontaux
			if( ( ( ( x1 == x2 ) && ( ( x1 == long_min ) || ( x1 == long_max ) ) )
						|| ( ( y1 == y2 ) && ( ( y1 == lat_min ) || ( y1 == lat_max ) ) ) ) == 0 ) {
				int a, b, c, d;
				double A, B, C, D;
				proj->map2screen( x1 + dx, y1, &a, &b );
				proj->map2screen( x2 + dx, y2, &c, &d );
				proj->map2screenDouble( x1 + dx, y1, &A, &B );
				proj->map2screenDouble( x2 + dx, y2, &C, &D );
				if( a != c || b != d ) pnt.DrawLine( a, b, c, d );
				if( A != C || B != D ) {
					if( proj->isInBounderies( a, b ) || proj->isInBounderies( c, d ) )
						coasts.push_back( QLineF( A, B, C, D ) );
				}
			}
		}

		x1 = p->at( i ).at( v ).x;
		y1 = p->at( i ).at( v ).y;
		x2 = p->at( i ).at( 0 ).x;
		y2 = p->at( i ).at( 0 ).y;

		if( ( ( ( x1 == x2 ) && ( ( x1 == long_min ) || ( x1 == long_max ) ) )
					|| ( ( y1 == y2 ) && ( ( y1 == lat_min ) || ( y1 == lat_max ) ) ) ) == 0 ) {
			int a, b, c, d;
			double A, B, C, D;
			proj->map2screen( x1 + dx, y1, &a, &b );
			proj->map2screen( x2 + dx, y2, &c, &d );
			proj->map2screenDouble( x1 + dx, y1, &A, &B );
			proj->map2screenDouble( x2 + dx, y2, &C, &D );
			if( a != c || b != d ) pnt.DrawLine( a, b, c, d );
			if( A != C || B != D ) {
				if( proj->isInBounderies( a, b ) || proj->isInBounderies( c, d ) )
					coasts.push_back( QLineF( A, B, C, D ) );
			}
		}
	}
}

#define DRAW_POLY_CONTOUR(POLY) if(POLY) DrawPolygonContour(pnt,POLY,dx,proj);

void GshhsPolyCell::drawSeaBorderLines( ocpnDC &pnt, double dx, Projection *proj )
{
	coasts.clear();
	DRAW_POLY_CONTOUR( &poly1 )
	DRAW_POLY_CONTOUR( &poly2 )
	DRAW_POLY_CONTOUR( &poly3 )
	DRAW_POLY_CONTOUR( &poly4 )
	DRAW_POLY_CONTOUR( &poly5 )
}

