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

#include <wx/wx.h>

#include "ViewPort.h"

#include <OCPNRegion.h>
#include <ProjectionType.h>

#include <windows/compatibility.h>

#include <geo/GeoRef.h>

#include <global/OCPN.h>
#include <global/Navigation.h>

#ifndef __WXMSW__
	#include <signal.h>
	#include <setjmp.h>
#endif

extern bool g_bskew_comp;

#ifndef __WXMSW__
extern struct sigaction sa_all;
extern struct sigaction sa_all_old;
extern sigjmp_buf env; // the context saved by sigsetjmp();
void catch_signals(int signo);
#endif


ViewPort::ViewPort()
	: valid(false)
	, quilt(false)
	, fullscreen_quilt(false)
	, view_scale_ppm(1.0)
{
	skew = 0.0;
	rotation = 0.0;
	pix_height = pix_width = 0;
	b_MercatorProjectionOverride = false;
}

bool ViewPort::is_quilt() const
{
	return quilt;
}

bool ViewPort::is_fullscreen_quilt() const
{
	return fullscreen_quilt;
}

void ViewPort::set_quilt(bool quilt, bool fullscreen_quilt)
{
	this->quilt = quilt;
	this->fullscreen_quilt = fullscreen_quilt;
}

double ViewPort::view_scale() const
{
	return view_scale_ppm;
}

void ViewPort::set_view_scale(double value)
{
	view_scale_ppm = value;
}

wxPoint ViewPort::GetPixFromLL(const geo::Position& pos) const
{
	double easting, northing;
	double xlon = pos.lon();

	//  Make sure lon and lon0 are same phase
	if (xlon * center_point.lon() < 0.0) {
		if (xlon < 0.0)
			xlon += 360.0;
		else
			xlon -= 360.0;
	}

	if (fabs(xlon - center_point.lon()) > 180.0) {
		if (xlon > center_point.lon())
			xlon -= 360.0;
		else
			xlon += 360.0;
	}

	if (PROJECTION_TRANSVERSE_MERCATOR == m_projection_type) {
		// We calculate northings as referenced to the equator
		// And eastings as though the projection point is midscreen.

		double tmeasting, tmnorthing;
		double tmceasting, tmcnorthing;
		geo::toTM(center_point, geo::Position(0.0, center_point.lon()), &tmceasting, &tmcnorthing);
		geo::toTM(geo::Position(pos.lat(), xlon), geo::Position(0.0, center_point.lon()),
				  &tmeasting, &tmnorthing);

		northing = tmnorthing - tmcnorthing;
		easting = tmeasting - tmceasting;
	} else if (PROJECTION_POLYCONIC == m_projection_type) {

		//    We calculate northings as referenced to the equator
		//    And eastings as though the projection point is midscreen.
		double pceasting, pcnorthing;
		geo::toPOLY(center_point, geo::Position(0.0, center_point.lon()), &pceasting, &pcnorthing);

		double peasting, pnorthing;
		geo::toPOLY(geo::Position(pos.lat(), xlon), geo::Position(0.0, center_point.lon()),
					&peasting, &pnorthing);

		easting = peasting;
		northing = pnorthing - pcnorthing;
	} else {
		geo::toSM(geo::Position(pos.lat(), xlon), center_point, &easting, &northing);
	}

	if (!wxFinite(easting) || !wxFinite(northing))
		return wxPoint(0, 0);

	double epix = easting * view_scale();
	double npix = northing * view_scale();
	double dxr = epix;
	double dyr = npix;

	// Apply VP Rotation
	if (global::OCPN::get().nav().get_data().CourseUp) {
		dxr = epix * cos(rotation) + npix * sin(rotation);
		dyr = npix * cos(rotation) - epix * sin(rotation);
	}
	wxPoint r;
	// We definitely need a round() function here
	r.x = (int)wxRound((pix_width / 2) + dxr);
	r.y = (int)wxRound((pix_height / 2) - dyr);

	return r;
}

