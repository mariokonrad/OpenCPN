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

#include "Track.h"

#include <Select.h>
#include <ocpnDC.h>
#include <RouteProp.h>
#include <ChartCanvas.h>

#include <global/OCPN.h>
#include <global/Navigation.h>
#include <global/GUI.h>
#include <global/ColorManager.h>

#include <geo/GeoRef.h>

#include <gpx/gpx.h>

#define TIMER_TRACK1 778

extern RouteList* pRouteList;
extern Select* pSelect;
extern ChartCanvas* cc1;
extern RouteProp* pRoutePropDialog;

BEGIN_EVENT_TABLE(Track, wxEvtHandler)
	EVT_TIMER(TIMER_TRACK1, Track::OnTimerTrack)
END_EVENT_TABLE()


Track::Track(void)
{
	m_TimerTrack.SetOwner(this, TIMER_TRACK1);
	m_TimerTrack.Stop();
	m_bRunning = false;
	m_bIsTrack = true;

	SetPrecision(global::OCPN::get().nav().get_track().TrackPrecision);

	m_prev_time = wxInvalidDateTime;
	m_lastStoredTP = NULL;

	trackPointState = firstPoint;
	m_lastStoredTP = NULL;
	m_removeTP = NULL;
	m_fixedTP = NULL;
	m_track_run = 0;
}

Track::~Track()
{
	Stop();
}

bool Track::IsRunning() const
{
	return m_bRunning;
}

void Track::SetPrecision(int prec)
{
	m_nPrecision = prec;
	switch (m_nPrecision) {
		case 0: // Low
			m_allowedMaxAngle = 10;
			m_allowedMaxXTE = 0.008;
			m_TrackTimerSec = 8;
			m_minTrackpoint_delta = 0.004;
			break;

		case 1: // Medium
			m_allowedMaxAngle = 10;
			m_allowedMaxXTE = 0.004;
			m_TrackTimerSec = 4;
			m_minTrackpoint_delta = 0.002;
			break;

		case 2: // High
			m_allowedMaxAngle = 10;
			m_allowedMaxXTE = 0.0015;
			m_TrackTimerSec = 2;
			m_minTrackpoint_delta = 0.001;
			break;
	}
}

void Track::Start(void)
{
	if (!m_bRunning) {
		AddPointNow(true); // Add initial point
		m_TimerTrack.Start(1000, wxTIMER_CONTINUOUS);
		m_bRunning = true;
	}
}

void Track::Stop(bool do_add_point)
{
	double delta = 0.0;
	if (m_lastStoredTP) {
		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		delta = geo::DistGreatCircle(nav.pos, m_lastStoredTP->get_position());
	}

	if (m_bRunning && ((delta > m_minTrackpoint_delta) || do_add_point))
		AddPointNow(true); // Add last point

	m_TimerTrack.Stop();
	m_bRunning = false;
	m_track_run = 0;
}

Track* Track::DoExtendDaily()
{
	Route* pExtendRoute = NULL;
	RoutePoint* pExtendPoint = NULL;

	RoutePoint* pLastPoint = this->GetPoint(1);

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (!route->m_bIsInLayer && route->m_bIsTrack && (route->guid() != this->guid())) {
			RoutePoint* track_node = route->GetLastPoint();
			if (track_node->GetCreateTime() <= pLastPoint->GetCreateTime()) {
				if (!pExtendPoint || track_node->GetCreateTime() > pExtendPoint->GetCreateTime()) {
					pExtendPoint = track_node;
					pExtendRoute = route;
				}
			}
		}
	}
	if (pExtendRoute
		&& pExtendRoute->GetPoint(1)->GetCreateTime().FromTimezone(wxDateTime::GMT0).IsSameDate(
			   pLastPoint->GetCreateTime().FromTimezone(wxDateTime::GMT0))) {
		int begin = 1;
		if (pLastPoint->GetCreateTime() == pExtendPoint->GetCreateTime())
			begin = 2;
		pSelect->DeleteAllSelectableTrackSegments(pExtendRoute);
		wxString suffix = _T("");
		if (get_name().IsNull()) {
			suffix = pExtendRoute->get_name();
			if (suffix.IsNull())
				suffix = wxDateTime::Today().FormatISODate();
		}
		pExtendRoute->CloneTrack(this, begin, this->GetnPoints(), suffix);
		pSelect->AddAllSelectableTrackSegments(pExtendRoute);
		pSelect->DeleteAllSelectableTrackSegments(this);
		this->ClearHighlights();
		return (Track*)pExtendRoute;
	} else {
		if (get_name().IsNull())
			set_name(wxDateTime::Today().FormatISODate());
		return NULL;
	}
}

