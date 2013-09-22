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

#include <MicrosoftCompatibility.h>

#include <geo/GeoRef.h>

#include "OCPNRegion.h"
#include "ProjectionType.h"

#ifndef __WXMSW__
	#include <signal.h>
	#include <setjmp.h>
#endif

extern bool g_bCourseUp;
extern bool g_bskew_comp;

#ifndef __WXMSW__
extern struct sigaction sa_all;
extern struct sigaction sa_all_old;
extern sigjmp_buf env; // the context saved by sigsetjmp();
void catch_signals(int signo);
#endif


ViewPort::ViewPort()
{
	bValid = false;
	skew = 0.;
	view_scale_ppm = 1;
	rotation = 0.;
	b_quilt = false;
	pix_height = pix_width = 0;
	b_MercatorProjectionOverride = false;
}

wxPoint ViewPort::GetPixFromLL(double lat, double lon) const
{
	double easting, northing;
	double xlon = lon;

	/*  Make sure lon and lon0 are same phase */
	if( xlon * clon < 0. ) {
		if( xlon < 0. ) xlon += 360.;
		else
			xlon -= 360.;
	}

	if( fabs( xlon - clon ) > 180. ) {
		if( xlon > clon ) xlon -= 360.;
		else
			xlon += 360.;
	}

	if( PROJECTION_TRANSVERSE_MERCATOR == m_projection_type ) {
		//    We calculate northings as referenced to the equator
		//    And eastings as though the projection point is midscreen.

		double tmeasting, tmnorthing;
		double tmceasting, tmcnorthing;
		toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );
		toTM( lat, xlon, 0., clon, &tmeasting, &tmnorthing );

		northing = tmnorthing - tmcnorthing;
		easting = tmeasting - tmceasting;
	} else if( PROJECTION_POLYCONIC == m_projection_type ) {

		//    We calculate northings as referenced to the equator
		//    And eastings as though the projection point is midscreen.
		double pceasting, pcnorthing;
		toPOLY( clat, clon, 0., clon, &pceasting, &pcnorthing );

		double peasting, pnorthing;
		toPOLY( lat, xlon, 0., clon, &peasting, &pnorthing );

		easting = peasting;
		northing = pnorthing - pcnorthing;
	} else
		toSM( lat, xlon, clat, clon, &easting, &northing );

	if (!wxFinite(easting) || !wxFinite(northing))
		return wxPoint( 0, 0 );

	double epix = easting * view_scale_ppm;
	double npix = northing * view_scale_ppm;
	double dxr = epix;
	double dyr = npix;

	//    Apply VP Rotation
	if( g_bCourseUp ) {
		dxr = epix * cos( rotation ) + npix * sin( rotation );
		dyr = npix * cos( rotation ) - epix * sin( rotation );
	}
	wxPoint r;
	//    We definitely need a round() function here
	r.x = (int) wxRound( ( pix_width / 2 ) + dxr );
	r.y = (int) wxRound( ( pix_height / 2 ) - dyr );

	return r;
}

wxPoint2DDouble ViewPort::GetDoublePixFromLL( double lat, double lon )
{
	double easting, northing;
	double xlon = lon;

	/*  Make sure lon and lon0 are same phase */
	if( xlon * clon < 0. ) {
		if( xlon < 0. ) xlon += 360.;
		else
			xlon -= 360.;
	}

	if( fabs( xlon - clon ) > 180. ) {
		if( xlon > clon ) xlon -= 360.;
		else
			xlon += 360.;
	}

	if( PROJECTION_TRANSVERSE_MERCATOR == m_projection_type ) {
		//    We calculate northings as referenced to the equator
		//    And eastings as though the projection point is midscreen.

		double tmeasting, tmnorthing;
		double tmceasting, tmcnorthing;
		toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );
		toTM( lat, xlon, 0., clon, &tmeasting, &tmnorthing );

		northing = tmnorthing - tmcnorthing;
		easting = tmeasting - tmceasting;
	} else if( PROJECTION_POLYCONIC == m_projection_type ) {

		//    We calculate northings as referenced to the equator
		//    And eastings as though the projection point is midscreen.
		double pceasting, pcnorthing;
		toPOLY( clat, clon, 0., clon, &pceasting, &pcnorthing );

		double peasting, pnorthing;
		toPOLY( lat, xlon, 0., clon, &peasting, &pnorthing );

		easting = peasting;
		northing = pnorthing - pcnorthing;
	}

	else
		toSM( lat, xlon, clat, clon, &easting, &northing );

	if( !wxFinite(easting) || !wxFinite(northing) ) return wxPoint( 0, 0 );

	double epix = easting * view_scale_ppm;
	double npix = northing * view_scale_ppm;
	double dxr = epix;
	double dyr = npix;

	//    Apply VP Rotation
	if( g_bCourseUp ) {
		dxr = epix * cos( rotation ) + npix * sin( rotation );
		dyr = npix * cos( rotation ) - epix * sin( rotation );
	}

	wxPoint2DDouble r;
	//    We definitely need a round() function here
	r.m_x = ( ( pix_width / 2 ) + dxr );
	r.m_y = ( ( pix_height / 2 ) - dyr );

	return r;
}