wxPoint2DDouble ViewPort::GetDoublePixFromLL(const geo::Position& pos) const
{
	double easting, northing;
	double xlon = pos.lon();

	// Make sure lon and lon0 are same phase
	if (xlon * center_point.lon() < 0.0) {
		if (xlon < 0.0)
			xlon += 360.0;
		else
			xlon -= 360.0;
	}

	if (fabs(xlon - center_point.lon()) > 180.0) {
		if (xlon > center_point.lon())
			xlon -= 360.0;
		else
			xlon += 360.0;
	}

	if (PROJECTION_TRANSVERSE_MERCATOR == m_projection_type) {
		// We calculate northings as referenced to the equator
		// And eastings as though the projection point is midscreen.

		double tmeasting, tmnorthing;
		double tmceasting, tmcnorthing;
		geo::toTM(center_point, geo::Position(0.0, center_point.lon()), &tmceasting, &tmcnorthing);
		geo::toTM(geo::Position(pos.lat(), xlon), geo::Position(0.0, center_point.lon()),
				  &tmeasting, &tmnorthing);

		northing = tmnorthing - tmcnorthing;
		easting = tmeasting - tmceasting;
	} else if (PROJECTION_POLYCONIC == m_projection_type) {
		// We calculate northings as referenced to the equator
		// And eastings as though the projection point is midscreen.
		double pceasting, pcnorthing;
		geo::toPOLY(center_point, geo::Position(0.0, center_point.lon()), &pceasting, &pcnorthing);

		double peasting, pnorthing;
		geo::toPOLY(geo::Position(pos.lat(), xlon), geo::Position(0.0, center_point.lon()),
					&peasting, &pnorthing);

		easting = peasting;
		northing = pnorthing - pcnorthing;
	} else {
		geo::toSM(geo::Position(pos.lat(), xlon), center_point, &easting, &northing);
	}

	if (!wxFinite(easting) || !wxFinite(northing))
		return wxPoint(0, 0);

	double epix = easting * view_scale();
	double npix = northing * view_scale();
	double dxr = epix;
	double dyr = npix;

	// Apply VP Rotation
	if (global::OCPN::get().nav().get_data().CourseUp) {
		dxr = epix * cos(rotation) + npix * sin(rotation);
		dyr = npix * cos(rotation) - epix * sin(rotation);
	}

	wxPoint2DDouble r;
	// We definitely need a round() function here
	r.m_x = ((pix_width / 2) + dxr);
	r.m_y = ((pix_height / 2) - dyr);

	return r;
}

geo::Position ViewPort::GetLLFromPix(const wxPoint& p) const
{
	int dx = p.x - (pix_width / 2);
	int dy = (pix_height / 2) - p.y;

	double xpr = dx;
	double ypr = dy;

	// Apply VP Rotation
	if (global::OCPN::get().nav().get_data().CourseUp) {
		xpr = (dx * cos(rotation)) - (dy * sin(rotation));
		ypr = (dy * cos(rotation)) + (dx * sin(rotation));
	}
	double d_east = xpr / view_scale();
	double d_north = ypr / view_scale();

	geo::Position pos;
	if (PROJECTION_TRANSVERSE_MERCATOR == m_projection_type) {
		double tmceasting;
		double tmcnorthing;
		geo::toTM(center_point, geo::Position(0.0, center_point.lon()), &tmceasting, &tmcnorthing);
		pos = geo::fromTM(d_east, d_north + tmcnorthing, geo::Position(0.0, center_point.lon()));
	} else if (PROJECTION_POLYCONIC == m_projection_type) {
		double polyeasting;
		double polynorthing;
		geo::toPOLY(center_point, geo::Position(0.0, center_point.lon()), &polyeasting,
					&polynorthing);
		pos = geo::fromPOLY(d_east, d_north + polynorthing, geo::Position(0.0, center_point.lon()));
	} else {
		// TODO This could be fromSM_ECC to better match some Raster charts
		//      However, it seems that cm93 (and S57) prefer no eccentricity correction
		//      Think about it....
		pos = geo::fromSM(d_east, d_north, center_point);
	}

	pos.normalize_lon();
	return pos;
}