void Track::AdjustCurrentTrackPoint(RoutePoint* prototype)
{
	if (prototype) {
		CloneAddedTrackPoint(m_lastStoredTP, prototype);
		m_prev_time = prototype->GetCreateTime().FromUTC();
	}
}

void Track::OnTimerTrack(wxTimerEvent&)
{
	m_TimerTrack.Stop();
	m_track_run++;

	if (m_lastStoredTP) {
		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		m_prev_dist = geo::DistGreatCircle(nav.pos, m_lastStoredTP->get_position());
	} else {
		m_prev_dist = 999.0;
	}

	bool b_addpoint = false;

	if ((m_TrackTimerSec > 0.0) && ((double)m_track_run >= m_TrackTimerSec)
		&& (m_prev_dist > m_minTrackpoint_delta)) {
		b_addpoint = true;
		m_track_run = 0;
	}

	if (b_addpoint) {
		AddPointNow();
	} else {
		const global::Navigation::Track& track = global::OCPN::get().nav().get_track();

		// continuously update track beginning point timestamp if no movement.
		if ((trackPointState == firstPoint) && !track.TrackDaily) {
			wxDateTime now = wxDateTime::Now();
			routepoints().front()->SetCreateTime(now.ToUTC());
		}
	}

	m_TimerTrack.Start(1000, wxTIMER_CONTINUOUS);
}

RoutePoint* Track::AddNewPoint(util::Vector2D point, wxDateTime time)
{
	RoutePoint* rPoint = new RoutePoint(geo::Position(point.lat, point.lon), _T("empty"), _T(""));
	rPoint->set_show_name(false);
	rPoint->set_visible(true);
	rPoint->m_GPXTrkSegNo = 1;
	rPoint->SetCreateTime(time);
	AddPoint(rPoint);

	// This is a hack, need to undo the action of Route::AddPoint
	rPoint->m_bIsInRoute = false;
	rPoint->m_bIsInTrack = true;
	return rPoint;
}

void Track::AddPointNow(bool do_add_point)
{
	static std::vector<RoutePoint> skippedPoints;

	wxDateTime now = wxDateTime::Now();

	if (m_prev_dist < 0.0005) // avoid zero length segs
		if (!do_add_point)
			return;

	if (m_prev_time.IsValid())
		if (m_prev_time == now) // avoid zero time segs
			if (!do_add_point)
				return;

	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
	util::Vector2D gpsPoint(nav.pos.lon(), nav.pos.lat());

	// The dynamic interval algorithm will gather all track points in a queue,
	// and analyze the cross track errors for each point before actually adding
	// a point to the track.

	switch (trackPointState) {
		case firstPoint: {
			m_lastStoredTP = AddNewPoint(gpsPoint, now.ToUTC());
			trackPointState = secondPoint;
			do_add_point = false;
			break;
		}
		case secondPoint: {
			util::Vector2D pPoint(nav.pos.lon(), nav.pos.lat());
			skipPoints.push_back(pPoint);
			skipTimes.push_back(now.ToUTC());
			trackPointState = potentialPoint;
			break;
		}
		case potentialPoint: {
			if (gpsPoint == skipPoints[skipPoints.size() - 1])
				break;

			unsigned int xteMaxIndex = 0;
			double xteMax = 0;

			// Scan points skipped so far and see if anyone has XTE over the threshold.
			for (unsigned int i = 0; i < skipPoints.size(); i++) {
				double xte = GetXTE(m_lastStoredTP->get_position(), nav.pos,
									geo::Position(skipPoints[i].lat, skipPoints[i].lon));
				if (xte > xteMax) {
					xteMax = xte;
					xteMaxIndex = i;
				}
			}
			if (xteMax > m_allowedMaxXTE) {
				RoutePoint* pTrackPoint
					= AddNewPoint(skipPoints[xteMaxIndex], skipTimes[xteMaxIndex]);
				pSelect->AddSelectableTrackSegment(m_lastStoredTP->get_position(),
												   pTrackPoint->get_position(), m_lastStoredTP,
												   pTrackPoint, this);

				m_prevFixedTP = m_fixedTP;
				m_fixedTP = m_removeTP;
				m_removeTP = m_lastStoredTP;
				m_lastStoredTP = pTrackPoint;
				for (unsigned int i = 0; i <= xteMaxIndex; i++) {
					skipPoints.pop_front();
					skipTimes.pop_front();
				}

				// Now back up and see if we just made 3 points in a straight line and the middle
				// one (the next to last) point can possibly be eliminated. Here we reduce the
				// allowed XTE as a function of leg length. (Half the XTE for very short legs).
				if (GetnPoints() > 2) {
					double dist = geo::DistGreatCircle(m_fixedTP->get_position(),
													   m_lastStoredTP->get_position());
					double xte = GetXTE(m_fixedTP, m_lastStoredTP, m_removeTP);
					if (xte < m_allowedMaxXTE / wxMax(1.0, 2.0 - dist * 2.0)) {
						routepoints().pop_back();
						routepoints().pop_back();
						routepoints().push_back(m_lastStoredTP);
						SetnPoints();
						pSelect->DeletePointSelectableTrackSegments(m_removeTP);
						pSelect->AddSelectableTrackSegment(m_fixedTP->get_position(),
														   m_lastStoredTP->get_position(),
														   m_fixedTP, m_lastStoredTP, this);
						delete m_removeTP;
						m_removeTP = m_fixedTP;
						m_fixedTP = m_prevFixedTP;
					}
				}
			}

			skipPoints.push_back(gpsPoint);
			skipTimes.push_back(now.ToUTC());
			break;
		}
	}

	// Check if this is the last point of the track.
	if (do_add_point) {
		RoutePoint* pTrackPoint = AddNewPoint(gpsPoint, now.ToUTC());
		pSelect->AddSelectableTrackSegment(m_lastStoredTP->get_position(),
										   pTrackPoint->get_position(), m_lastStoredTP, pTrackPoint,
										   this);
	}

	m_prev_time = now;
}

