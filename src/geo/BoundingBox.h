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

#ifndef __WXBOUNDINGBOX_H__
#define __WXBOUNDINGBOX_H__

#include <wx/geometry.h>

class wxTransformMatrix;
class wxPoint2DDouble;

namespace geo {

//Purpose   The BoundingBox class stores one BoundingBox.
//The BoundingBox is defined by two coordiates,
//a upperleft coordinate and a lowerright coordinate.
class BoundingBox
{
	public:
		enum OVERLAP
		{
			_IN,
			_ON,
			_OUT
		};

	public:
		BoundingBox();
		BoundingBox(const BoundingBox&);
		BoundingBox(const wxPoint2DDouble&);
		BoundingBox(double xmin, double ymin, double xmax, double ymax);
		virtual ~BoundingBox();

		bool And(BoundingBox*, double Marge = 0);

		void EnLarge(const double Marge);
		void Shrink(const double Marge);

		void Expand(const wxPoint2DDouble& , const wxPoint2DDouble&);
		void Expand(const wxPoint2DDouble&);
		void Expand(double x,double y);
		void Expand(const BoundingBox& bbox);

		OVERLAP Intersect(BoundingBox &, double Marge = 0) const;
		bool LineIntersect(const wxPoint2DDouble & begin, const wxPoint2DDouble & end) const;
		bool PointInBox(const wxPoint2DDouble &, double Marge = 0) const;
		virtual bool PointInBox(double, double, double Marge = 0) const;

		void Reset();

		void Translate( wxPoint2DDouble& );
		void MapBbox( const wxTransformMatrix & matrix);

		double GetWidth() const;
		double GetHeight() const;
		bool GetValid() const;
		void SetValid(bool);

		void SetBoundingBox(const wxPoint2DDouble& a_point);

		void SetMin(double, double);
		void SetMax(double, double);
		wxPoint2DDouble GetMin() const;
		wxPoint2DDouble GetMax() const;
		double GetMinX() const;
		double GetMinY() const;
		double GetMaxX() const;
		double GetMaxY() const;

		BoundingBox & operator+(BoundingBox &);
		BoundingBox & operator=(const BoundingBox &);

	protected:
		//bounding box in world
		double m_minx;
		double m_miny;
		double m_maxx;
		double m_maxy;
		bool m_validbbox;
};

}

#endif
