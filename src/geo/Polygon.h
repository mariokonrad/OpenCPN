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

#ifndef __GEO__POLYGON__H__
#define __GEO__POLYGON__H__

namespace geo {

struct PointD
{
	PointD(double x = 0.0, double y = 0.0)
		: x(x)
		, y(y)
	{}

	double x;
	double y;
};

/// TODO: is the reversed order y/x really intentional?
struct PointF
{
	PointF(float x = 0.0f, float y = 0.0f)
		: y(y)
		, x(x)
	{}

	float y;
	float x;
};

class Polygon
{
public:
	static bool inside(const PointD*, int, const PointD&);
	static bool insidef(const PointF*, int, const PointF&); // FIXME: really necessary?

private:
	static bool intersect(const PointD& p1, const PointD& p2, const PointD& p3, const PointD& p4);
	static int ccw(const PointD&, const PointD&, const PointD&);

	// FIXME: those two really necessary?
	static bool intersectf(const PointF& p1, const PointF& p2, const PointF& p3, const PointF& p4);
	static int ccwf(const PointF&, const PointF&, const PointF&);
};

}

#endif