void Track::Draw(ocpnDC& dc, const ViewPort& VP)
{
	if (!IsVisible() || GetnPoints() == 0)
		return;

	double radius = 0.0;
	if (global::OCPN::get().nav().get_track().HighliteTracks) {
		double radius_meters = 20;
		radius = radius_meters * VP.view_scale();
	}

	unsigned short int FromSegNo = 1;

	RoutePointList::iterator route_point = routepoints().begin();
	RoutePoint* prp = *route_point;

	// Establish basic colour
	const global::ColorManager& colors = global::OCPN::get().color();
	wxColour basic_colour;
	if (m_bRunning || prp->icon_name().StartsWith(_T("xmred"))) {
		basic_colour = colors.get_color(_T("URED"));
	} else if (prp->icon_name().StartsWith(_T("xmblue"))) {
		basic_colour = colors.get_color(_T("BLUE3"));
	} else if (prp->icon_name().StartsWith(_T("xmgreen"))) {
		basic_colour = colors.get_color(_T("UGREN"));
	} else {
		basic_colour = colors.get_color(_T("CHMGD"));
	}

	int style = wxSOLID;
	int width = global::OCPN::get().gui().view().track_line_width;
	wxColour col;
	if (m_style != STYLE_UNDEFINED)
		style = m_style;
	if (m_width != STYLE_UNDEFINED)
		width = m_width;
	if (m_Colour == wxEmptyString) {
		col = basic_colour;
	} else {
		for (unsigned int i = 0; i < sizeof(gpx::GpxxColorNames) / sizeof(wxString); i++) {
			if (m_Colour == gpx::GpxxColorNames[i]) {
				col = gpx::GpxxColors[i];
				break;
			}
		}
	}
	dc.SetPen(*wxThePenList->FindOrCreatePen(col, width, style));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(col, wxSOLID));

	// Draw the first point
	wxPoint rpt;
	wxPoint rptn;
	DrawPointWhich(dc, 1, &rpt);

	++route_point;
	while (route_point != routepoints().end()) {
		RoutePoint* prp = *route_point;
		unsigned short int ToSegNo = prp->m_GPXTrkSegNo;

		wxPoint r = cc1->GetCanvasPointPix(prp->get_position());

		// We do inline decomposition of the line segments, in a simple minded way
		// If the line segment length is less than approximately 2 pixels, then simply don't render
		// it, but continue on to the next point.
		if ((abs(r.x - rpt.x) > 1) || (abs(r.y - rpt.y) > 1)) {
			prp->Draw(dc, &rptn);

			if (ToSegNo == FromSegNo)
				RenderSegment(dc, rpt.x, rpt.y, rptn.x, rptn.y, VP, false,
							  (int)radius); // no arrows, with hilite

			rpt = rptn;
		}

		++route_point;
		FromSegNo = ToSegNo;
	}

	// Draw last segment, dynamically, maybe.....

	if (m_bRunning) {
		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		wxPoint r = cc1->GetCanvasPointPix(nav.pos);
		RenderSegment(dc, rpt.x, rpt.y, r.x, r.y, VP, false, (int)radius); // no arrows, with hilite
	}
}

