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

#include "bbox.h"
#include <wx/matrix.h>

BoundingBox::BoundingBox()
{
	m_minx = m_miny = m_maxx =  m_maxy = 0.0;
	m_validbbox = FALSE;
}


BoundingBox::BoundingBox(const BoundingBox &other)
{
	m_minx = other.m_minx;
	m_miny = other.m_miny;
	m_maxx = other.m_maxx;
	m_maxy = other.m_maxy;
	m_validbbox= other.m_validbbox;
}


BoundingBox::BoundingBox(const wxPoint2DDouble& a)
{
	m_minx = a.m_x;
	m_maxx = a.m_x;
	m_miny = a.m_y;
	m_maxy = a.m_y;
	m_validbbox = TRUE;
}

BoundingBox::BoundingBox(double xmin, double ymin, double xmax, double ymax)
{
	m_minx = xmin;
	m_miny = ymin;
	m_maxx = xmax;
	m_maxy = ymax;
	m_validbbox = TRUE;
}

BoundingBox::~BoundingBox()
{
}

// This function checks if two bboxes intersect
bool BoundingBox::And(BoundingBox *_bbox, double Marge)
{
	assert (m_validbbox == TRUE);
	assert (_bbox->GetValid());
	m_minx = wxMax(m_minx, _bbox->m_minx);
	m_maxx = wxMin(m_maxx, _bbox->m_maxx);
	m_miny = wxMax(m_miny, _bbox->m_miny);
	m_maxy = wxMin(m_maxy, _bbox->m_maxy);
	return (bool)
		(
		 ((m_minx - Marge) < (m_maxx + Marge)) &&
		 ((m_miny - Marge) < (m_maxy + Marge))
		);
}

// Shrink the boundingbox with the given marge
void BoundingBox::Shrink(const double Marge)
{
	assert (m_validbbox == TRUE);

	m_minx += Marge;
	m_maxx -= Marge;
	m_miny += Marge;
	m_maxy -= Marge;
}


// Expand the boundingbox with another boundingbox
void BoundingBox::Expand(const BoundingBox &other)
{
	if (!m_validbbox)
	{
		*this=other;
	}
	else
	{
		m_minx = wxMin(m_minx, other.m_minx);
		m_maxx = wxMax(m_maxx, other.m_maxx);
		m_miny = wxMin(m_miny, other.m_miny);
		m_maxy = wxMax(m_maxy, other.m_maxy);
	}
}


// Expand the boundingbox with a point
void BoundingBox::Expand(const wxPoint2DDouble& a_point)
{
	if (!m_validbbox)
	{
		m_minx = m_maxx = a_point.m_x;
		m_miny = m_maxy = a_point.m_y;
		m_validbbox=TRUE;
	}
	else
	{
		m_minx = wxMin(m_minx, a_point.m_x);
		m_maxx = wxMax(m_maxx, a_point.m_x);
		m_miny = wxMin(m_miny, a_point.m_y);
		m_maxy = wxMax(m_maxy, a_point.m_y);
	}
}

// Expand the boundingbox with a point
void BoundingBox::Expand(double x,double y)
{
	if (!m_validbbox)
	{
		m_minx = m_maxx = x;
		m_miny = m_maxy = y;
		m_validbbox=TRUE;
	}
	else
	{
		m_minx = wxMin(m_minx, x);
		m_maxx = wxMax(m_maxx, x);
		m_miny = wxMin(m_miny, y);
		m_maxy = wxMax(m_maxy, y);
	}
}


// Expand the boundingbox with two points
void BoundingBox::Expand(const wxPoint2DDouble& a, const wxPoint2DDouble& b)
{
	Expand(a);
	Expand(b);
}

