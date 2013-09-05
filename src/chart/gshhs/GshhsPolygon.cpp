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

#include "GshhsPolygon.h"
#include "GshhsPoint.h"

int GshhsPolygon::readInt4()
{
	union {
		unsigned int n;
		unsigned char buf[4];
	} res;

	unsigned char in[4];

	int nb = 0;
	nb += fread( &in, 1, 4, file );
	res.buf[3] = in[0];
	res.buf[2] = in[1];
	res.buf[1] = in[2];
	res.buf[0] = in[3];

	if( nb != 4 ) {
		ok = false;
		res.n = 0;
	}

	return res.n;
}

int GshhsPolygon::readInt2()
{
	union {
		unsigned int n;
		unsigned char buf[4];
	} v;

	int nb = 0;
	nb += fread( &v.buf[0], 2, 1, file );
	if( nb != 2 ) {
		ok = false;
		v.n = 0;
	}
	return v.buf[1]<<8 | v.buf[0];
}

GshhsPolygon::GshhsPolygon( FILE *file_ )
{
	file = file_;
	ok = true;
	id = readInt4();
	n = readInt4();
	flag = readInt4();
	west = readInt4() * 1e-6;
	east = readInt4() * 1e-6;
	south = readInt4() * 1e-6;
	north = readInt4() * 1e-6;
	area = readInt4();

	if( ( ( flag >> 8 ) & 255 ) >= 7 ) { //GSHHS Release 2.0
		areaFull = readInt4();
		container = readInt4();
		ancestor = readInt4();

		greenwich = ( flag >> 16 ) & 1;
		antarctic = ( west == 0 && east == 360 );
		if( ok ) {
			double x = 0, y = 0;
			for( int i = 0; i < n; i++ ) {
				x = readInt4() * 1e-6;
				if( greenwich && x > 270 ) x -= 360;
				y = readInt4() * 1e-6;
				lsPoints.push_back( new GshhsPoint( x, y ) );
			}
			if( antarctic ) {
				lsPoints.insert( lsPoints.begin(), new GshhsPoint( 360, y ) );
				lsPoints.insert( lsPoints.begin(), new GshhsPoint( 360, -90 ) );
				lsPoints.push_back( new GshhsPoint( 0, -90 ) );
			}
		}
	} else {
		greenwich = ( flag >> 16 ) & 1;
		antarctic = ( west == 0 && east == 360 );
		if( ok ) {
			for( int i = 0; i < n; i++ ) {
				double x = 0, y = 0;
				x = readInt4() * 1e-6;
				if( greenwich && x > 270 ) x -= 360;
				y = readInt4() * 1e-6;
				lsPoints.push_back( new GshhsPoint( x, y ) );
			}
		}
	}
}

GshhsPolygon::~GshhsPolygon()
{
	std::vector<GshhsPoint *>::iterator itp;
	for( itp = lsPoints.begin(); itp != lsPoints.end(); itp++ ) {
		delete *itp;
		*itp = NULL;
	}
	lsPoints.clear();
}

