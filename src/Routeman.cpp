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
#include <ConsoleCanvas.h>
#include <RouteProp.h>
#include <RouteManagerDialog.h>
#include <Multiplexer.h>
#include <Select.h>
#include <Config.h>

#include <util/Vector2D.h>

#include <geo/GeoRef.h>

#include <gui/StyleManager.h>

#include <global/OCPN.h>
#include <global/Navigation.h>
#include <global/GUI.h>
#include <global/ColorManager.h>

#include <navigation/RouteTracker.h>

#include <plugin/PlugInManager.h>

#include <algorithm>

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

extern RouteProp* pRoutePropDialog;
extern RouteManagerDialog* pRouteManagerDialog;
extern Multiplexer* g_pMUX;

extern PlugInManager* g_pi_manager;

Routeman::Routeman()
	: pActiveRoute(NULL)
	, pActivePoint(NULL)
	, pRouteActivatePoint(NULL)
{
}

Routeman::~Routeman()
{
	if (pRouteActivatePoint)
		delete pRouteActivatePoint;
}

Route* Routeman::RouteExists(const wxString& guid) const
{
	RouteList::iterator i = std::find_if(pRouteList->begin(), pRouteList->end(), Route::SameGUID(guid));
	return i == pRouteList->end() ? NULL : *i;
}

bool Routeman::RouteExists(const Route* route) const
{
	RouteList::const_iterator i = std::find(pRouteList->begin(), pRouteList->end(), route);
	return i != pRouteList->end();
}

bool Routeman::IsRouteValid(const Route* pRoute) const
{
	return RouteExists(pRoute);
}

// Make a 2-D search to find the route containing a given waypoint
Route* Routeman::FindRouteContainingWaypoint(const RoutePoint* pWP) const
{
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;

		RoutePointList::const_iterator point
			= std::find(route->routepoints().begin(), route->routepoints().end(), pWP);
		if (point != route->routepoints().end())
			return route;
	}

	return NULL;
}

Routeman::RouteArray Routeman::GetRouteArrayContaining(const RoutePoint* pWP)
{
	RouteArray routes;

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		RoutePointList::const_iterator point
			= std::find(route->routepoints().begin(), route->routepoints().end(), pWP);
		if (point != route->routepoints().end())
			routes.push_back(route);
	}

	return routes;
}

