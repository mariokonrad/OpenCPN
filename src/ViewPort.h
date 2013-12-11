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
#include <Position.h>

namespace geo { class LatLonBoundingBox; }

class ViewPort
{
public:
	ViewPort();

	wxPoint GetPixFromLL(const Position& pos) const;
	Position GetLLFromPix(const wxPoint& p) const;
	wxPoint2DDouble GetDoublePixFromLL(const Position& pos) const;

	OCPNRegion GetVPRegionIntersect(const OCPNRegion& Region, size_t n, const float* llpoints,
									int chart_native_scale, wxPoint* ppoints = NULL) const;

	wxRect GetVPRectIntersect(size_t n, const float* llpoints) const;

	void SetBoxes(void);
	void Invalidate();
	void Validate();
	bool IsValid() const;
	void SetRotationAngle(double angle_rad);
	void SetProjectionType(int type);

	const geo::LatLonBoundingBox& GetBBox() const;
	geo::LatLonBoundingBox& GetBBox();
	void set_positive();

	double latitude() const;
	double longitude() const;

	const Position& get_position() const;
	void set_position(const Position& pos);

	// FIXME: move public attributes to private

	// Generic
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
	Position center_point;
	geo::LatLonBoundingBox vpBBox; // An un-skewed rectangular lat/lon bounding box which contains
								   // the entire vieport
	bool bValid; // This VP is valid
};

#endif