OCPNRegion ViewPort::GetVPRegionIntersect(
		const OCPNRegion & Region,
		size_t n,
		const float * llpoints,
		int chart_native_scale,
		wxPoint * ppoints) const
{
	using geo::BoundingBox;

	// Calculate the intersection between a given OCPNRegion (Region) and a polygon specified by
	// lat/lon points.

	// If the viewpoint is highly overzoomed wrt to chart native scale, the polygon region may be
	// huge.
	// This can be very expensive, and lead to crashes on some platforms (gtk in particular)
	// So, look for this case and handle appropriately with respect to the given Region

	if (chart_scale < chart_native_scale / 10) {
		// Make a positive definite vp
		ViewPort vp_positive = *this;
		while (vp_positive.vpBBox.GetMinX() < 0) {
			vp_positive.set_position(geo::Position(vp_positive.latitude(), vp_positive.longitude() + 360.0));
			vp_positive.vpBBox.Translate(360.0, 0.0);
		}

		// Scan the points one-by-one, so that we can get min/max to make a bbox
		const float* pfp = llpoints;
		float lon_max = -10000.0;
		float lon_min = 10000.0;
		float lat_max = -10000.0;
		float lat_min = 10000.0;

		for (unsigned int ip = 0; ip < n; ip++) {
			lon_max = wxMax(lon_max, pfp[1]);
			lon_min = wxMin(lon_min, pfp[1]);
			lat_max = wxMax(lat_max, pfp[0]);
			lat_min = wxMin(lat_min, pfp[0]);

			pfp += 2;
		}

		BoundingBox chart_box(lon_min, lat_min, lon_max, lat_max);

		// Case:  vpBBox is completely outside the chart box, or vice versa
		// Return an empty region
		if (BoundingBox::_OUT == chart_box.Intersect((BoundingBox&)vp_positive.vpBBox)) {
			if (BoundingBox::_OUT == chart_box.Intersect((BoundingBox&)vpBBox)) {
				// try again with the chart translated 360
				BoundingBox trans_box = chart_box;
				trans_box.Translate(360.0, 0.0);

				if (BoundingBox::_OUT == trans_box.Intersect((BoundingBox&)vp_positive.vpBBox)) {
					if (BoundingBox::_OUT == trans_box.Intersect((BoundingBox&)vpBBox)) {
						return OCPNRegion();
					}
				}
			}
		}

		// Case:  vpBBox is completely inside the chart box
		// Note that this test is not perfect, and will fail for some charts.
		// The chart coverage may be  essentially triangular, and the viewport box
		// may be in the "cut off" segment of the chart_box, and not actually
		// exhibit any true overlap.  Results will be reported incorrectly.
		// How to fix: maybe scrub the chart points and see if it is likely that
		// a region may be safely built and intersection tested.

		if (BoundingBox::_IN == chart_box.Intersect((BoundingBox&)vp_positive.vpBBox)) {
			return Region;
		}

		if (BoundingBox::_IN == chart_box.Intersect((BoundingBox&)vpBBox)) {
			return Region;
		}

		// The ViewPort and the chart region overlap in some way....
		// Create the intersection of the two bboxes
		// Boxes must be same phase
		while (chart_box.GetMinX() < 0) {
			chart_box.Translate(360.0, 0.0);
		}

		double cb_minlon = wxMax(chart_box.GetMinX(), vp_positive.vpBBox.GetMinX());
		double cb_maxlon = wxMin(chart_box.GetMaxX(), vp_positive.vpBBox.GetMaxX());
		double cb_minlat = wxMax(chart_box.GetMinY(), vp_positive.vpBBox.GetMinY());
		double cb_maxlat = wxMin(chart_box.GetMaxY(), vp_positive.vpBBox.GetMaxY());

		if (cb_maxlon < cb_minlon)
			cb_maxlon += 360.0;

		wxPoint p1 = GetPixFromLL(geo::Position(cb_maxlat, cb_minlon)); // upper left
		wxPoint p2 = GetPixFromLL(geo::Position(cb_minlat, cb_maxlon)); // lower right

		OCPNRegion r(p1, p2);
		r.Intersect(Region);
		return r;
	}

	// More "normal" case

	wxPoint* pp = NULL;

	// Use the passed point buffer if available
	if (ppoints == NULL)
		pp = new wxPoint[n];
	else
		pp = ppoints;

	const float* pfp = llpoints;

	for (unsigned int ip = 0; ip < n; ip++) {
		wxPoint p = GetPixFromLL(geo::Position(pfp[0], pfp[1]));
		pp[ip] = p;
		pfp += 2;
	}

#ifdef __WXGTK__
	sigaction(SIGSEGV, NULL, &sa_all_old); // save existing action for this signal

	struct sigaction temp;
	sigaction(SIGSEGV, NULL, &temp); // inspect existing action for this signal

	temp.sa_handler = catch_signals; // point to my handler
	sigemptyset(&temp.sa_mask); // make the blocking set
	// empty, so that all
	// other signals will be
	// unblocked during my handler
	temp.sa_flags = 0;
	sigaction(SIGSEGV, &temp, NULL);

	if (sigsetjmp(env, 1)) { //  Something in the below code block faulted....
		sigaction(SIGSEGV, &sa_all_old, NULL); // reset signal handler
		return Region;
	} else {
		OCPNRegion r = OCPNRegion(n, pp);
		if (NULL == ppoints)
			delete[] pp;

		sigaction(SIGSEGV, &sa_all_old, NULL); // reset signal handler
		r.Intersect(Region);
		return r;
	}

#else
	OCPNRegion r = OCPNRegion(n, pp);
	if (NULL == ppoints)
		delete[] pp;

	r.Intersect(Region);
	return r;
#endif
}

