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

#ifndef __GEO__BOUNDINGBOX__H__
#define __GEO__BOUNDINGBOX__H__

namespace geo {

// The BoundingBox class stores one BoundingBox.
// The BoundingBox is defined by two coordiates,
// a upperleft coordinate and a lowerright coordinate.
class BoundingBox
{
public:
	enum OVERLAP {
		_IN,
		_ON,
		_OUT
	};

public:
	BoundingBox();
	BoundingBox(const BoundingBox&);
	BoundingBox(double xmin, double ymin, double xmax, double ymax);
	virtual ~BoundingBox();

	bool And(const BoundingBox&, double Marge = 0.0);

	void EnLarge(const double Marge);
	void Shrink(const double Marge);

	void Expand(double x, double y);
	void Expand(const BoundingBox& bbox);

	OVERLAP Intersect(BoundingBox&, double Marge = 0) const;
	bool LineIntersect(double x0, double y0, double x1, double y1) const;
	virtual bool PointInBox(double x, double y, double Marge = 0) const;

	void Reset();

	void Translate(double, double);

	double GetWidth() const;
	double GetHeight() const;
	bool GetValid() const;
	void SetValid(bool);

	void SetMin(double, double);
	void SetMax(double, double);
	double GetMinX() const;
	double GetMinY() const;
	double GetMaxX() const;
	double GetMaxY() const;

	BoundingBox& operator+(BoundingBox&);
	BoundingBox& operator=(const BoundingBox&);

protected:
	// bounding box in world
	double m_minx;
	double m_miny;
	double m_maxx;
	double m_maxy;
	bool m_validbbox;
};

}

#endif
