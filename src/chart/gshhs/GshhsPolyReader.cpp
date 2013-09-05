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

#include "GshhsPolyReader.h"
#include "GshhsPolyCell.h"
#include "Projection.h"
#include "GshhsReader.h"

#include <wx/log.h>

#define INTER_MAX_LIMIT 1.0000001
#define INTER_MIN_LIMIT -0.0000001

GshhsPolyReader::GshhsPolyReader( int quality )
{
	fpoly = NULL;

	for( int i = 0; i < 360; i++ ) {
		for( int j = 0; j < 180; j++ ) {
			allCells[i][j] = NULL;
		}
	}
	currentQuality = -1;
	InitializeLoadQuality( quality );
	this->abortRequested = false;
}

//-------------------------------------------------------------------------
GshhsPolyReader::~GshhsPolyReader()
{
	for( int i = 0; i < 360; i++ ) {
		for( int j = 0; j < 180; j++ ) {
			if( allCells[i][j] != NULL ) {
				delete allCells[i][j];
				allCells[i][j] = NULL;
			}
		}
	}
}

//-------------------------------------------------------------------------
int GshhsPolyReader::ReadPolyVersion()
{
	char txtn = 'c';
	wxString fname = GshhsReader::getFileName_Land( 0 );
	if( fpoly ) fclose( fpoly );
	fpoly = fopen( fname.mb_str(), "rb" );

	/* init header */
	if( !fpoly ) return 0;

	readPolygonFileHeader( fpoly, &polyHeader );

	return polyHeader.version;
}

void GshhsPolyReader::InitializeLoadQuality( int quality )  // 5 levels: 0=low ... 4=full
{
	if( currentQuality != quality ) {
		currentQuality = quality;

		wxString fname = GshhsReader::getFileName_Land( quality );

		if( fpoly ) fclose( fpoly );

		fpoly = fopen( fname.mb_str(), "rb" );
		if( fpoly ) readPolygonFileHeader( fpoly, &polyHeader );

		for( int i = 0; i < 360; i++ ) {
			for( int j = 0; j < 180; j++ ) {
				if( allCells[i][j] != NULL ) {
					delete allCells[i][j];
					allCells[i][j] = NULL;
				}
			}
		}
	}
}

bool GshhsPolyReader::crossing1( QLineF trajectWorld )
{
	if( !proj || proj == NULL ) return false;

	int cxmin, cxmax, cymax, cymin;
	cxmin = (int) floor( wxMin( trajectWorld.p1().x, trajectWorld.p2().x ) );
	cxmax = (int) ceil( wxMax( trajectWorld.p1().x, trajectWorld.p2().x ) );

	if(cxmin < 0) {
		cxmin += 360;
		cxmax += 360;
	}

	if(cxmax - cxmin > 180) { /* dont go long way around world */
		cxmin = (int) floor( wxMax( trajectWorld.p1().x, trajectWorld.p2().x ) ) - 360;
		cxmax = (int) ceil( wxMin( trajectWorld.p1().x, trajectWorld.p2().x ) );
	}

	cymin = (int) floor( wxMin( trajectWorld.p1().y, trajectWorld.p2().y ) );
	cymax = (int) ceil( wxMax( trajectWorld.p1().y, trajectWorld.p2().y ) );
	int cx, cxx, cy;
	GshhsPolyCell *cel;

	for( cx = cxmin; cx < cxmax; cx++ ) {
		cxx = cx;
		while( cxx < 0 )
			cxx += 360;
		while( cxx >= 360 )
			cxx -= 360;

		double p1x=trajectWorld.p1().x, p2x = trajectWorld.p2().x;
		if(cxx < 180) {
			if(p1x > 180) p1x -= 360;
			if(p2x > 180) p2x -= 360;
		} else {
			if(p1x < 180) p1x += 360;
			if(p2x < 180) p2x += 360;
		}

		QLineF rtrajectWorld(p1x, trajectWorld.p1().y, p2x, trajectWorld.p2().y);                

		for( cy = cymin; cy < cymax; cy++ ) {
			if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
				if( allCells[cxx][cy + 90] == NULL ) {
					cel = new GshhsPolyCell( fpoly, cxx, cy, proj, &polyHeader );
					assert( cel );
					allCells[cxx][cy + 90] = cel;
				} else
					cel = allCells[cxx][cy + 90];

				contour_list &poly1 = cel->getPoly1();
				for( unsigned int pi = 0; pi < poly1.size(); pi++ ) {
					contour c = poly1[pi];
					double lx = c[c.size()-1].x, ly = c[c.size()-1].y;
					for( unsigned int pj = 0; pj < c.size(); pj++ ) {
						QLineF l(lx, ly, c[pj].x, c[pj].y);
						if( my_intersects( rtrajectWorld, l ) )
							return true;
						lx = c[pj].x, ly = c[pj].y;
					}
				}
			}
		}
	}
	return false;
}