wxRect ViewPort::GetVPRectIntersect(size_t n, const float* llpoints) const
{
	// Calculate the intersection between the currect VP screen
	// and the bounding box of a polygon specified by lat/lon points.

	const float* pfp = llpoints;

	geo::BoundingBox point_box;
	for (unsigned int ip = 0; ip < n; ip++) {
		point_box.Expand(pfp[1], pfp[0]);
		pfp += 2;
	}

	wxPoint pul = GetPixFromLL(geo::Position(point_box.GetMaxY(), point_box.GetMinX()));
	wxPoint plr = GetPixFromLL(geo::Position(point_box.GetMinY(), point_box.GetMaxX()));

	OCPNRegion r(pul, plr);
	OCPNRegion rs(rv_rect);

	r.Intersect(rs);

	return r.GetBox();
}

void ViewPort::SetBoxes(void)
{
	// In the case where canvas rotation is applied, we need to define a larger "virtual" pixel
	// window size to ensure that
	// enough chart data is fatched and available to fill the rotated screen.
	rv_rect = wxRect(0, 0, pix_width, pix_height);

	// Specify the minimum required rectangle in unrotated screen space which will supply full
	// screen data after specified rotation
	if ((g_bskew_comp && (fabs(skew) > 0.001)) || (fabs(rotation) > 0.001)) {

		double rotator = rotation;
		rotator -= skew;

		int dy = wxRound(fabs(pix_height * cos(rotator)) + fabs(pix_width * sin(rotator)));
		int dx = wxRound(fabs(pix_width * cos(rotator)) + fabs(pix_height * sin(rotator)));

		// It is important for MSW build that viewport pixel dimensions be multiples of 4.....
		if (dy % 4)
			dy += 4 - (dy % 4);
		if (dx % 4)
			dx += 4 - (dx % 4);

		// Grow the source rectangle appropriately
		if (fabs(rotator) > 0.001)
			rv_rect.Inflate((dx - pix_width) / 2, (dy - pix_height) / 2);
	}

	// Compute Viewport lat/lon reference points for co-ordinate hit testing

	// This must be done in unrotated space with respect to full unrotated screen space calculated
	// above
	double rotation_save = rotation;
	SetRotationAngle(0.0);

	geo::Position pos_ul = GetLLFromPix(wxPoint(rv_rect.x, rv_rect.y));
	double lat_ul = pos_ul.lat();
	double lon_ul = pos_ul.lon();

	geo::Position pos_ur = GetLLFromPix(wxPoint(rv_rect.x + rv_rect.width, rv_rect.y));
	double lat_ur = pos_ur.lat();
	double lon_ur = pos_ur.lon();

	geo::Position pos_lr = GetLLFromPix(wxPoint(rv_rect.x + rv_rect.width, rv_rect.y + rv_rect.height));
	double lat_lr = pos_lr.lat();
	double lon_lr = pos_lr.lon();

	geo::Position pos_ll = GetLLFromPix(wxPoint(rv_rect.x, rv_rect.y + rv_rect.height));
	double lat_ll = pos_ll.lat();
	double lon_ll = pos_ll.lon();

	if (center_point.lon() < 0.0) {
		if ((lon_ul > 0.0) && (lon_ur < 0.0)) {
			lon_ul -= 360.0;
			lon_ll -= 360.0;
		}
	} else {
		if ((lon_ul > 0.0) && (lon_ur < 0.0)) {
			lon_ur += 360.0;
			lon_lr += 360.0;
		}
	}

	if (lon_ur < lon_ul) {
		lon_ur += 360.0;
		lon_lr += 360.0;
	}

	if (lon_ur > 360.0) {
		lon_ur -= 360.0;
		lon_lr -= 360.0;
		lon_ul -= 360.0;
		lon_ll -= 360.0;
	}

	double dlat_min = lat_ul;
	dlat_min = fmin(dlat_min, lat_ur);
	dlat_min = fmin(dlat_min, lat_lr);
	dlat_min = fmin(dlat_min, lat_ll);

	double dlon_min = lon_ul;
	dlon_min = fmin(dlon_min, lon_ur);
	dlon_min = fmin(dlon_min, lon_lr);
	dlon_min = fmin(dlon_min, lon_ll);

	double dlat_max = lat_ul;
	dlat_max = fmax(dlat_max, lat_ur);
	dlat_max = fmax(dlat_max, lat_lr);
	dlat_max = fmax(dlat_max, lat_ll);

	double dlon_max = lon_ur;
	dlon_max = fmax(dlon_max, lon_ul);
	dlon_max = fmax(dlon_max, lon_lr);
	dlon_max = fmax(dlon_max, lon_ll);

	// Set the viewport lat/lon bounding box appropriately
	vpBBox.SetMin(dlon_min, dlat_min);
	vpBBox.SetMax(dlon_max, dlat_max);

	// Restore the rotation angle
	SetRotationAngle(rotation_save);
}