bool Routeman::ActivateRoute(Route* pRouteToActivate, RoutePoint* pStartPoint)
{
	pActiveRoute = pRouteToActivate;

	if (pStartPoint) {
		pActivePoint = pStartPoint;
	} else {
		pActivePoint = pActiveRoute->routepoints().front();
	}

	wxJSONValue v;
	v[_T("Route_activated")] = pRouteToActivate->get_name();
	v[_T("GUID")] = pRouteToActivate->guid();
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

	v[_T("GUID")] = pRP_target->guid();
	v[_T("WP_activated")] = pRP_target->GetName();

	for (RoutePointList::iterator point = pActiveRoute->routepoints().begin();
		 point != pActiveRoute->routepoints().end(); ++point) {
		RoutePoint* pn = *point;
		pn->set_blink(false); // turn off all blinking points
		pn->m_bIsActive = false;
	}

	RoutePointList::iterator point = pActiveRoute->routepoints().begin();
	RoutePoint* prp_first = *point;

	// If activating first point in route, create a "virtual" waypoint at present position
	if (pRP_target == prp_first) {
		if (pRouteActivatePoint)
			delete pRouteActivatePoint;

		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		// Current location
		pRouteActivatePoint = new RoutePoint(nav.pos, _T(""), _T(""), _T(""), false);
		pRouteActivatePoint->set_show_name(false);

		pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
	} else {
		prp_first->set_blink(false);
		++point;
		RoutePoint* np_prev = prp_first;
		for (; point != pActiveRoute->routepoints().end(); ++point) {
			if (*point == pRP_target) {
				pActiveRouteSegmentBeginPoint = np_prev;
				break;
			}
			np_prev = *point;
		}
	}

	pRP_target->set_blink(true); // blink the active point
	pRP_target->m_bIsActive = true; // and active

	g_blink_rect = pRP_target->CurrentRect_in_DC; // set up global blinker

	m_bArrival = false;
	m_arrival_min = 1e6;
	m_arrival_test = 0;

	// Update the RouteProperties Dialog, if currently shown
	if ((NULL != pRoutePropDialog) && (pRoutePropDialog->IsShown())) {
		if (pRoutePropDialog->getRoute() == pA) {
			// FIXME: weird: if set, overwrite it?
			if (pRoutePropDialog->getEnroutePoint())
				pRoutePropDialog->setEnroutePoint(pActivePoint);
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
		pActivePoint->set_blink(false);
		pActivePoint->m_bIsActive = false;

		v[_T("isSkipped")] = skipped;
		v[_T("GUID")] = pActivePoint->guid();
		v[_T("WP_arrived")] = pActivePoint->GetName();
	}
	int n_index_active = pActiveRoute->GetIndexOf(pActivePoint);
	if ((n_index_active + 1) <= pActiveRoute->GetnPoints()) {
		pActiveRouteSegmentBeginPoint = pActivePoint;

		pActiveRoute->m_pRouteActivePoint = pActiveRoute->GetPoint(n_index_active + 1);

		pActivePoint = pActiveRoute->GetPoint(n_index_active + 1);
		v[_T("Next_WP")] = pActivePoint->GetName();
		v[_T("GUID")] = pActivePoint->guid();

		pActivePoint->set_blink(true);
		pActivePoint->m_bIsActive = true;
		g_blink_rect = pActivePoint->CurrentRect_in_DC; // set up global blinker

		m_bArrival = false;

		// Update the RouteProperties Dialog, if currently shown
		if ((NULL != pRoutePropDialog) && (pRoutePropDialog->IsShown())) {
			if (pRoutePropDialog->getRoute() == pr) {
				// FIXME: weird: if set, overwrite it?
				if (pRoutePropDialog->getEnroutePoint())
					pRoutePropDialog->setEnroutePoint(pActivePoint);
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
		geo::toSM(pActivePoint->get_position(), nav.pos, &east, &north);
		double a = atan(north / east);
		if (fabs(pActivePoint->longitude() - nav.pos.lon()) < 180.0) {
			if (pActivePoint->longitude() > nav.pos.lon())
				CurrentBrgToActivePoint = 90.0 - (a * 180.0 / M_PI);
			else
				CurrentBrgToActivePoint = 270.0 - (a * 180.0 / M_PI);
		} else {
			if (pActivePoint->longitude() > nav.pos.lon())
				CurrentBrgToActivePoint = 270.0 - (a * 180.0 / M_PI);
			else
				CurrentBrgToActivePoint = 90.0 - (a * 180.0 / M_PI);
		}

		// Calculate range using Great Circle Formula

		double d5 = geo::DistGreatCircle(nav.pos, pActivePoint->get_position());
		CurrentRngToActivePoint = d5;

		// Get the XTE vector, normal to current segment
		util::Vector2D va;
		util::Vector2D vb;
		util::Vector2D vn;

		double brg1, dist1, brg2, dist2;
		geo::DistanceBearingMercator(pActivePoint->get_position(),
									 pActiveRouteSegmentBeginPoint->get_position(), &brg1, &dist1);
		vb.x = dist1 * sin(brg1 * M_PI / 180.0);
		vb.y = dist1 * cos(brg1 * M_PI / 180.0);

		geo::DistanceBearingMercator(pActivePoint->get_position(), nav.pos, &brg2, &dist2);
		va.x = dist2 * sin(brg2 * M_PI / 180.0);
		va.y = dist2 * cos(brg2 * M_PI / 180.0);

		double sdelta = lengthOfNormal(va, vb, vn); // NM
		CurrentXTEToActivePoint = sdelta;

		// Calculate the distance to the arrival line, which is perpendicular to the current
		// route segment
		// Taking advantage of the calculated normal from current position to route segment vn
		util::Vector2D vToArriveNormal = va - vn;
		CurrentRangeToActiveNormalCrossing = vToArriveNormal.length();

		// Compute current segment course
		// Using simple Mercater projection
		double x1;
		double y1;
		double x2;
		double y2;
		geo::toSM(pActiveRouteSegmentBeginPoint->get_position(),
				  pActiveRouteSegmentBeginPoint->get_position(), &x1, &y1);

		geo::toSM(pActivePoint->get_position(), pActiveRouteSegmentBeginPoint->get_position(), &x2,
				  &y2);

		double e1 = atan2((x2 - x1), (y2 - y1));
		CurrentSegmentCourse = e1 * 180.0 / M_PI;
		if (CurrentSegmentCourse < 0)
			CurrentSegmentCourse += 360;

		//      Compute XTE direction
		double h = atan(vn.y / vn.x);
		if (vn.x > 0)
			CourseToRouteSegment = 90.0 - (h * 180.0 / M_PI);
		else
			CourseToRouteSegment = 270.0 - (h * 180.0 / M_PI);

		h = CurrentBrgToActivePoint - CourseToRouteSegment;
		if (h < 0)
			h = h + 360;

		if (h > 180)
			XTEDir = 1;
		else
			XTEDir = -1;

		// Determine Arrival

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
	if (!ActivateNextPoint(pActiveRoute, false)) { // at the end?
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

const wxPen& Routeman::GetRoutePen(void) const
{
	return *m_pRoutePen;
}

const wxPen& Routeman::GetSelectedRoutePen(void) const
{
	return *m_pSelectedRoutePen;
}

const wxPen& Routeman::GetActiveRoutePen(void) const
{
	return *m_pActiveRoutePen;
}

const wxPen& Routeman::GetActiveRoutePointPen(void) const
{
	return *m_pActiveRoutePointPen;
}

const wxPen& Routeman::GetRoutePointPen(void) const
{
	return *m_pRoutePointPen;
}

const wxBrush& Routeman::GetSelectedRouteBrush(void) const
{
	return *m_pSelectedRouteBrush;
}

const wxBrush& Routeman::GetActiveRouteBrush(void) const
{
	return *m_pActiveRouteBrush;
}

bool Routeman::DeactivateRoute(bool b_arrival)
{
	if (pActivePoint) {
		pActivePoint->set_blink(false);
		pActivePoint->m_bIsActive = false;
	}

	if (pActiveRoute) {
		pActiveRoute->m_bRtIsActive = false;
		pActiveRoute->m_pRouteActivePoint = NULL;
	}

	wxJSONValue v;
	if (!b_arrival) {
		v[_T("Route_deactivated")] = pActiveRoute->get_name();
		v[_T("GUID")] = pActiveRoute->guid();
		wxString msg_id(_T("OCPN_RTE_DEACTIVATED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
	} else {
		v[_T("GUID")] = pActiveRoute->guid();
		v[_T("Route_ended")] = pActiveRoute->get_name();
		wxString msg_id(_T("OCPN_RTE_ENDED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
	}

	pActiveRoute = NULL;

	if (pRouteActivatePoint)
		delete pRouteActivatePoint;
	pRouteActivatePoint = NULL;

	console->clear_background();
	console->Show(false);
	m_bDataValid = false;
	return true;
}

bool Routeman::UpdateAutopilot()
{
	// FIXME: split up method

	// Send all known Autopilot messages upstream

	// RMB
	{
		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;
		m_NMEA0183.Rmb.IsDataValid = NTrue;
		m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;

		if (XTEDir < 0)
			m_NMEA0183.Rmb.DirectionToSteer = Left;
		else
			m_NMEA0183.Rmb.DirectionToSteer = Right;

		wxString name = pActivePoint->GetName();
		m_NMEA0183.Rmb.To = name.Truncate(6);
		name = pActiveRouteSegmentBeginPoint->GetName();
		m_NMEA0183.Rmb.From = name.Truncate(6);

		if (pActivePoint->latitude() < 0.0)
			m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(-pActivePoint->latitude(), _T("S"));
		else
			m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(pActivePoint->latitude(), _T("N"));

		if (pActivePoint->longitude() < 0.0)
			m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(-pActivePoint->longitude(), _T("W"));
		else
			m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(pActivePoint->longitude(), _T("E"));

		m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
		m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;
		m_NMEA0183.Rmb.DestinationClosingVelocityKnots = global::OCPN::get().nav().get_data().sog;

		if (m_bArrival)
			m_NMEA0183.Rmb.IsArrivalCircleEntered = NTrue;
		else
			m_NMEA0183.Rmb.IsArrivalCircleEntered = NFalse;

		m_NMEA0183.Rmb.Write(snt);

		g_pMUX->SendNMEAMessage(snt.Sentence);
	}

	// RMC
	{
		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;
		m_NMEA0183.Rmc.IsDataValid = NTrue;

		if (nav.pos.lat() < 0.0)
			m_NMEA0183.Rmc.Position.Latitude.Set(-nav.pos.lat(), _T("S"));
		else
			m_NMEA0183.Rmc.Position.Latitude.Set(nav.pos.lat(), _T("N"));

		if (nav.pos.lon() < 0.0)
			m_NMEA0183.Rmc.Position.Longitude.Set(-nav.pos.lon(), _T("W"));
		else
			m_NMEA0183.Rmc.Position.Longitude.Set(nav.pos.lon(), _T("E"));

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
			m_NMEA0183.Rmc.MagneticVariation = 361.0; // A signal to NMEA converter, gVAR is unknown

		wxDateTime now = wxDateTime::Now();
		wxDateTime utc = now.ToUTC();
		wxString time = utc.Format(_T("%H%M%S"));
		m_NMEA0183.Rmc.UTCTime = time;

		wxString date = utc.Format(_T("%d%m%y"));
		m_NMEA0183.Rmc.Date = date;

		m_NMEA0183.Rmc.Write(snt);

		g_pMUX->SendNMEAMessage(snt.Sentence);
	}

	// APB
	{
		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;

		m_NMEA0183.Apb.IsLoranBlinkOK = NTrue;
		m_NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;

		m_NMEA0183.Apb.CrossTrackErrorMagnitude = CurrentXTEToActivePoint;

		if (XTEDir < 0)
			m_NMEA0183.Apb.DirectionToSteer = Left;
		else
			m_NMEA0183.Apb.DirectionToSteer = Right;

		m_NMEA0183.Apb.CrossTrackUnits = _T("N");

		if (m_bArrival)
			m_NMEA0183.Apb.IsArrivalCircleEntered = NTrue;
		else
			m_NMEA0183.Apb.IsArrivalCircleEntered = NFalse;

		// We never pass the perpendicular, since we declare arrival before reaching this point
		m_NMEA0183.Apb.IsPerpendicular = NFalse;

		wxString name = pActivePoint->GetName();
		m_NMEA0183.Apb.To = name.Truncate(6);

		double brg1;
		double dist1;
		geo::DistanceBearingMercator(pActivePoint->get_position(),
									 pActiveRouteSegmentBeginPoint->get_position(), &brg1, &dist1);

		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

		if (nav.MagneticAPB && !wxIsNaN(nav.var)) {

			double brg1m = ((brg1 - nav.var) >= 0.0) ? (brg1 - nav.var) : (brg1 - nav.var + 360.0);
			double bapm = ((CurrentBrgToActivePoint - nav.var) >= 0.0)
							  ? (CurrentBrgToActivePoint - nav.var)
							  : (CurrentBrgToActivePoint - nav.var + 360.0);

			m_NMEA0183.Apb.BearingOriginToDestination = brg1m;
			m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("M");

			m_NMEA0183.Apb.BearingPresentPositionToDestination = bapm;
			m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("M");

			m_NMEA0183.Apb.HeadingToSteer = bapm;
			m_NMEA0183.Apb.HeadingToSteerUnits = _T("M");
		} else {
			m_NMEA0183.Apb.BearingOriginToDestination = brg1;
			m_NMEA0183.Apb.BearingOriginToDestinationUnits = _T("T");

			m_NMEA0183.Apb.BearingPresentPositionToDestination = CurrentBrgToActivePoint;
			m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _T("T");

			m_NMEA0183.Apb.HeadingToSteer = CurrentBrgToActivePoint;
			m_NMEA0183.Apb.HeadingToSteerUnits = _T("T");
		}

		m_NMEA0183.Apb.Write(snt);
		g_pMUX->SendNMEAMessage(snt.Sentence);
	}

	return true;
}

bool Routeman::DoesRouteContainSharedPoints(const Route* pRoute)
{
	if (!pRoute)
		return false;

	// walk the route, looking at each point to see if it is used by another route
	// or is isolated
	for (RoutePointList::const_iterator point = pRoute->routepoints().begin();
		 point != pRoute->routepoints().end(); ++point) {

		// check all other routes to see if this point appears in any other route
		RouteArray routes = GetRouteArrayContaining(*point);
		for (unsigned int ir = 0; ir < routes.size(); ++ir) { // FIXME: std::find
			if (pRoute == routes[ir])
				continue;
			else
				return true;
		}
	}

	// Now walk the route again, looking for isolated type shared waypoints
	for (RoutePointList::const_iterator point = pRoute->routepoints().begin();
		 point != pRoute->routepoints().end(); ++point) {
		if ((*point)->m_bKeepXRoute)
			return true;
	}

	return false;
}

void Routeman::DeleteRoute(Route* pRoute)
{
	if (!pRoute)
		return;

	::wxBeginBusyCursor();

	if (GetpActiveRoute() == pRoute)
		DeactivateRoute();

	if (pRoute->m_bIsInLayer)
		return;

	// Remove the route from associated lists
	pSelect->DeleteAllSelectableRouteSegments(pRoute);
	pRouteList->erase(std::find(pRouteList->begin(), pRouteList->end(), pRoute));

	// walk the route, tentatively deleting/marking points used only by this route
	RoutePointList::iterator pnode = pRoute->routepoints().begin();
	while (pnode != pRoute->routepoints().end()) {
		RoutePoint* prp = *pnode;

		// check all other routes to see if this point appears in any other route
		Route* pcontainer_route = FindRouteContainingWaypoint(prp);

		if (pcontainer_route == NULL && prp->is_in_route()) {
			prp->m_bIsInRoute = false; // Take this point out of this (and only) route
			if (!prp->m_bKeepXRoute) {
				// This does not need to be done with navobj.xml storage, since the waypoints are
				// stored with the route

				pSelect->DeleteSelectablePoint(prp, SelectItem::TYPE_ROUTEPOINT);

				// Remove all instances of this point from the list.
				RoutePointList::iterator pdnode = pnode;
				while (pdnode != pRoute->routepoints().end()) {
					pRoute->routepoints().erase(pdnode);
					pdnode = std::find(pRoute->routepoints().begin(), pRoute->routepoints().end(),
									   prp);
				}

				pnode = pRoute->routepoints().end();
				delete prp;
			} else {
				prp->set_dynamic_name(false);
				prp->m_bIsolatedMark = true; // This has become an isolated mark
				prp->m_bKeepXRoute = false; // and is no longer part of a route
			}
		}

		if (pnode != pRoute->routepoints().end()) {
			++pnode;
		} else {
			pnode = pRoute->routepoints().begin(); // restart the list
		}
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
			pConfig->disable_changeset_update();
			pConfig->DeleteConfigRoute(proute);
			DeleteRoute(proute);
			i = pRouteList->begin();
			pConfig->enable_changeset_update();
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
			pConfig->disable_changeset_update();
			pConfig->DeleteConfigRoute(proute);
			DeleteTrack(proute);
			i = pRouteList->begin();
			pConfig->enable_changeset_update();
		} else {
			++i;
		}
	}

	::wxEndBusyCursor();
}

void Routeman::DeleteTrack(Route* pRoute)
{
	if (!pRoute)
		return;

	if (pRoute->m_bIsInLayer)
		return;

	::wxBeginBusyCursor();

	wxProgressDialog *pprog = NULL;
	int count = pRoute->routepoints().size();
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
	pRouteList->erase(std::find(pRouteList->begin(), pRouteList->end(), pRoute));

	// walk the route, tentatively deleting/marking points used only by this route
	int ic = 0;
	RoutePointList::iterator pnode = pRoute->routepoints().begin();
	while (pnode != pRoute->routepoints().end()) {
		if (pprog) {
			pprog->Update(ic, wxString::Format(_T("%d/%d"), ic, count));
			ic++;
		}

		RoutePoint* prp = *pnode;

		// check all other routes to see if this point appears in any other route
		Route* pcontainer_route = NULL; // FindRouteContainingWaypoint(prp);

		if (pcontainer_route == NULL) {
			prp->m_bIsInRoute = false; // Take this point out of this (and only) route
			if (!prp->m_bKeepXRoute) {
				pConfig->disable_changeset_update();
				pConfig->DeleteWayPoint(prp);
				pSelect->DeleteSelectablePoint(prp, SelectItem::TYPE_ROUTEPOINT);
				pConfig->enable_changeset_update();

				pRoute->routepoints().erase(pnode);
				pnode = pRoute->routepoints().end();
				delete prp;
			}
		}
		if (pnode != pRoute->routepoints().end()) {
			++pnode;
		} else {
			pnode = pRoute->routepoints().begin();
		}
	}

	navigation::RouteTracker& tracker = global::OCPN::get().tracker();
	if (tracker.is_active_track(pRoute)) {
		tracker.stop(false, false);
	}

	delete pRoute;

	::wxEndBusyCursor();

	if (pprog) {
		delete pprog;
		pprog = NULL;
	}
}

void Routeman::SetColorScheme(global::ColorScheme)
{
	const global::GUI::View& view = global::OCPN::get().gui().view();
	const global::ColorManager& colors = global::OCPN::get().color();

	m_pActiveRoutePointPen
		= wxThePenList->FindOrCreatePen(wxColour(0, 0, 255), view.route_line_width, wxSOLID);
	m_pRoutePointPen
		= wxThePenList->FindOrCreatePen(wxColour(0, 0, 255), view.route_line_width, wxSOLID);

	// Or in something like S-52 compliance

	m_pRoutePen = wxThePenList->FindOrCreatePen(colors.get_color(_T("UINFB")),
												view.route_line_width, wxSOLID);
	m_pSelectedRoutePen = wxThePenList->FindOrCreatePen(colors.get_color(_T("UINFO")),
														view.route_line_width, wxSOLID);
	m_pActiveRoutePen = wxThePenList->FindOrCreatePen(colors.get_color(_T("UARTE")),
													  view.route_line_width, wxSOLID);

	m_pRouteBrush = wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("UINFB")), wxSOLID);
	m_pSelectedRouteBrush
		= wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("UINFO")), wxSOLID);
	m_pActiveRouteBrush = wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("PLRTE")), wxSOLID);
}

const wxString& Routeman::GetRouteReverseMessage(void) const
{
	static const wxString MESSAGE(_("Waypoints can be renamed to reflect the new order, the names will be '001', '002' etc.\n\nDo you want to rename the waypoints?"));

	return MESSAGE;
}

Route* Routeman::FindRouteByGUID(const wxString& guid) const
{
	return RouteExists(guid);
}

bool Routeman::is_data_valid() const
{
	return m_bDataValid;
}

