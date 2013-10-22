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

#ifndef __VIEWPORT__H__
#define __VIEWPORT__H__

#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <geo/LatLonBoundingBox.h>
#include <OCPNRegion.h>

class LatLonBoundingBox;

class ViewPort
{
	public:
		ViewPort();

		wxPoint GetPixFromLL(double lat, double lon) const;
		void GetLLFromPix(const wxPoint &p, double *lat, double *lon);
		wxPoint2DDouble GetDoublePixFromLL(double lat, double lon);

		OCPNRegion GetVPRegionIntersect(
				const OCPNRegion & Region,
				size_t n,
				float * llpoints,
				int chart_native_scale,
				wxPoint * ppoints = NULL);

		wxRect GetVPRectIntersect(size_t n, float * llpoints);

		void SetBoxes(void);
		void Invalidate();
		void Validate();
		bool IsValid() const;
		void SetRotationAngle(double angle_rad);
		void SetProjectionType(int type);

		const LatLonBoundingBox & GetBBox() const;
		LatLonBoundingBox & GetBBox();
		void set_positive();

		//  Generic
		double clat; // center point
		double clon;
		double view_scale_ppm;
		double skew;
		double rotation;

		double chart_scale; // conventional chart displayed scale

		int pix_width;
		int pix_height;

		bool b_quilt;
		bool b_FullScreenQuilt;

		int m_projection_type;
		bool b_MercatorProjectionOverride;
		wxRect rv_rect;

	private:
		LatLonBoundingBox vpBBox; // An un-skewed rectangular lat/lon bounding box which contains the entire vieport
		bool bValid; // This VP is valid
};

#endif