void ViewPort::SetBBoxDirect(double latmin, double lonmin, double latmax, double lonmax)
{
	vpBBox.SetMin(lonmin, latmin);
	vpBBox.SetMax(lonmax, latmax);
}

const geo::Position& ViewPort::get_position() const
{
	return center_point;
}

void ViewPort::set_position(const geo::Position& pos)
{
	center_point = pos;
}

double ViewPort::latitude() const
{
	return center_point.lat();
}

double ViewPort::longitude() const
{
	return center_point.lon();
}

void ViewPort::Invalidate()
{
	valid = false;
}

void ViewPort::Validate()
{
	valid = true;
}

bool ViewPort::IsValid() const
{
	return valid;
}

void ViewPort::SetRotationAngle(double angle_rad)
{
	rotation = angle_rad;
}

void ViewPort::SetProjectionType(int type)
{
	m_projection_type = type;
}

const geo::LatLonBoundingBox& ViewPort::GetBBox() const
{
	return vpBBox;
}

geo::LatLonBoundingBox& ViewPort::GetBBox()
{
	return vpBBox;
}

void ViewPort::set_positive()
{
	while (GetBBox().GetMinX() < 0) {
		center_point = geo::Position(center_point.lat(), center_point.lon() + 360.0);
		GetBBox().Translate(360.0, 0.0);
	}
}