// Enlarge the boundingbox with the given marge
void BoundingBox::EnLarge(const double marge)
{
	if (!m_validbbox)
	{
		m_minx = m_maxx = marge;
		m_miny = m_maxy = marge;
		m_validbbox=TRUE;
	}
	else
	{
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
OVERLAP BoundingBox::Intersect(BoundingBox &other, double Marge)
{
	assert (m_validbbox == TRUE);

	// other boundingbox must exist
	assert (&other);

	if (((m_minx - Marge) > (other.m_maxx + Marge)) ||
			((m_maxx + Marge) < (other.m_minx - Marge)) ||
			((m_maxy + Marge) < (other.m_miny - Marge)) ||
			((m_miny - Marge) > (other.m_maxy + Marge)))
		return _OUT;

	// Check if other.bbox is inside this bbox
	if ((m_minx <= other.m_minx) &&
			(m_maxx >= other.m_maxx) &&
			(m_maxy >= other.m_maxy) &&
			(m_miny <= other.m_miny))
		return _IN;

	// Boundingboxes intersect
	return _ON;
}


// Checks if a line intersects the boundingbox
bool BoundingBox::LineIntersect(const wxPoint2DDouble& begin, const wxPoint2DDouble& end )
{
	assert (m_validbbox == TRUE);

	return (bool)
		!(((begin.m_y > m_maxy) && (end.m_y > m_maxy)) ||
				((begin.m_y < m_miny) && (end.m_y < m_miny)) ||
				((begin.m_x > m_maxx) && (end.m_x > m_maxx)) ||
				((begin.m_x < m_minx) && (end.m_x < m_minx)));
}


// Is the given point in the boundingbox ??
bool BoundingBox::PointInBox(double x, double y, double Marge)
{
	assert (m_validbbox == TRUE);

	if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
			y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
		return TRUE;
	return FALSE;
}


//
// Is the given point in the boundingbox ??
//
bool BoundingBox::PointInBox(const wxPoint2DDouble& a, double Marge)
{
	assert (m_validbbox == TRUE);

	return PointInBox(a.m_x, a.m_y, Marge);
}


wxPoint2DDouble BoundingBox::GetMin()
{
	assert (m_validbbox == TRUE);

	return wxPoint2DDouble(m_minx, m_miny);
}


wxPoint2DDouble BoundingBox::GetMax()
{
	assert (m_validbbox == TRUE);

	return wxPoint2DDouble(m_maxx, m_maxy);
}

bool BoundingBox::GetValid() const
{
	return m_validbbox;
}

void BoundingBox::SetMin(double px, double py)
{
	m_minx = px;
	m_miny = py;
	if (!m_validbbox)
	{
		m_maxx = px;
		m_maxy = py;
		m_validbbox = TRUE;
	}
}

void BoundingBox::SetMax(double px, double py)
{
	m_maxx = px;
	m_maxy = py;
	if (!m_validbbox)
	{
		m_minx = px;
		m_miny = py;
		m_validbbox = TRUE;
	}
}

void BoundingBox::SetValid(bool value)
{
	m_validbbox = value;
}

// adds an offset to the boundingbox
// usage : a_boundingbox.Translate(a_point);
void BoundingBox::Translate(wxPoint2DDouble& offset)
{
	assert (m_validbbox == TRUE);

	m_minx += offset.m_x;
	m_maxx += offset.m_x;
	m_miny += offset.m_y;
	m_maxy += offset.m_y;
}


// clears the bounding box settings
void BoundingBox::Reset()
{
	m_minx = 0.0;
	m_maxx = 0.0;
	m_miny = 0.0;
	m_maxy = 0.0;
	m_validbbox = FALSE;
}


void BoundingBox::SetBoundingBox(const wxPoint2DDouble& a_point)
{
	m_minx = a_point.m_x;
	m_maxx = a_point.m_x;
	m_miny = a_point.m_y;
	m_maxy = a_point.m_y;
}


// Expands the boundingbox with the given point
// usage : a_boundingbox = a_boundingbox + pointer_to_an_offset;
BoundingBox& BoundingBox::operator+(BoundingBox &other)
{
	assert (m_validbbox == TRUE);
	assert (other.GetValid());

	Expand(other);
	return *this;
}


// makes a boundingbox same as the other
BoundingBox& BoundingBox::operator=( const BoundingBox &other)
{
	assert (other.GetValid());

	m_minx = other.m_minx;
	m_maxx = other.m_maxx;
	m_miny = other.m_miny;
	m_maxy = other.m_maxy;
	m_validbbox = other.m_validbbox;
	return *this;
}

void BoundingBox::MapBbox( const wxTransformMatrix& matrix)
{
	assert (m_validbbox == TRUE);

	double x1,y1,x2,y2,x3,y3,x4,y4;

	matrix.TransformPoint( m_minx, m_miny, x1, y1 );
	matrix.TransformPoint( m_minx, m_maxy, x2, y2 );
	matrix.TransformPoint( m_maxx, m_maxy, x3, y3 );
	matrix.TransformPoint( m_maxx, m_miny, x4, y4 );

	double xmin = wxMin(x1,x2);
	xmin = wxMin(xmin,x3);
	xmin = wxMin(xmin,x4);

	double xmax = wxMax( x1, x2);
	xmax = wxMax(xmax,x3);
	xmax = wxMax(xmax,x4);

	double ymin = wxMin(y1, y2);
	ymin = wxMin(ymin,y3);
	ymin = wxMin(ymin,y4);

	double ymax = wxMax(y1,y2);
	ymax = wxMax(ymax,y3);
	ymax = wxMax(ymax,y4);

	// Use these min and max values to set the new boundingbox
	m_minx = xmin;
	m_miny = ymin;
	m_maxx = xmax;
	m_maxy = ymax;
}

//----------------------------------------------------------------
//    LLBBox Implementation
//----------------------------------------------------------------


// Is the given LL point in the boundingbox ??
bool LLBBox::PointInBox(double Lon, double Lat, double Marge)
{
	double x = Lon;
	double y = Lat;

	//    Box is centered in East lon, crossing IDL
	if(m_maxx > 180.)
	{
		if( x < m_maxx - 360.)
			x +=  360.;

		if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
				y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
			return TRUE;
		return FALSE;
	}

	//    Box is centered in Wlon, crossing IDL
	else if(m_minx < -180.)
	{
		if(x > m_minx + 360.)
			x -= 360.;

		if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
				y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
			return TRUE;
		return FALSE;
	}

	else
	{
		if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
				y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
			return TRUE;
		return FALSE;
	}
}