void GshhsPolyReader::readPolygonFileHeader( FILE *polyfile, PolygonFileHeader *header )
{
	//    int FReadResult = 0;

	fseek( polyfile, 0, SEEK_SET );
	fread( header, sizeof(PolygonFileHeader), 1, polyfile );
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapPlain( ocpnDC &pnt, Projection *proj, wxColor seaColor,
		wxColor landColor )
{
	if( !fpoly ) return;

	int cxmin, cxmax, cymax, cymin;  // cellules visibles
	cxmin = (int) floor( proj->getXmin() );
	cxmax = (int) ceil( proj->getXmax() );
	cymin = (int) floor( proj->getYmin() );
	cymax = (int) ceil( proj->getYmax() );
	int dx, cx, cxx, cy;
	GshhsPolyCell *cel;

	for( cx = cxmin; cx < cxmax; cx++ ) {
		cxx = cx;
		while( cxx < 0 )
			cxx += 360;
		while( cxx >= 360 )
			cxx -= 360;

		for( cy = cymin; cy < cymax; cy++ ) {
			if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
				if( allCells[cxx][cy + 90] == NULL ) {
					cel = new GshhsPolyCell( fpoly, cxx, cy, proj, &polyHeader );
					assert( cel );
					allCells[cxx][cy + 90] = cel;
				} else {
					cel = allCells[cxx][cy + 90];
				}
				dx = cx - cxx;
				cel->drawMapPlain( pnt, dx, proj, seaColor, landColor );
			}
		}
	}
}

//-------------------------------------------------------------------------
void GshhsPolyReader::drawGshhsPolyMapSeaBorders( ocpnDC &pnt, Projection *proj )
{
	if( !fpoly ) return;
	this->abortRequested = true;
	int cxmin, cxmax, cymax, cymin;  // cellules visibles
	cxmin = (int) floor( proj->getXmin() );
	cxmax = (int) ceil( proj->getXmax() );
	cymin = (int) floor( proj->getYmin() );
	cymax = (int) ceil( proj->getYmax() );
	int dx, cx, cxx, cy;
	GshhsPolyCell *cel;

	for( cx = cxmin; cx < cxmax; cx++ ) {
		cxx = cx;
		while( cxx < 0 )
			cxx += 360;
		while( cxx >= 360 )
			cxx -= 360;

		for( cy = cymin; cy < cymax; cy++ ) {
			if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
				if( allCells[cxx][cy + 90] == NULL ) {
					cel = new GshhsPolyCell( fpoly, cxx, cy, proj, &polyHeader );
					assert( cel );
					allCells[cxx][cy + 90] = cel;
				} else {
					cel = allCells[cxx][cy + 90];
				}
				dx = cx - cxx;
				cel->drawSeaBorderLines( pnt, dx, proj );
			}
		}
	}
	this->abortRequested = false;
}


bool GshhsPolyReader::crossing( QLineF traject, QLineF trajectWorld ) const
{
	if( !proj || proj == NULL ) return false;
	if( !proj->isInBounderies( traject.p1().x, traject.p1().y )
			&& !proj->isInBounderies( traject.p2().x, traject.p2().y ) ) return false;
	//wxRealPoint dummy;
	int cxmin, cxmax, cymax, cymin;
	cxmin = (int) floor( wxMin( trajectWorld.p1().x, trajectWorld.p2().x ) );
	cxmax = (int) ceil( wxMax( trajectWorld.p1().x, trajectWorld.p2().x ) );
	cymin = (int) floor( wxMin( trajectWorld.p1().y, trajectWorld.p2().y ) );
	cymax = (int) ceil( wxMax( trajectWorld.p1().y, trajectWorld.p2().y ) );
	int cx, cxx, cy;
	GshhsPolyCell *cel;

	for( cx = cxmin; cx < cxmax; cx++ ) {
		cxx = cx;
		while( cxx < 0 )
			cxx += 360;
		while( cxx >= 360 )
			cxx -= 360;

		for( cy = cymin; cy < cymax; cy++ ) {
			if( this->abortRequested ) return false;
			if( cxx >= 0 && cxx <= 359 && cy >= -90 && cy <= 89 ) {
				if( this->abortRequested ) return false;
				if( allCells[cxx][cy + 90] == NULL ) continue;
				cel = allCells[cxx][cy + 90];
				std::vector < QLineF > *coasts = cel->getCoasts();
				if( coasts->empty() ) continue;
				for( unsigned int cs = 0; cs < coasts->size(); cs++ ) {
					if( this->abortRequested ) {
						return false;
					}
					if( my_intersects( traject, coasts->at( cs ) ) )
						return true;
				}
			}
		}
	}
	return false;
}

bool GshhsPolyReader::my_intersects( QLineF line1, QLineF line2 ) const
{
	// implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
	wxRealPoint a = line1.p2() - line1.p1();
	wxRealPoint b = line2.p1() - line2.p2();
	wxRealPoint c = line1.p1() - line2.p1();

	const double denominator = a.y * b.x - a.x * b.y;
	if( denominator == 0 ) return false;

	const double reciprocal = 1 / denominator;
	const double na = ( b.y * c.x - b.x * c.y ) * reciprocal;

	if( na < INTER_MIN_LIMIT || na > INTER_MAX_LIMIT ) return false;

	const double nb = ( a.x * c.y - a.y * c.x ) * reciprocal;
	if( nb < INTER_MIN_LIMIT || nb > INTER_MAX_LIMIT ) return false;

	return true;
}

