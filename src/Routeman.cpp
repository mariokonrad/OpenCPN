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

#include "Routeman.h"
#include "dychart.h"
#include <StyleManager.h>
#include <CDI.h>
#include <WayPointman.h>
#include <ConsoleCanvas.h>
#include <RouteProp.h>
#include <RouteManagerDialog.h>
#include <Multiplexer.h>
#include <Select.h>
#include <MarkIcon.h>
#include <Config.h>
#include <Vector2D.h>
#include <UserColors.h>
#include <App.h>

#include <geo/GeoRef.h>

#include <global/OCPN.h>
#include <global/Navigation.h>

#include <plugin/PlugInManager.h>

#include <cstdlib>
#include <cmath>
#include <ctime>

#include <wx/image.h>
#include <wx/tokenzr.h>
#include <wx/progdlg.h>
#include <wx/listimpl.cpp>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/apptrait.h>

extern ConsoleCanvas* console;
extern RouteList* pRouteList;
extern Select* pSelect;
extern Config* pConfig;

extern wxRect g_blink_rect;

extern bool g_bMagneticAPB;

extern Track* g_pActiveTrack;
extern RouteProp* pRoutePropDialog;
extern RouteManagerDialog* pRouteManagerDialog;
extern int g_route_line_width;
extern Multiplexer* g_pMUX;

extern PlugInManager* g_pi_manager;
extern wxString g_uploadConnection;

Routeman::Routeman(App* parent)
{
	m_pparent_app = parent;
	pActiveRoute = NULL;
	pActivePoint = NULL;
	pRouteActivatePoint = NULL;
}

Routeman::~Routeman()
{
	if (pRouteActivatePoint)
		delete pRouteActivatePoint;
}

Route* Routeman::RouteExists(const wxString& guid) const
{
	// FIXME: use std::find
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (guid == route->m_GUID)
			return route;
	}
	return NULL;
}

bool Routeman::RouteExists(Route * route) const
{
	// FIXME: use std::find
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		if (*i == route)
			return true;
	}
	return false;
}

bool Routeman::IsRouteValid(Route * pRoute) const
{
	return RouteExists(pRoute);
}

// Make a 2-D search to find the route containing a given waypoint
Route* Routeman::FindRouteContainingWaypoint(RoutePoint* pWP)
{
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;

		for (RoutePointList::const_iterator point = route->pRoutePointList->begin();
			 point != route->pRoutePointList->end(); ++point) {
			if (*point == pWP)
				return route;
		}
	}

	return NULL;
}

Routeman::RouteArray * Routeman::GetRouteArrayContaining(RoutePoint * pWP) // FIXME: return std container
{
	RouteArray* pArray = new RouteArray;

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		for (RoutePointList::iterator waypoint = route->pRoutePointList->begin();
			 waypoint != route->pRoutePointList->end(); ++waypoint) {
			if (*waypoint == pWP)
				pArray->push_back((void*)route);
		}
	}

	if (pArray->size()) {
		return pArray;
	} else {
		delete pArray;
		return NULL;
	}
}

RoutePoint* Routeman::FindBestActivatePoint(Route* pR, double lat, double lon, double cog,
											double WXUNUSED(sog))
{
	if (!pR)
		return NULL;

	// Walk thru all the points to find the "best"
	RoutePoint* best_point = NULL;
	double min_time_found = 1e6;

	for (RoutePointList::iterator i = pR->pRoutePointList->begin(); i != pR->pRoutePointList->end();
		 ++i) {

		RoutePoint* pn = *i;

		double brg;
		double dist;
		geo::DistanceBearingMercator(pn->m_lat, pn->m_lon, lat, lon, &brg, &dist);

		double angle = brg - cog;
		double soa = cos(angle * M_PI / 180.0);

		double time_to_wp = dist / soa;

		if (time_to_wp > 0) {
			if (time_to_wp < min_time_found) {
				min_time_found = time_to_wp;
				best_point = pn;
			}
		}
	}
	return best_point;
}

