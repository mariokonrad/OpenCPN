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

#include "Polygon.h"
#include <cmath>

namespace geo {

/// This routine determines if the point passed is in the polygon. It uses
/// the classical polygon hit-testing algorithm: a horizontal ray starting
/// at the point is extended infinitely rightwards and the number of
/// polygon edges that intersect the ray are counted. If the number is odd,
/// the point is inside the polygon.
///
/// Polygon is assumed OPEN, not CLOSED.
///
/// @retval true The point is inside the polygon.
/// @retval false The point is outside the polygon..
bool Polygon::inside(const PointD* polygon, int polygon_size, const PointD& point)
{
	int num_intersection = 0;

	PointD p = point;
	p.x = 1.e8;

	// Now go through each of the lines in the polygon and see if it intersects
	for (int i = 0; i < polygon_size - 1; ++i) {
		if (intersect(point, p, polygon[i], polygon[i + 1]))
			++num_intersection;
	}

	// And the last line
	if (intersect(point, p, polygon[polygon_size - 1], polygon[0]))
		++num_intersection;

	return (num_intersection % 2) == 1;
}

/// Given two line segments, determine if they intersect.
///
/// @retval true Intersection
/// @retval false No intersection
bool Polygon::intersect(const PointD& p1, const PointD& p2, const PointD& p3, const PointD& p4)
{
	return (((ccw(p1, p2, p3) * ccw(p1, p2, p4)) <= 0)
			&& ((ccw(p3, p4, p1) * ccw(p3, p4, p2) <= 0)));
}

/// Checks wheather or not the three points are clockwise or counter clockwise.
///
/// @retval -1 Clockwise
/// @retval +1 Counterclockwise
int Polygon::ccw(const PointD& p0, const PointD& p1, const PointD& p2)
{
	double dx1 = p1.x - p0.x;
	double dx2 = p2.x - p0.x;
	double dy1 = p1.y - p0.y;
	double dy2 = p2.y - p0.y;

	// This is a slope comparison: we don't do divisions because of divide by zero
	// possibilities with pure horizontal and pure vertical lines.
	return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1);
}



/// @see inside
bool Polygon::insidef(const PointF* polygon, int polygon_size, const PointF& point)
{
	int num_intersection = 0;

	PointF p = point;
	p.x = 1.e8;

	// Now go through each of the lines in the polygon and see if it intersects
	for (int i = 0; i < polygon_size - 1; ++i) {
		if (intersectf(point, p, polygon[i], polygon[i + 1]))
			++num_intersection;
	}

	// And the last line
	if (intersectf(point, p, polygon[polygon_size - 1], polygon[0]))
		++num_intersection;

	return (num_intersection % 2) == 1;
}

/// @see intersect
bool Polygon::intersectf(const PointF& p1, const PointF& p2, const PointF& p3, const PointF& p4)
{
	return (((ccwf(p1, p2, p3) * ccwf(p1, p2, p4)) <= 0)
			&& ((ccwf(p3, p4, p1) * ccwf(p3, p4, p2) <= 0)));
}

/// @see ccw
int Polygon::ccwf(const PointF& p0, const PointF& p1, const PointF& p2)
{
	float dx1 = p1.x - p0.x;
	float dx2 = p2.x - p0.x;
	float dy1 = p1.y - p0.y;
	float dy2 = p2.y - p0.y;

	// This is a slope comparison: we don't do divisions because of divide by zero
	// possibilities with pure horizontal and pure vertical lines.
	return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1);
}

}