void ViewPort::GetLLFromPix( const wxPoint &p, double *lat, double *lon )
{
	int dx = p.x - ( pix_width / 2 );
	int dy = ( pix_height / 2 ) - p.y;

	double xpr = dx;
	double ypr = dy;

	//    Apply VP Rotation
	if( g_bCourseUp ) {
		xpr = ( dx * cos( rotation ) ) - ( dy * sin( rotation ) );
		ypr = ( dy * cos( rotation ) ) + ( dx * sin( rotation ) );
	}
	double d_east = xpr / view_scale_ppm;
	double d_north = ypr / view_scale_ppm;

	double slat, slon;
	if( PROJECTION_TRANSVERSE_MERCATOR == m_projection_type ) {
		double tmceasting, tmcnorthing;
		toTM( clat, clon, 0., clon, &tmceasting, &tmcnorthing );

		fromTM( d_east, d_north + tmcnorthing, 0., clon, &slat, &slon );
	} else if( PROJECTION_POLYCONIC == m_projection_type ) {
		double polyeasting, polynorthing;
		toPOLY( clat, clon, 0., clon, &polyeasting, &polynorthing );

		fromPOLY( d_east, d_north + polynorthing, 0., clon, &slat, &slon );
	}

	//TODO  This could be fromSM_ECC to better match some Raster charts
	//      However, it seems that cm93 (and S57) prefer no eccentricity correction
	//      Think about it....
	else
		fromSM( d_east, d_north, clat, clon, &slat, &slon );

	*lat = slat;

	if( slon < -180. ) slon += 360.;
	else if( slon > 180. ) slon -= 360.;
	*lon = slon;
}

