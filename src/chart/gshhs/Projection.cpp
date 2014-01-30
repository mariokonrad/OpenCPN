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

#include "Projection.h"
#include <windows/compatibility.h>
#include <cmath>

#ifndef M_PI_2
#define M_PI_2  (M_PI / 2.0)
#endif
#ifndef M_PI_4
#define M_PI_4  (M_PI / 4.0)
#endif

Projection::Projection()
	: W(0)
	, H(0)
	, CX(0)
	, CY(0)
	, xW(-90)
	, xE(90)
	, yN(90)
	, yS(-90)
	, PX(0)
	, PY(0)
{
	frozen = false;
	scalemax = 10e20;
	scale = -1;
	useTempo = true;
}

Projection::Projection(int w, int h, double cx, double cy)
	: W(0)
	, H(0)
	, CX(0)
	, CY(0)
	, xW(-90)
	, xE(90)
	, yN(90)
	, yS(-90)
	, PX(0)
	, PY(0)
{
	frozen = false;
	scalemax = 10e20;
	scale = -1;
	SetScreenSize(w, h);
	SetCenterInMap(cx, cy);
	useTempo = true;
}

double Projection::degToRad(double d) const
{
	return d * M_PI / 180;
}

double Projection::radToDeg(double r) const
{
	return r * 180 / M_PI;
}

void Projection::SetScale(double sc)
{
	scale = sc;
	if (scale < scaleall)
		scale = scaleall;
	if (scale > scalemax)
		scale = scalemax;
	updateBoundaries();
}

void Projection::SetCenterInMap(double x, double y)
{
	while (x > 180.0) {
		x -= 360.0;
	}
	while (x < -180.0) {
		x += 360.0;
	}
	CX = x;
	CY = y;

	// compute projection
	PX = CX;
	PY = radToDeg(log(tan(degToRad(CY) / 2 + M_PI_4)));

	updateBoundaries();
}

void Projection::SetScreenSize(int w, int h)
{
	W = w;
	H = h;

	// compute scaleall

	double sx = W / 360.0;
	double sy1 = log(tan(degToRad(84) / 2 + M_PI_4));
	double sy2 = log(tan(degToRad(-80) / 2 + M_PI_4));
	double sy = H / fabs(radToDeg(sy1 - sy2));
	scaleall = (sy < sx) ? sy : sx;

	double sX = W / fabs(xE - xW);
	double sYN = log(tan(degToRad(yN) / 2 + M_PI_4));
	double sYS = log(tan(degToRad(yS) / 2 + M_PI_4));
	double sY = H / fabs(radToDeg(sYN - sYS));

	if (scale < scaleall) {
		scale = scaleall;
	}

	updateBoundaries();
}

void Projection::updateBoundaries()
{
	// lat
	yS = radToDeg(2 * atan(exp((double)(degToRad(PY - H / (2 * scale))))) - M_PI_2);
	yN = radToDeg(2 * atan(exp((double)(degToRad(PY + H / (2 * scale))))) - M_PI_2);

	// lon
	xW = PX - W / (2 * scale);
	xE = PX + W / (2 * scale);

	// xW and yN => upper corner

	if ((getW() * getH()) != 0)
		coefremp = 10000.0 * fabs(((xE - xW) * (yN - yS)) / (getW() * getH()));
	else
		coefremp = 10000.0;
}

bool Projection::isInBounderies(int x, int y) const
{
	return (x >= 0 && y >= 0 && x <= W && y <= H);
}

void Projection::map2screen(double x, double y, int* i, int* j) const
{
	if (y <= -90)
		y = -89.9;
	if (y >= 90)
		y = 89.9;

	*i = round(scale * (x - xW));
	*j = H / 2 + round(scale * (PY - radToDeg(log(tan(degToRad(y) / 2 + M_PI_4)))));
}

void Projection::map2screenDouble(double x, double y, double* i, double* j) const
{
	if (y <= -90)
		y = -89.9999999999999999999999999999999999999999999999999999999;
	if (y >= 90)
		y = 89.999999999999999999999999999999999999999999999999999999999;
	double diff = x - xW;
	*i = scale * diff;
	double trick = PY - radToDeg(log(tan(degToRad(y) / (double)2.0 + M_PI_4)));
	*j = ((double)H / (double)2.0 + (scale * trick));
}

bool Projection::intersect(double w, double e, double s, double n) const
{
	return !(w > xE || e < xW || s > yN || n < yS);
}

