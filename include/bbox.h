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

enum OVERLAP {_IN,_ON,_OUT};

//Purpose   The BoundingBox class stores one BoundingBox.
//The BoundingBox is defined by two coordiates,
//a upperleft coordinate and a lowerright coordinate.
class BoundingBox
{
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

    OVERLAP Intersect(BoundingBox &, double Marge = 0);
    bool LineIntersect(const wxPoint2DDouble& begin, const wxPoint2DDouble& end );
    bool PointInBox( const wxPoint2DDouble&, double Marge = 0);
    virtual bool PointInBox( double, double, double Marge = 0);

    void Reset();

    void Translate( wxPoint2DDouble& );
    void MapBbox( const wxTransformMatrix & matrix);

    double  GetWidth() {return m_maxx-m_minx;};
    double  GetHeight(){return m_maxy-m_miny;};
    bool    GetValid()  const;
    void    SetValid(bool);

    void    SetBoundingBox(const wxPoint2DDouble& a_point);

    void    SetMin(double, double);
    void    SetMax(double, double);
    inline  wxPoint2DDouble GetMin();
    inline  wxPoint2DDouble GetMax();
    inline  double GetMinX(){return m_minx;};
    inline  double GetMinY(){return m_miny;};
    inline  double GetMaxX(){return m_maxx;};
    inline  double GetMaxY(){return m_maxy;};

    BoundingBox&  operator+( BoundingBox& );
    BoundingBox&  operator=(  const BoundingBox& );

protected:
    //bounding box in world
    double        m_minx;
    double        m_miny;
    double        m_maxx;
    double        m_maxy;
    bool          m_validbbox;
};

//    A class derived from BoundingBox
//    that is assummed to be a geographic area, with coordinates
//    expressed in Lat/Lon.
//    This class understands the International Date Line (E/W Longitude)

class LLBBox : public BoundingBox
{
      public:
            bool PointInBox(double Lon, double Lat, double Marge);
};



#endif
