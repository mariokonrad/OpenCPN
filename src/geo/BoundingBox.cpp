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

#include "BoundingBox.h"
#include <algorithm>
#include <cassert>

namespace geo {

BoundingBox::BoundingBox()
	: m_minx(0.0)
	, m_miny(0.0)
	, m_maxx(0.0)
	, m_maxy(0.0)
	, m_validbbox(false)
{
}

BoundingBox::BoundingBox(const BoundingBox& other)
{
	m_minx = other.m_minx;
	m_miny = other.m_miny;
	m_maxx = other.m_maxx;
	m_maxy = other.m_maxy;
	m_validbbox = other.m_validbbox;
}

BoundingBox::BoundingBox(double xmin, double ymin, double xmax, double ymax)
	: m_minx(xmin)
	, m_miny(ymin)
	, m_maxx(xmax)
	, m_maxy(ymax)
	, m_validbbox(true)
{
}

BoundingBox::~BoundingBox()
{
}

// This function checks if two bboxes intersect
bool BoundingBox::And(const BoundingBox& box, double Marge)
{
	assert(m_validbbox == true);
	assert(box.GetValid());
	m_minx = std::max(m_minx, box.m_minx);
	m_maxx = std::min(m_maxx, box.m_maxx);
	m_miny = std::max(m_miny, box.m_miny);
	m_maxy = std::min(m_maxy, box.m_maxy);
	return ((m_minx - Marge) < (m_maxx + Marge)) && ((m_miny - Marge) < (m_maxy + Marge));
}

// Shrink the boundingbox with the given marge
void BoundingBox::Shrink(const double Marge)
{
	assert (m_validbbox == true);

	m_minx += Marge;
	m_maxx -= Marge;
	m_miny += Marge;
	m_maxy -= Marge;
}

// Expand the boundingbox with another boundingbox
void BoundingBox::Expand(const BoundingBox& other)
{
	if (!m_validbbox) {
		*this = other;
	} else {
		m_minx = std::min(m_minx, other.m_minx);
		m_maxx = std::max(m_maxx, other.m_maxx);
		m_miny = std::min(m_miny, other.m_miny);
		m_maxy = std::max(m_maxy, other.m_maxy);
	}
}

// Expand the boundingbox with a point
void BoundingBox::Expand(double x, double y)
{
	if (!m_validbbox) {
		m_minx = m_maxx = x;
		m_miny = m_maxy = y;
		m_validbbox = true;
	} else {
		m_minx = std::min(m_minx, x);
		m_maxx = std::max(m_maxx, x);
		m_miny = std::min(m_miny, y);
		m_maxy = std::max(m_maxy, y);
	}
}

// Enlarge the boundingbox with the given marge
void BoundingBox::EnLarge(const double marge)
{
	if (!m_validbbox) {
		m_minx = m_maxx = marge;
		m_miny = m_maxy = marge;
		m_validbbox = true;
	} else {
		m_minx -= marge;
		m_maxx += marge;
		m_miny -= marge;
		m_maxy += marge;
	}
}

// Calculates if two boundingboxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
BoundingBox::OVERLAP BoundingBox::Intersect(BoundingBox& other, double Marge) const
{
	assert(m_validbbox == true);

	// other boundingbox must exist
	assert(&other);

	if (((m_minx - Marge) > (other.m_maxx + Marge))
		|| ((m_maxx + Marge) < (other.m_minx - Marge))
		|| ((m_maxy + Marge) < (other.m_miny - Marge))
		|| ((m_miny - Marge) > (other.m_maxy + Marge)))
		return _OUT;

	// Check if other.bbox is inside this bbox
	if ((m_minx <= other.m_minx)
		&& (m_maxx >= other.m_maxx)
		&& (m_maxy >= other.m_maxy)
		&& (m_miny <= other.m_miny))
		return _IN;

	// Boundingboxes intersect
	return _ON;
}

// Checks if a line intersects the boundingbox
bool BoundingBox::LineIntersect(double x0, double y0, double x1, double y1) const
{
	assert(m_validbbox == true);
	return static_cast<bool>(!(((y0 > m_maxy) && (y1 > m_maxy))
							   || ((y0 < m_miny) && (y1 < m_miny))
							   || ((x0 > m_maxx) && (x1 > m_maxx))
							   || ((x0 < m_minx) && (x1 < m_minx))));
}

// Is the given point in the boundingbox ??
bool BoundingBox::PointInBox(double x, double y, double Marge) const
{
	assert(m_validbbox == true);

	if (x >= (m_minx - Marge) && x <= (m_maxx + Marge)
		&& y >= (m_miny - Marge) && y <= (m_maxy + Marge))
		return true;
	return false;
}

bool BoundingBox::GetValid() const
{
	return m_validbbox;
}

void BoundingBox::SetMin(double px, double py)
{
	m_minx = px;
	m_miny = py;
	if (!m_validbbox) {
		m_maxx = px;
		m_maxy = py;
		m_validbbox = true;
	}
}

void BoundingBox::SetMax(double px, double py)
{
	m_maxx = px;
	m_maxy = py;
	if (!m_validbbox) {
		m_minx = px;
		m_miny = py;
		m_validbbox = true;
	}
}

void BoundingBox::SetValid(bool value)
{
	m_validbbox = value;
}

// adds an offset to the boundingbox
void BoundingBox::Translate(double dx, double dy)
{
	assert(m_validbbox == true);

	m_minx += dx;
	m_maxx += dx;
	m_miny += dy;
	m_maxy += dy;
}

// clears the bounding box settings
void BoundingBox::Reset()
{
	m_minx = 0.0;
	m_maxx = 0.0;
	m_miny = 0.0;
	m_maxy = 0.0;
	m_validbbox = false;
}

// Expands the boundingbox with the given point
// usage : a_boundingbox = a_boundingbox + pointer_to_an_offset;
BoundingBox& BoundingBox::operator+(BoundingBox& other)
{
	assert(m_validbbox == true);
	assert(other.GetValid());

	Expand(other);
	return *this;
}

// makes a boundingbox same as the other
BoundingBox& BoundingBox::operator=(const BoundingBox& other)
{
	assert(other.GetValid());

	m_minx = other.m_minx;
	m_maxx = other.m_maxx;
	m_miny = other.m_miny;
	m_maxy = other.m_maxy;
	m_validbbox = other.m_validbbox;
	return *this;
}

double BoundingBox::GetMinX() const
{
	return m_minx;
}

double BoundingBox::GetMinY() const
{
	return m_miny;
}

double BoundingBox::GetMaxX() const
{
	return m_maxx;
}

double BoundingBox::GetMaxY() const
{
	return m_maxy;
}

double BoundingBox::GetWidth() const
{
	return m_maxx - m_minx;
}

double BoundingBox::GetHeight() const
{
	return m_maxy - m_miny;
}

}