OCPNRegion ViewPort::GetVPRegionIntersect(
		const OCPNRegion & Region,
		size_t n,
		float * llpoints,
		int chart_native_scale,
		wxPoint * ppoints)
{
	//  Calculate the intersection between a given OCPNRegion (Region) and a polygon specified by lat/lon points.

	//    If the viewpoint is highly overzoomed wrt to chart native scale, the polygon region may be huge.
	//    This can be very expensive, and lead to crashes on some platforms (gtk in particular)
	//    So, look for this case and handle appropriately with respect to the given Region

	if( chart_scale < chart_native_scale / 10 ) {
		//    Make a positive definite vp
		ViewPort vp_positive = *this;
		while( vp_positive.vpBBox.GetMinX() < 0 ) {
			vp_positive.clon += 360.;
			wxPoint2DDouble t( 360., 0. );
			vp_positive.vpBBox.Translate( t );
		}

		//    Scan the points one-by-one, so that we can get min/max to make a bbox
		float *pfp = llpoints;
		float lon_max = -10000.;
		float lon_min = 10000.;
		float lat_max = -10000.;
		float lat_min = 10000.;

		for( unsigned int ip = 0; ip < n; ip++ ) {
			lon_max = wxMax(lon_max, pfp[1]);
			lon_min = wxMin(lon_min, pfp[1]);
			lat_max = wxMax(lat_max, pfp[0]);
			lat_min = wxMin(lat_min, pfp[0]);

			pfp += 2;
		}

		BoundingBox chart_box( lon_min, lat_min, lon_max, lat_max );

		//    Case:  vpBBox is completely outside the chart box, or vice versa
		//    Return an empty region
		if( _OUT == chart_box.Intersect( (BoundingBox&) vp_positive.vpBBox ) ) {
			if( _OUT == chart_box.Intersect( (BoundingBox&) vpBBox ) ) {
				// try again with the chart translated 360
				wxPoint2DDouble rtw( 360., 0. );
				BoundingBox trans_box = chart_box;
				trans_box.Translate( rtw );

				if( _OUT == trans_box.Intersect( (BoundingBox&) vp_positive.vpBBox ) ) {
					if( _OUT == trans_box.Intersect( (BoundingBox&) vpBBox ) ) {
						return OCPNRegion();
					}
				}
			}
		}

		//    Case:  vpBBox is completely inside the chart box
		//      Note that this test is not perfect, and will fail for some charts.
		//      The chart coverage may be  essentially triangular, and the viewport box
		//      may be in the "cut off" segment of the chart_box, and not actually
		//      exhibit any true overlap.  Results will be reported incorrectly.
		//      How to fix: maybe scrub the chart points and see if it is likely that
		//      a region may be safely built and intersection tested.

		if( _IN == chart_box.Intersect( (BoundingBox&) vp_positive.vpBBox ) ) {
			return Region;
		}

		if(_IN == chart_box.Intersect((BoundingBox&)vpBBox))
		{
			return Region;
		}

		//    The ViewPort and the chart region overlap in some way....
		//    Create the intersection of the two bboxes
		//    Boxes must be same phase
		while( chart_box.GetMinX() < 0 ) {
			wxPoint2DDouble t( 360., 0. );
			chart_box.Translate( t );
		}

		double cb_minlon = wxMax(chart_box.GetMinX(), vp_positive.vpBBox.GetMinX());
		double cb_maxlon = wxMin(chart_box.GetMaxX(), vp_positive.vpBBox.GetMaxX());
		double cb_minlat = wxMax(chart_box.GetMinY(), vp_positive.vpBBox.GetMinY());
		double cb_maxlat = wxMin(chart_box.GetMaxY(), vp_positive.vpBBox.GetMaxY());

		if( cb_maxlon < cb_minlon ) cb_maxlon += 360.;

		wxPoint p1 = GetPixFromLL( cb_maxlat, cb_minlon );  // upper left
		wxPoint p2 = GetPixFromLL( cb_minlat, cb_maxlon );   // lower right

		OCPNRegion r( p1, p2 );
		r.Intersect( Region );
		return r;
	}

	//    More "normal" case

	wxPoint *pp;

	//    Use the passed point buffer if available
	if( ppoints == NULL ) pp = new wxPoint[n];
	else
		pp = ppoints;

	float *pfp = llpoints;

	for( unsigned int ip = 0; ip < n; ip++ ) {
		wxPoint p = GetPixFromLL( pfp[0], pfp[1] );
		pp[ip] = p;
		pfp += 2;
	}

#ifdef __WXGTK__
	sigaction(SIGSEGV, NULL, &sa_all_old);             // save existing action for this signal

	struct sigaction temp;
	sigaction(SIGSEGV, NULL, &temp);// inspect existing action for this signal

	temp.sa_handler = catch_signals;// point to my handler
	sigemptyset(&temp.sa_mask);// make the blocking set
	// empty, so that all
	// other signals will be
	// unblocked during my handler
	temp.sa_flags = 0;
	sigaction(SIGSEGV, &temp, NULL);

	if(sigsetjmp(env, 1))//  Something in the below code block faulted....
	{
		sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler

		return Region;

	}

	else
	{

		OCPNRegion r = OCPNRegion(n, pp);
		if(NULL == ppoints)
			delete[] pp;

		sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler
		r.Intersect(Region);
		return r;
	}

#else
	OCPNRegion r = OCPNRegion( n, pp );

	if( NULL == ppoints ) delete[] pp;

	r.Intersect( Region );
	return r;

#endif
}

wxRect ViewPort::GetVPRectIntersect(size_t n, float * llpoints)
{
	//  Calculate the intersection between the currect VP screen
	//  and the bounding box of a polygon specified by lat/lon points.

	float *pfp = llpoints;

	BoundingBox point_box;
	for( unsigned int ip = 0; ip < n; ip++ ) {
		point_box.Expand(pfp[1], pfp[0]);
		pfp += 2;
	}

	wxPoint pul = GetPixFromLL( point_box.GetMaxY(), point_box.GetMinX() );
	wxPoint plr = GetPixFromLL( point_box.GetMinY(), point_box.GetMaxX() );

	OCPNRegion r( pul, plr );
	OCPNRegion rs(rv_rect);

	r.Intersect(rs);

	return r.GetBox();
}

