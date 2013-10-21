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

#ifndef __GEO__POLYTESSGEO__H__
#define __GEO__POLYTESSGEO__H__

#include <ogr_geometry.h>
#include <cstdio>

//  Error Return Codes
#define ERROR_NONE        0
#define ERROR_NO_DLL      1
#define ERROR_BAD_OGRPOLY 2

class wxOutputStream;

namespace geo {

class ExtendedGeometry;
class PolyTriGroup;

/// Tesselator
class PolyTessGeo
{
	public:
		PolyTessGeo();
		~PolyTessGeo();

		PolyTessGeo(unsigned char *polybuf, int nrecl, int index);      // Build this from SENC file record

		PolyTessGeo(
				OGRPolygon * poly,
				bool bSENC_SM,
				double ref_lat,
				double ref_lon,
				bool bUseInternalTess);  // Build this from OGRPolygon

		PolyTessGeo(ExtendedGeometry *pxGeom);

		bool IsOk(){ return m_bOK;}

        int BuildDeferredTess(void);

		int Write_PolyTriGroup(FILE *ofs);
		int Write_PolyTriGroup(wxOutputStream & ostream);

		double Get_xmin() const { return xmin;}
		double Get_xmax() const { return xmax;}
		double Get_ymin() const { return ymin;}
		double Get_ymax() const { return ymax;}
		PolyTriGroup * Get_PolyTriGroup_head(){ return m_ppg_head;}
		int GetnVertexMax(){ return m_nvertex_max; }
		int ErrorCode;


	private:
		int BuildTessGL(void);
		int PolyTessGeoGL(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon);
		int PolyTessGeoTri(OGRPolygon *poly, bool bSENC_SM, double ref_lat, double ref_lon);
		int my_bufgets( char *buf, int buf_len_max );

		bool m_bOK;
		ExtendedGeometry * m_pxgeom;
		double xmin;
		double xmax;
		double ymin;
		double ymax;
		PolyTriGroup * m_ppg_head; // head of a PolyTriGroup chain
		int m_nvertex_max; // and computed max vertex count used by drawing primitives as optimization

		int ncnt;
		int nwkb;

		char * m_buf_head;
		char * m_buf_ptr;                   // used to read passed SENC record
		int m_nrecl;

		double m_ref_lat;
		double m_ref_lon;
};

}

#endif
