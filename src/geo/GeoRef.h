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
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 **************************************************************************/

#ifndef __GEO__GEOREF_H__
#define __GEO__GEOREF_H__

#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctype.h>

#include <geo/Position.h>

namespace geo {

struct GeoRef // FIXME: memory allocation outside GeoRef (using malloc), use std containers instead
{
	int status;
	int count;
	int order;
	double* tx;
	double* ty;
	double* lon;
	double* lat;
	double* pwx;
	double* pwy;
	double* wpx;
	double* wpy;
	int txmax;
	int tymax;
	int txmin;
	int tymin;
	double lonmax;
	double lonmin;
	double latmax;
	double latmin;
};

#define DATUM_INDEX_WGS84     100
#define DATUM_INDEX_UNKNOWN   -1

const double WGS84_semimajor_axis_meters = 6378137.0;     // WGS84 semimajor axis
const double mercator_k0                 = 0.9996;

void toTM(float lat, float lon, float lat0, float lon0, double* x, double* y);
Position fromTM(double x, double y, double lat0, double lon0);

void toSM(double lat, double lon, double lat0, double lon0, double* x, double* y);
void fromSM(double x, double y, double lat0, double lon0, double* lat, double* lon);

void toSM_ECC(double lat, double lon, double lat0, double lon0, double* x, double* y);
void fromSM_ECC(double x, double y, double lat0, double lon0, double* lat, double* lon);

void toPOLY(double lat, double lon, double lat0, double lon0, double* x, double* y);
Position fromPOLY(double x, double y, double lat0, double lon0);

/// distance in nautical miles
Position ll_gc_ll(const Position& pos, double crs, double dist);
void ll_gc_ll_reverse(const Position& pos1, const Position& pos2, double* bearing, double* dist);

Position PositionBearingDistanceMercator(const Position& pos, double brg, double dist);
double DistGreatCircle(const Position& start, const Position& destination);

int GetDatumIndex(const char* str);
void MolodenskyTransform(const Position& pos, Position& to, int from_datum_index,
						 int to_datum_index);

void DistanceBearingMercator(const Position& pos0, const Position& pos1, double* brg, double* dist);

int Georef_Calculate_Coefficients(struct GeoRef* cp, int nlin_lon);
int Georef_Calculate_Coefficients_Proj(struct GeoRef* cp);

}

#endif