bool Routeman::ActivateRoute(Route* pRouteToActivate, RoutePoint* pStartPoint)
{
	pActiveRoute = pRouteToActivate;

	if (pStartPoint) {
		pActivePoint = pStartPoint;
	} else {
		pActivePoint = pActiveRoute->pRoutePointList->front();
	}

	wxJSONValue v;
	v[_T("Route_activated")] = pRouteToActivate->m_RouteNameString;
	v[_T("GUID")] = pRouteToActivate->m_GUID;
	wxString msg_id(_T("OCPN_RTE_ACTIVATED"));
	g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);

	ActivateRoutePoint(pRouteToActivate, pActivePoint);

	m_bArrival = false;
	m_arrival_min = 1e6;
	m_arrival_test = 0;

	pRouteToActivate->m_bRtIsActive = true;

	m_bDataValid = false;

	console->ShowWithFreshFonts();

	return true;
}

bool Routeman::ActivateRoutePoint(Route* pA, RoutePoint* pRP_target)
{
	wxJSONValue v;
	pActiveRoute = pA;

	pActivePoint = pRP_target;
	pActiveRoute->m_pRouteActivePoint = pRP_target;

	v[_T("GUID")] = pRP_target->m_GUID;
	v[_T("WP_activated")] = pRP_target->GetName();

	for (RoutePointList::iterator point = pActiveRoute->pRoutePointList->begin();
		 point != pActiveRoute->pRoutePointList->end(); ++point) {
		RoutePoint* pn = *point;
		pn->m_bBlink = false; // turn off all blinking points
		pn->m_bIsActive = false;
	}

	RoutePointList::iterator point = pActiveRoute->pRoutePointList->begin();
	RoutePoint* prp_first = *point;

	//  If activating first point in route, create a "virtual" waypoint at present position
	if (pRP_target == prp_first) {
		if (pRouteActivatePoint)
			delete pRouteActivatePoint;

		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		// Current location
		pRouteActivatePoint = new RoutePoint(nav.lat, nav.lon, _T(""), _T(""), _T(""), false);
		pRouteActivatePoint->m_bShowName = false;

		pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
	} else {
		prp_first->m_bBlink = false;
		++point;
		RoutePoint* np_prev = prp_first;
		for (; point != pActiveRoute->pRoutePointList->end(); ++point) {
			if (*point == pRP_target) {
				pActiveRouteSegmentBeginPoint = np_prev;
				break;
			}
			np_prev = *point;
		}
	}

	pRP_target->m_bBlink = true; // blink the active point
	pRP_target->m_bIsActive = true; // and active

	g_blink_rect = pRP_target->CurrentRect_in_DC; // set up global blinker

	m_bArrival = false;
	m_arrival_min = 1e6;
	m_arrival_test = 0;

	// Update the RouteProperties Dialog, if currently shown
	if ((NULL != pRoutePropDialog) && (pRoutePropDialog->IsShown())) {
		if (pRoutePropDialog->getRoute() == pA) {
			if (pRoutePropDialog->getEnroutePoint())
				pRoutePropDialog->setEnroutePoint(
					pActivePoint); // FIXME: weird: if set, overwrite it?
			pRoutePropDialog->SetRouteAndUpdate(pA);
			pRoutePropDialog->UpdateProperties();
		}
	}

	wxString msg_id(_T("OCPN_WPT_ACTIVATED"));
	g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);

	return true;
}