void ViewPort::SetBoxes( void )
{

	//  In the case where canvas rotation is applied, we need to define a larger "virtual" pixel window size to ensure that
	//  enough chart data is fatched and available to fill the rotated screen.
	rv_rect = wxRect( 0, 0, pix_width, pix_height );

	//  Specify the minimum required rectangle in unrotated screen space which will supply full screen data after specified rotation
	if( ( g_bskew_comp && ( fabs( skew ) > .001 ) ) || ( fabs( rotation ) > .001 ) ) {

		double rotator = rotation;
		rotator -= skew;

		int dy = wxRound(fabs( pix_height * cos( rotator ) ) + fabs( pix_width * sin(rotator)));
		int dx = wxRound(fabs( pix_width * cos( rotator ) ) + fabs( pix_height * sin(rotator)));

		//  It is important for MSW build that viewport pixel dimensions be multiples of 4.....
		if( dy % 4 ) dy += 4 - ( dy % 4 );
		if( dx % 4 ) dx += 4 - ( dx % 4 );

		//  Grow the source rectangle appropriately
		if( fabs( rotator ) > .001 )
			rv_rect.Inflate( ( dx - pix_width ) / 2, ( dy - pix_height ) / 2 );
	}

	//  Compute Viewport lat/lon reference points for co-ordinate hit testing

	//  This must be done in unrotated space with respect to full unrotated screen space calculated above
	double rotation_save = rotation;
	SetRotationAngle( 0. );

	double lat_ul, lat_ur, lat_lr, lat_ll;
	double lon_ul, lon_ur, lon_lr, lon_ll;

	GetLLFromPix( wxPoint( rv_rect.x, rv_rect.y ), &lat_ul, &lon_ul );
	GetLLFromPix( wxPoint( rv_rect.x + rv_rect.width, rv_rect.y ), &lat_ur, &lon_ur );
	GetLLFromPix( wxPoint( rv_rect.x + rv_rect.width, rv_rect.y + rv_rect.height ), &lat_lr,
			&lon_lr );
	GetLLFromPix( wxPoint( rv_rect.x, rv_rect.y + rv_rect.height ), &lat_ll, &lon_ll );

	if( clon < 0.0) {
		if( ( lon_ul > 0.0) && ( lon_ur < 0.0) ) {
			lon_ul -= 360.0;
			lon_ll -= 360.0;
		}
	} else {
		if( ( lon_ul > 0.0) && ( lon_ur < 0.0) ) {
			lon_ur += 360.0;
			lon_lr += 360.0;
		}
	}

	if( lon_ur < lon_ul ) {
		lon_ur += 360.0;
		lon_lr += 360.0;
	}

	if( lon_ur > 360.0) {
		lon_ur -= 360.0;
		lon_lr -= 360.0;
		lon_ul -= 360.0;
		lon_ll -= 360.0;
	}

	double dlat_min = lat_ul;
	dlat_min = fmin ( dlat_min, lat_ur );
	dlat_min = fmin ( dlat_min, lat_lr );
	dlat_min = fmin ( dlat_min, lat_ll );

	double dlon_min = lon_ul;
	dlon_min = fmin ( dlon_min, lon_ur );
	dlon_min = fmin ( dlon_min, lon_lr );
	dlon_min = fmin ( dlon_min, lon_ll );

	double dlat_max = lat_ul;
	dlat_max = fmax ( dlat_max, lat_ur );
	dlat_max = fmax ( dlat_max, lat_lr );
	dlat_max = fmax ( dlat_max, lat_ll );

	double dlon_max = lon_ur;
	dlon_max = fmax ( dlon_max, lon_ul );
	dlon_max = fmax ( dlon_max, lon_lr );
	dlon_max = fmax ( dlon_max, lon_ll );

	//  Set the viewport lat/lon bounding box appropriately
	vpBBox.SetMin( dlon_min, dlat_min );
	vpBBox.SetMax( dlon_max, dlat_max );

	// Restore the rotation angle
	SetRotationAngle( rotation_save );
}

void ViewPort::Invalidate()
{
	bValid = false;
}

void ViewPort::Validate()
{
	bValid = true;
}

bool ViewPort::IsValid() const
{
	return bValid;
}

void ViewPort::SetRotationAngle(double angle_rad)
{
	rotation = angle_rad;
}

void ViewPort::SetProjectionType(int type)
{
	m_projection_type = type;
}

const LatLonBoundingBox & ViewPort::GetBBox() const
{
	return vpBBox;
}

LatLonBoundingBox & ViewPort::GetBBox()
{
	return vpBBox;
}

void ViewPort::set_positive()
{
	wxPoint2DDouble t(360.0, 0.0);
	while (GetBBox().GetMinX() < 0) {
		clon += 360.0;
		GetBBox().Translate(t);
	}
}