Route* Track::RouteFromTrack(wxProgressDialog* pprog) // FIXME: clean up this mess
{
	Route* route = new Route();
	RoutePointList::iterator prpnode = routepoints().begin();
	RoutePoint* pWP_src = *prpnode;
	RoutePointList::iterator prpnodeX;
	RoutePoint* pWP_dst;
	RoutePoint* prp_OK = NULL; // last routepoint known not to exceed xte limit, if not yet added

	const global::Navigation::Track& track = global::OCPN::get().nav().get_track();

	wxString icon = _T("xmblue");
	if (track.TrackDeltaDistance >= 0.1)
		icon = _T("diamond");

	int ic = 0;
	int next_ic = 0;
	int back_ic = 0;
	int nPoints = routepoints().size();
	bool isProminent = true;
	double delta_dist;
	double leg_speed = 0.1;

	if (pRoutePropDialog)
		leg_speed = pRoutePropDialog->getPlanSpeed();
	else
		leg_speed = track.PlanSpeed;

	// add first point

	pWP_dst = new RoutePoint(pWP_src->get_position(), icon, _T(""));
	route->AddPoint(pWP_dst);

	pWP_dst->set_show_name(false);

	pSelect->AddSelectableRoutePoint(pWP_dst->get_position(), pWP_dst);

	// add intermediate points as needed

	++prpnode;

	while (prpnode != routepoints().end()) {
		RoutePoint* prp = *prpnode;
		prpnodeX = prpnode;
		pWP_dst = pWP_src;

		delta_dist = 0.0;
		double delta_hdg = 0.0;
		back_ic = next_ic;

		geo::DistanceBearingMercator(prp->get_position(), pWP_src->get_position(), &delta_hdg,
									 &delta_dist);

		if ((delta_dist > (leg_speed * 6.0)) && !prp_OK) {
			int delta_inserts = floor(delta_dist / (leg_speed * 4.0));
			delta_dist = delta_dist / (delta_inserts + 1);

			while (delta_inserts--) {
				geo::Position t = geo::ll_gc_ll(pWP_src->get_position(), delta_hdg, delta_dist);
				pWP_dst = new RoutePoint(t, icon, _T(""));
				route->AddPoint(pWP_dst);
				pWP_dst->set_show_name(false);
				pSelect->AddSelectableRoutePoint(pWP_dst->get_position(), pWP_dst);

				pSelect->AddSelectableRouteSegment(pWP_src->get_position(), pWP_dst->get_position(),
												   pWP_src, pWP_dst, route);

				pWP_src = pWP_dst;
			}
			prpnodeX = prpnode;
			pWP_dst = pWP_src;
			next_ic = 0;
			delta_dist = 0.0;
			back_ic = next_ic;
			prp_OK = prp;
			isProminent = true;
		} else {
			isProminent = false;
			if (delta_dist >= (leg_speed * 4.0))
				isProminent = true;
			if (!prp_OK)
				prp_OK = prp;
		}

		while (prpnodeX != routepoints().end()) {
			RoutePoint* prpX = *prpnodeX;
			double xte = GetXTE(pWP_src, prpX, prp);
			if (isProminent || (xte > track.TrackDeltaDistance)) {

				pWP_dst = new RoutePoint(prp_OK->get_position(), icon, _T(""));

				route->AddPoint(pWP_dst);
				pWP_dst->set_show_name(false);

				pSelect->AddSelectableRoutePoint(pWP_dst->get_position(), pWP_dst);

				pSelect->AddSelectableRouteSegment(pWP_src->get_position(), pWP_dst->get_position(),
												   pWP_src, pWP_dst, route);

				pWP_src = pWP_dst;
				next_ic = 0;
				prpnodeX = routepoints().end();
				prp_OK = NULL;
			}

			if (prpnodeX != routepoints().end())
				--prpnodeX;
			if (back_ic-- <= 0)
				prpnodeX = routepoints().end();
		}

		if (prp_OK) {
			prp_OK = prp;
		}

		geo::DistanceBearingMercator(prp->get_position(), pWP_src->get_position(), NULL,
									 &delta_dist);

		if (!((delta_dist > (track.TrackDeltaDistance)) && !prp_OK)) {
			++prpnode;
			next_ic++;
		}
		ic++;
		if (pprog)
			pprog->Update((ic * 100) / nPoints);
	}

	// add last point, if needed
	if (delta_dist >= track.TrackDeltaDistance) {
		pWP_dst = new RoutePoint(routepoints().back()->get_position(), icon, _T(""));
		route->AddPoint(pWP_dst);
		pWP_dst->set_show_name(false);
		pSelect->AddSelectableRoutePoint(pWP_dst->get_position(), pWP_dst);
		pSelect->AddSelectableRouteSegment(pWP_src->get_position(), pWP_dst->get_position(),
										   pWP_src, pWP_dst, route);
	}
	route->set_name(get_name());
	route->set_startString(get_startString());
	route->set_endString(get_endString());
	route->m_bDeleteOnArrival = false;

	return route;
}

