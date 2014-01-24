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



static int Intersect(PointD, PointD, PointD, PointD);
static int CCW(PointD, PointD, PointD);

static int Intersect_FL(PointF, PointF, PointF, PointF);
static int CCW_FL(PointF, PointF, PointF);

/// This routine determines if the point passed is in the polygon. It uses
/// the classical polygon hit-testing algorithm: a horizontal ray starting
/// at the point is extended infinitely rightwards and the number of
/// polygon edges that intersect the ray are counted. If the number is odd,
/// the point is inside the polygon.
///
/// Polygon is assumed OPEN, not CLOSED.
/// RETURN VALUE
/// (bool) TRUE if the point is inside the polygon, FALSE if not.
int G_PtInPolygon(const PointD* rgpts, int wnumpts, float x, float y)
{
	const PointD* ppt;
	const PointD* ppt1;
	PointD pt1;
	PointD pt2;
	PointD pt0;
	int wnumintsct = 0;

	pt0.x = x;
	pt0.y = y;

	pt1 = pt2 = pt0;
	pt2.x = 1.e8;

	// Now go through each of the lines in the polygon and see if it
	// intersects
	ppt = rgpts;
	for (int i = 0; i < wnumpts - 1; i++, ppt++) {
		ppt1 = ppt;
		ppt1++;
		if (Intersect(pt0, pt2, *ppt, *(ppt1)))
			wnumintsct++;
	}

	// And the last line
	if (Intersect(pt0, pt2, *ppt, *rgpts))
		wnumintsct++;

	return wnumintsct & 1;
}

/// Given two line segments, determine if they intersect.
///
/// TRUE if they intersect, FALSE if not.
static int Intersect(PointD p1, PointD p2, PointD p3, PointD p4)
{
	int i;
	i = CCW(p1, p2, p3);
	i = CCW(p1, p2, p4);
	i = CCW(p3, p4, p1);
	i = CCW(p3, p4, p2);
	return (((CCW(p1, p2, p3) * CCW(p1, p2, p4)) <= 0)
			&& ((CCW(p3, p4, p1) * CCW(p3, p4, p2) <= 0)));
}

/// Determines, given three points, if when travelling from the first to
/// the second to the third, we travel in a counterclockwise direction.
///
/// (int) 1 if the movement is in a counterclockwise direction, -1 if
/// not.
static int CCW(PointD p0, PointD p1, PointD p2)
{
	double dx1 = p1.x - p0.x;
	double dx2 = p2.x - p0.x;
	double dy1 = p1.y - p0.y;
	double dy2 = p2.y - p0.y;

	// This is a slope comparison: we don't do divisions because
	// of divide by zero possibilities with pure horizontal and pure
	// vertical lines.
	return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1);
}

int G_PtInPolygon_FL(const PointF* rgpts, int wnumpts, float x, float y)
{
	// FIXME: check for code duplication (possibly use a template)

	const PointF* ppt;
	const PointF* ppt1;
	PointF pt0;
	PointF pt1;
	PointF pt2;
	int wnumintsct = 0;

	pt0.x = x;
	pt0.y = y;

	pt1 = pt2 = pt0;
	pt2.x = 1.e8;

	// Now go through each of the lines in the polygon and see if it
	// intersects
	ppt = rgpts;
	for (int i = 0; i < wnumpts - 1; i++, ppt++) {
		ppt1 = ppt;
		ppt1++;
		if (Intersect_FL(pt0, pt2, *ppt, *(ppt1)))
			wnumintsct++;
	}

	// And the last line
	if (Intersect_FL(pt0, pt2, *ppt, *rgpts))
		wnumintsct++;

	return wnumintsct & 1;
}

/// Given two line segments, determine if they intersect.
///
/// RETURN VALUE
/// TRUE if they intersect, FALSE if not.
static int Intersect_FL(PointF p1, PointF p2, PointF p3, PointF p4)
{
	int i;
	i = CCW_FL(p1, p2, p3);
	i = CCW_FL(p1, p2, p4);
	i = CCW_FL(p3, p4, p1);
	i = CCW_FL(p3, p4, p2);
	return (((CCW_FL(p1, p2, p3) * CCW_FL(p1, p2, p4)) <= 0)
			&& ((CCW_FL(p3, p4, p1) * CCW_FL(p3, p4, p2) <= 0)));
}

/// Determines, given three points, if when travelling from the first to
/// second to the third, we travel in a counterclockwise direction.
///
/// RETURN VALUE
/// (int) 1 if the movement is in a counterclockwise direction, -1 if
/// not.
static int CCW_FL(PointF p0, PointF p1, PointF p2)
{
	float dx1 = p1.x - p0.x;
	float dx2 = p2.x - p0.x;
	float dy1 = p1.y - p0.y;
	float dy2 = p2.y - p0.y;

	// This is a slope comparison: we don't do divisions because
	// of divide by zero possibilities with pure horizontal and pure
	// vertical lines.

	return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1);
}

}