bool Routeman::ActivateNextPoint(Route* pr, bool skipped)
{
	wxJSONValue v;
	if (pActivePoint) {
		pActivePoint->m_bBlink = false;
		pActivePoint->m_bIsActive = false;

		v[_T("isSkipped")] = skipped;
		v[_T("GUID")] = pActivePoint->m_GUID;
		v[_T("WP_arrived")] = pActivePoint->GetName();
	}
	int n_index_active = pActiveRoute->GetIndexOf(pActivePoint);
	if ((n_index_active + 1) <= pActiveRoute->GetnPoints()) {
		pActiveRouteSegmentBeginPoint = pActivePoint;

		pActiveRoute->m_pRouteActivePoint = pActiveRoute->GetPoint(n_index_active + 1);

		pActivePoint = pActiveRoute->GetPoint(n_index_active + 1);
		v[_T("Next_WP")] = pActivePoint->GetName();
		v[_T("GUID")] = pActivePoint->m_GUID;

		pActivePoint->m_bBlink = true;
		pActivePoint->m_bIsActive = true;
		g_blink_rect = pActivePoint->CurrentRect_in_DC; // set up global blinker

		m_bArrival = false;

		// Update the RouteProperties Dialog, if currently shown
		if ((NULL != pRoutePropDialog) && (pRoutePropDialog->IsShown())) {
			if (pRoutePropDialog->getRoute() == pr) {
				if (pRoutePropDialog->getEnroutePoint())
					pRoutePropDialog->setEnroutePoint(
						pActivePoint); // FIXME: weird: if set, overwrite it?
				pRoutePropDialog->SetRouteAndUpdate(pr);
				pRoutePropDialog->UpdateProperties();
			}
		}

		wxString msg_id(_T("OCPN_WPT_ARRIVED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);

		return true;
	}

	return false;
}

bool Routeman::UpdateProgress()
{
	bool bret_val = false;

	if (pActiveRoute) {
		// Update bearing, range, and crosstrack error

		// Bearing is calculated as Mercator Sailing, i.e. a  cartographic "bearing"
		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		double north;
		double east;
		geo::toSM(pActivePoint->m_lat, pActivePoint->m_lon, nav.lat, nav.lon, &east, &north);
		double a = atan(north / east);
		if (fabs(pActivePoint->m_lon - nav.lon) < 180.0) {
			if (pActivePoint->m_lon > nav.lon)
				CurrentBrgToActivePoint = 90.0 - (a * 180.0 / M_PI);
			else
				CurrentBrgToActivePoint = 270.0 - (a * 180.0 / M_PI);
		} else {
			if (pActivePoint->m_lon > nav.lon)
				CurrentBrgToActivePoint = 270.0 - (a * 180.0 / M_PI);
			else
				CurrentBrgToActivePoint = 90.0 - (a * 180.0 / M_PI);
		}

		// Calculate range using Great Circle Formula

		double d5
			= geo::DistGreatCircle(nav.lat, nav.lon, pActivePoint->m_lat, pActivePoint->m_lon);
		CurrentRngToActivePoint = d5;

		// Get the XTE vector, normal to current segment
		Vector2D va, vb, vn;

		double brg1, dist1, brg2, dist2;
		geo::DistanceBearingMercator(pActivePoint->m_lat, pActivePoint->m_lon,
									 pActiveRouteSegmentBeginPoint->m_lat,
									 pActiveRouteSegmentBeginPoint->m_lon, &brg1, &dist1);
		vb.x = dist1 * sin(brg1 * M_PI / 180.0);
		vb.y = dist1 * cos(brg1 * M_PI / 180.0);

		geo::DistanceBearingMercator(pActivePoint->m_lat, pActivePoint->m_lon, nav.lat, nav.lon,
									 &brg2, &dist2);
		va.x = dist2 * sin(brg2 * M_PI / 180.0);
		va.y = dist2 * cos(brg2 * M_PI / 180.0);

		double sdelta = vGetLengthOfNormal(&va, &vb, &vn); // NM
		CurrentXTEToActivePoint = sdelta;

		// Calculate the distance to the arrival line, which is perpendicular to the current
		// route segment
		// Taking advantage of the calculated normal from current position to route segment vn
		Vector2D vToArriveNormal;
		vSubtractVectors(&va, &vn, &vToArriveNormal);

		CurrentRangeToActiveNormalCrossing = vVectorMagnitude(&vToArriveNormal);

		// Compute current segment course
		// Using simple Mercater projection
		double x1;
		double y1;
		double x2;
		double y2;
		geo::toSM(pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon,
				  pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon, &x1,
				  &y1);

		geo::toSM(pActivePoint->m_lat, pActivePoint->m_lon, pActiveRouteSegmentBeginPoint->m_lat,
				  pActiveRouteSegmentBeginPoint->m_lon, &x2, &y2);

		double e1 = atan2((x2 - x1), (y2 - y1));
		CurrentSegmentCourse = e1 * 180.0 / M_PI;
		if (CurrentSegmentCourse < 0)
			CurrentSegmentCourse += 360;

		//      Compute XTE direction
		double h = atan(vn.y / vn.x);
		if (vn.x > 0)
			CourseToRouteSegment = 90.0 - (h * 180.0 / M_PI);
		else
			CourseToRouteSegment = 270. - (h * 180.0 / M_PI);

		h = CurrentBrgToActivePoint - CourseToRouteSegment;
		if (h < 0)
			h = h + 360;

		if (h > 180)
			XTEDir = 1;
		else
			XTEDir = -1;

		//      Determine Arrival

		bool bDidArrival = false;

		if (CurrentRangeToActiveNormalCrossing <= pActiveRoute->GetRouteArrivalRadius()) {
			m_bArrival = true;
			UpdateAutopilot();

			bDidArrival = true;
			DoAdvance();
		} else {
			// Test to see if we are moving away from the arrival point, and
			// have been mving away for 2 seconds.
			// If so, we should declare "Arrival"
			if ((CurrentRangeToActiveNormalCrossing - m_arrival_min)
				> pActiveRoute->GetRouteArrivalRadius()) {
				if (++m_arrival_test > 2) {
					m_bArrival = true;
					UpdateAutopilot();

					bDidArrival = true;
					DoAdvance();
				}
			} else
				m_arrival_test = 0;
		}

		if (!bDidArrival)
			m_arrival_min = wxMin(m_arrival_min, CurrentRangeToActiveNormalCrossing);

		if (!bDidArrival) // Only once on arrival
			UpdateAutopilot();

		bret_val = true; // a route is active
	}

	m_bDataValid = true;

	return bret_val;
}

void Routeman::DoAdvance(void)
{
	if (!ActivateNextPoint(pActiveRoute, false)) // at the end?
	{
		Route* pthis_route = pActiveRoute;
		DeactivateRoute(true); // this is an arrival

		if (pthis_route->m_bDeleteOnArrival) {
			pConfig->DeleteConfigRoute(pthis_route);
			DeleteRoute(pthis_route);
			if (pRoutePropDialog) {
				pRoutePropDialog->SetRouteAndUpdate(NULL);
				pRoutePropDialog->UpdateProperties();
			}
		}

		if (pRouteManagerDialog)
			pRouteManagerDialog->UpdateRouteListCtrl();
	}
}

bool Routeman::IsAnyRouteActive(void) const
{
	return pActiveRoute != NULL;
}

Route* Routeman::GetpActiveRoute()
{
	return pActiveRoute;
}

RoutePoint* Routeman::GetpActivePoint()
{
	return pActivePoint;
}

double Routeman::GetCurrentRngToActivePoint() const
{
	return CurrentRngToActivePoint;
}

double Routeman::GetCurrentBrgToActivePoint() const
{
	return CurrentBrgToActivePoint;
}

double Routeman::GetCurrentRngToActiveNormalArrival() const
{
	return CurrentRangeToActiveNormalCrossing;
}

double Routeman::GetCurrentXTEToActivePoint() const
{
	return CurrentXTEToActivePoint;
}

double Routeman::GetCurrentSegmentCourse() const
{
	return CurrentSegmentCourse;
}

int Routeman::GetXTEDir() const
{
	return XTEDir;
}

wxPen* Routeman::GetRoutePen(void)
{
	return m_pRoutePen;
}

wxPen* Routeman::GetSelectedRoutePen(void)
{
	return m_pSelectedRoutePen;
}

wxPen* Routeman::GetActiveRoutePen(void)
{
	return m_pActiveRoutePen;
}

wxPen* Routeman::GetActiveRoutePointPen(void)
{
	return m_pActiveRoutePointPen;
}

wxPen* Routeman::GetRoutePointPen(void)
{
	return m_pRoutePointPen;
}

wxBrush* Routeman::GetRouteBrush(void)
{
	return m_pRouteBrush;
}

wxBrush* Routeman::GetSelectedRouteBrush(void)
{
	return m_pSelectedRouteBrush;
}

wxBrush* Routeman::GetActiveRouteBrush(void)
{
	return m_pActiveRouteBrush;
}

wxBrush* Routeman::GetActiveRoutePointBrush(void)
{
	return m_pActiveRoutePointBrush;
}

wxBrush* Routeman::GetRoutePointBrush(void)
{
	return m_pRoutePointBrush;
}

bool Routeman::DeactivateRoute(bool b_arrival)
{
	if (pActivePoint) {
		pActivePoint->m_bBlink = false;
		pActivePoint->m_bIsActive = false;
	}

	if (pActiveRoute) {
		pActiveRoute->m_bRtIsActive = false;
		pActiveRoute->m_pRouteActivePoint = NULL;
	}

	wxJSONValue v;
	if (!b_arrival) {
		v[_T("Route_deactivated")] = pActiveRoute->m_RouteNameString;
		v[_T("GUID")] = pActiveRoute->m_GUID;
		wxString msg_id(_T("OCPN_RTE_DEACTIVATED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
	} else {
		v[_T("GUID")] = pActiveRoute->m_GUID;
		v[_T("Route_ended")] = pActiveRoute->m_RouteNameString;
		wxString msg_id(_T("OCPN_RTE_ENDED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
	}

	pActiveRoute = NULL;

	if (pRouteActivatePoint)
		delete pRouteActivatePoint;
	pRouteActivatePoint = NULL;

	console->pCDI->ClearBackground();
	console->Show(false);
	m_bDataValid = false;
	return true;
}

bool Routeman::UpdateAutopilot()
{
	// FIXME: split up method

	//Send all known Autopilot messages upstream

	//RMB
	{
		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;
		m_NMEA0183.Rmb.IsDataValid = NTrue;
		m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;

		if (XTEDir < 0)
			m_NMEA0183.Rmb.DirectionToSteer = Left;
		else
			m_NMEA0183.Rmb.DirectionToSteer = Right;

		m_NMEA0183.Rmb.To = pActivePoint->GetName().Truncate( 6 );
		m_NMEA0183.Rmb.From = pActiveRouteSegmentBeginPoint->GetName().Truncate( 6 );

		if (pActivePoint->m_lat < 0.0)
			m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(-pActivePoint->m_lat, _T("S"));
		else
			m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(pActivePoint->m_lat, _T("N"));

		if (pActivePoint->m_lon < 0.0)
			m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(-pActivePoint->m_lon, _T("W"));
		else
			m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(pActivePoint->m_lon, _T("E"));

		m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
		m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;
		m_NMEA0183.Rmb.DestinationClosingVelocityKnots = global::OCPN::get().nav().get_data().sog;

		if (m_bArrival)
			m_NMEA0183.Rmb.IsArrivalCircleEntered = NTrue;
		else
			m_NMEA0183.Rmb.IsArrivalCircleEntered = NFalse;

		m_NMEA0183.Rmb.Write( snt );

		g_pMUX->SendNMEAMessage( snt.Sentence );
	}

	// RMC
	{
		const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;
		m_NMEA0183.Rmc.IsDataValid = NTrue;

		if (nav.lat < 0.0)
			m_NMEA0183.Rmc.Position.Latitude.Set(-nav.lat, _T("S"));
		else
			m_NMEA0183.Rmc.Position.Latitude.Set(nav.lat, _T("N"));

		if (nav.lon < 0.0)
			m_NMEA0183.Rmc.Position.Longitude.Set(-nav.lon, _T("W"));
		else
			m_NMEA0183.Rmc.Position.Longitude.Set(nav.lon, _T("E"));

		m_NMEA0183.Rmc.SpeedOverGroundKnots = nav.sog;
		m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue = nav.cog;

		if (!wxIsNaN(nav.var)) {
			if (nav.var < 0.0) {
				m_NMEA0183.Rmc.MagneticVariation = -nav.var;
				m_NMEA0183.Rmc.MagneticVariationDirection = West;
			} else {
				m_NMEA0183.Rmc.MagneticVariation = nav.var;
				m_NMEA0183.Rmc.MagneticVariationDirection = East;
			}
		} else
			m_NMEA0183.Rmc.MagneticVariation = 361.; // A signal to NMEA converter, gVAR is unknown

		wxDateTime now = wxDateTime::Now();
		wxDateTime utc = now.ToUTC();
		wxString time = utc.Format( _T("%H%M%S") );
		m_NMEA0183.Rmc.UTCTime = time;

		wxString date = utc.Format( _T("%d%m%y") );
		m_NMEA0183.Rmc.Date = date;

		m_NMEA0183.Rmc.Write( snt );

		g_pMUX->SendNMEAMessage( snt.Sentence );
	}

	// APB
	{
		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;

		m_NMEA0183.Apb.IsLoranBlinkOK = NTrue;
		m_NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;

		m_NMEA0183.Apb.CrossTrackErrorMagnitude = CurrentXTEToActivePoint;

		if( XTEDir < 0 ) m_NMEA0183.Apb.DirectionToSteer = Left;
		else
			m_NMEA0183.Apb.DirectionToSteer = Right;

		m_NMEA0183.Apb.CrossTrackUnits = _T("N");

		if( m_bArrival )
			m_NMEA0183.Apb.IsArrivalCircleEntered = NTrue;
		else
			m_NMEA0183.Apb.IsArrivalCircleEntered = NFalse;

		//  We never pass the perpendicular, since we declare arrival before reaching this point
		m_NMEA0183.Apb.IsPerpendicular = NFalse;

		m_NMEA0183.Apb.To = pActivePoint->GetName().Truncate( 6 );

		double brg1, dist1;
		geo::DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon,
				pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon,
				&brg1,
				&dist1 );

		const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

		if( g_bMagneticAPB && !wxIsNaN(nav.var) ) {

			double brg1m = ((brg1 + nav.var) >= 0.0) ? (brg1 + nav.var) : (brg1 + nav.var + 360.0);
			double bapm = ((CurrentBrgToActivePoint + nav.var) >= 0.0)
				? (CurrentBrgToActivePoint + nav.var)
				: (CurrentBrgToActivePoint + nav.var + 360.0);

			m_NMEA0183.Apb.BearingOriginToDestination = brg1m;
			m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("M");

			m_NMEA0183.Apb.BearingPresentPositionToDestination = bapm;
			m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("M");

			m_NMEA0183.Apb.HeadingToSteer = bapm;
			m_NMEA0183.Apb.HeadingToSteerUnits = _T("M");
		}
		else {
			m_NMEA0183.Apb.BearingOriginToDestination = brg1;
			m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("T");

			m_NMEA0183.Apb.BearingPresentPositionToDestination = CurrentBrgToActivePoint;
			m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("T");


			m_NMEA0183.Apb.HeadingToSteer = CurrentBrgToActivePoint;
			m_NMEA0183.Apb.HeadingToSteerUnits = _T("T");
		}

		m_NMEA0183.Apb.Write( snt );
		g_pMUX->SendNMEAMessage( snt.Sentence );
	}


	return true;
}

bool Routeman::DoesRouteContainSharedPoints(Route* pRoute)
{
	if (!pRoute)
		return false;

	// walk the route, looking at each point to see if it is used by another route
	// or is isolated
	for (RoutePointList::iterator point = pRoute->pRoutePointList->begin();
		 point != pRoute->pRoutePointList->end(); ++point) {

		// check all other routes to see if this point appears in any other route
		RouteArray* pRA = GetRouteArrayContaining(*point);
		if (pRA) {
			for (unsigned int ir = 0; ir < pRA->size(); ++ir) {
				Route* route = static_cast<Route*>(pRA->Item(ir));
				if (route == pRoute)
					continue;
				else
					return true;
			}
		}
	}

	// Now walk the route again, looking for isolated type shared waypoints
	for (RoutePointList::const_iterator point = pRoute->pRoutePointList->begin();
		 point != pRoute->pRoutePointList->end(); ++point) {
		if ((*point)->m_bKeepXRoute == true)
			return true;
	}

	return false;
}

void Routeman::DeleteRoute(Route * pRoute)
{
	if (!pRoute)
		return;

	::wxBeginBusyCursor();

	if (GetpActiveRoute() == pRoute)
		DeactivateRoute();

	if (pRoute->m_bIsInLayer)
		return;

	//    Remove the route from associated lists
	pSelect->DeleteAllSelectableRouteSegments(pRoute);
	pRouteList->remove(pRoute);

	// walk the route, tentatively deleting/marking points used only by this route
	wxRoutePointListNode *pnode = pRoute->pRoutePointList->GetFirst();
	while (pnode) {
		RoutePoint *prp = pnode->GetData();

		// check all other routes to see if this point appears in any other route
		Route *pcontainer_route = FindRouteContainingWaypoint( prp );

		if (pcontainer_route == NULL && prp->m_bIsInRoute) {
			prp->m_bIsInRoute = false; // Take this point out of this (and only) route
			if (!prp->m_bKeepXRoute) {
				// This does not need to be done with navobj.xml storage, since the waypoints are stored with the route
				// pConfig->DeleteWayPoint(prp);

				pSelect->DeleteSelectablePoint(prp, SelectItem::TYPE_ROUTEPOINT);

				// Remove all instances of this point from the list.
				wxRoutePointListNode *pdnode = pnode;
				while (pdnode) {
					pRoute->pRoutePointList->DeleteNode(pdnode);
					pdnode = pRoute->pRoutePointList->Find(prp);
				}

				pnode = NULL;
				delete prp;
			} else {
				prp->m_bDynamicName = false;
				prp->m_bIsolatedMark = true; // This has become an isolated mark
				prp->m_bKeepXRoute = false; // and is no longer part of a route
			}
		}

		if (pnode)
			pnode = pnode->GetNext();
		else
			pnode = pRoute->pRoutePointList->GetFirst(); // restart the list
	}

	delete pRoute;

	::wxEndBusyCursor();
}

void Routeman::DeleteAllRoutes(void)
{
	// FIXME: almost code duplication of Routeman::DeleteAllTracks(void)

	::wxBeginBusyCursor();

	// delete routes that are in layer and also not a track
	// this algorithm alters the container

	RouteList::iterator i = pRouteList->begin();
	while (i != pRouteList->end()) {
		Route* proute = *i;

		if (proute->m_bIsInLayer) {
			++i;
			continue;
		}

		if (!proute->m_bIsTrack) {
			pConfig->m_bSkipChangeSetUpdate = true;
			pConfig->DeleteConfigRoute(proute);
			DeleteRoute(proute);
			i = pRouteList->begin();
			pConfig->m_bSkipChangeSetUpdate = false;
		} else {
			++i;
		}
	}

	::wxEndBusyCursor();
}

void Routeman::DeleteAllTracks(void)
{
	// FIXME: almost code duplication of Routeman::DeleteAllRoutes(void)

	::wxBeginBusyCursor();

	// delete routes that are in layer and also a track
	// this algorithm alters the container

	RouteList::iterator i = pRouteList->begin();
	while (i != pRouteList->end()) {
		Route* proute = *i;

		if (proute->m_bIsInLayer) {
			++i;
			continue;
		}

		if (proute->m_bIsTrack) {
			pConfig->m_bSkipChangeSetUpdate = true;
			pConfig->DeleteConfigRoute(proute);
			DeleteTrack(proute);
			i = pRouteList->begin();
			pConfig->m_bSkipChangeSetUpdate = false;
		} else {
			++i;
		}
	}

	::wxEndBusyCursor();
}

void Routeman::DeleteTrack(Route * pRoute)
{
	if (!pRoute)
		return;

	if (pRoute->m_bIsInLayer)
		return;

	::wxBeginBusyCursor();

	wxProgressDialog *pprog = NULL;
	int count = pRoute->pRoutePointList->GetCount();
	if (count > 200) {
		pprog = new wxProgressDialog(
			_("OpenCPN Track Delete"),
			_T("0/0"),
			count,
			NULL,
			wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
		pprog->SetSize(400, wxDefaultCoord);
		pprog->Centre();
	}

	// Remove the route from associated lists
	pSelect->DeleteAllSelectableTrackSegments(pRoute);
	pRouteList->remove(pRoute);

	// walk the route, tentatively deleting/marking points used only by this route
	int ic = 0;
	wxRoutePointListNode* pnode = (pRoute->pRoutePointList)->GetFirst();
	while (pnode) {
		if (pprog) {
			wxString msg;
			msg.Printf(_T("%d/%d"), ic, count);
			pprog->Update(ic, msg);
			ic++;
		}

		RoutePoint* prp = pnode->GetData();

		// check all other routes to see if this point appears in any other route
		Route* pcontainer_route = NULL; // FindRouteContainingWaypoint(prp);

		if (pcontainer_route == NULL) {
			prp->m_bIsInRoute = false; // Take this point out of this (and only) route
			if (!prp->m_bKeepXRoute) {
				pConfig->m_bSkipChangeSetUpdate = true;
				pConfig->DeleteWayPoint(prp);
				pSelect->DeleteSelectablePoint(prp, SelectItem::TYPE_ROUTEPOINT);
				pConfig->m_bSkipChangeSetUpdate = false;

				pRoute->pRoutePointList->DeleteNode(pnode);
				pnode = NULL;
				delete prp;
			}
		}
		if (pnode)
			pnode = pnode->GetNext();
		else
			pnode = pRoute->pRoutePointList->GetFirst(); // restart the list
	}

	if ((Track*)pRoute == g_pActiveTrack) {
		g_pActiveTrack = NULL;
		m_pparent_app->TrackOff();
	}

	delete pRoute;

	::wxEndBusyCursor();

	if (pprog)
		delete pprog;
}

void Routeman::SetColorScheme(ColorScheme)
{
	m_pActiveRoutePointPen
		= wxThePenList->FindOrCreatePen(wxColour(0, 0, 255), g_route_line_width, wxSOLID);
	m_pRoutePointPen
		= wxThePenList->FindOrCreatePen(wxColour(0, 0, 255), g_route_line_width, wxSOLID);

	// Or in something like S-52 compliance

	m_pRoutePen
		= wxThePenList->FindOrCreatePen(GetGlobalColor(_T("UINFB")), g_route_line_width, wxSOLID);
	m_pSelectedRoutePen
		= wxThePenList->FindOrCreatePen(GetGlobalColor(_T("UINFO")), g_route_line_width, wxSOLID);
	m_pActiveRoutePen
		= wxThePenList->FindOrCreatePen(GetGlobalColor(_T("UARTE")), g_route_line_width, wxSOLID);

	m_pRouteBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("UINFB")), wxSOLID);
	m_pSelectedRouteBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("UINFO")), wxSOLID);
	m_pActiveRouteBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("PLRTE")), wxSOLID);
}

wxString Routeman::GetRouteReverseMessage(void) const
{
	return wxString(_("Waypoints can be renamed to reflect the new order, the names will be '001', '002' etc.\n\nDo you want to rename the waypoints?"));
}

Route* Routeman::FindRouteByGUID(const wxString& guid) const
{
	return RouteExists(guid);
}

bool Routeman::is_data_valid() const
{
	return m_bDataValid;
}