void Track::DouglasPeuckerReducer(std::vector<RoutePoint*>& list, int from, int to, double delta)
{
	list[from]->m_bIsActive = true;
	list[to]->m_bIsActive = true;

	int maxdistIndex = -1;
	double maxdist = 0;

	for (int i = from + 1; i < to; i++) {
		double dist = 1852.0 * GetXTE(list[from], list[to], list[i]);

		if (dist > maxdist) {
			maxdist = dist;
			maxdistIndex = i;
		}
	}

	if (maxdist > delta) {
		DouglasPeuckerReducer(list, from, maxdistIndex, delta);
		DouglasPeuckerReducer(list, maxdistIndex, to, delta);
	}
}

int Track::Simplify(double maxDelta)
{
	int reduction = 0;
	std::vector<RoutePoint*> pointlist;

	::wxBeginBusyCursor();

	for (RoutePointList::iterator i = routepoints().begin(); i != routepoints().end(); ++i) {
		RoutePoint* routepoint = *i;
		routepoint->m_bIsActive = false;
		pointlist.push_back(routepoint);
	}

	DouglasPeuckerReducer(pointlist, 0, pointlist.size() - 1, maxDelta);

	pSelect->DeleteAllSelectableTrackSegments(this);
	routepoints().clear();

	for (size_t i = 0; i < pointlist.size(); i++) {
		if (pointlist[i]->m_bIsActive) {
			pointlist[i]->m_bIsActive = false;
			routepoints().push_back(pointlist[i]);
		} else {
			delete pointlist[i];
			reduction++;
		}
	}

	SetnPoints();
	pSelect->AddAllSelectableTrackSegments(this);

	UpdateSegmentDistances();
	::wxEndBusyCursor();
	return reduction;
}

double Track::GetXTE(const geo::Position& fm1, const geo::Position& fm2, const geo::Position& to) const
{
	util::Vector2D v;
	util::Vector2D w;
	util::Vector2D p;

	// First we get the cartesian coordinates to the line endpoints, using
	// the current position as origo.

	double brg1;
	double dist1;
	double brg2;
	double dist2;
	geo::DistanceBearingMercator(to, fm1, &brg1, &dist1);
	w.x = dist1 * sin(brg1 * M_PI / 180.0);
	w.y = dist1 * cos(brg1 * M_PI / 180.0);

	geo::DistanceBearingMercator(to, fm2, &brg2, &dist2);
	v.x = dist2 * sin(brg2 * M_PI / 180.0);
	v.y = dist2 * cos(brg2 * M_PI / 180.0);

	p.x = 0.0;
	p.y = 0.0;

	const double lengthSquared = (v - w).sqr();
	if (lengthSquared == 0.0) {
		return (p - v).length();
	}

	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of origo onto the line.
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2

	util::Vector2D a = p - v;
	util::Vector2D b = w - v;

	double t = a.dot(b) / lengthSquared;

	if (t < 0.0)
		return (p - v).length(); // Beyond the 'v' end of the segment
	else if (t > 1.0)
		return (p - w).length(); // Beyond the 'w' end of the segment
	util::Vector2D projection = v + t * (w - v); // Projection falls on the segment
	return (p - projection).length();
}

double Track::GetXTE(const RoutePoint* fm1, const RoutePoint* fm2, const RoutePoint* to) const
{
	if (!fm1 || !fm2 || !to)
		return 0.0;
	if (fm1 == to)
		return 0.0;
	if (fm2 == to)
		return 0.0;
	return GetXTE(fm1->get_position(), fm2->get_position(), to->get_position());
}

