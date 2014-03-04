/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "Route.h"
#include <ocpnDC.h>
#include <Multiplexer.h>
#include <Select.h>
#include <MessageBox.h>
#include <ViewPort.h>
#include <Config.h>

#include <windows/compatibility.h>

#include <util/math.h>
#include <util/uuid.h>

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/Navigation.h>
#include <global/ColorManager.h>

#include <navigation/RouteManager.h>
#include <navigation/WaypointManager.h>

#include <geo/GeoRef.h>
#include <geo/LineClip.h>

#include <gpx/gpx.h>

#include <algorithm>

extern Select* pSelect;
extern Config* pConfig;
extern Multiplexer* g_pMUX;

const double Route::DEFAULT_SPEED = 5.0;
const int Route::STYLE_UNDEFINED = -1;

Route::SameGUID::SameGUID(const wxString& guid)
	: guid(guid)
{
}

bool Route::SameGUID::operator()(const Route* route) const
{
	return route && (guid == route->guid());
}

Route::Route(void)
	: m_bRtIsSelected(false)
	, m_bRtIsActive(false)
	, m_pRouteActivePoint(NULL)
	, m_bIsBeingCreated(false)
	, m_route_length(0.0)
	, m_route_time(0.0)
	, m_bIsTrack(false)
	, m_pLastAddedPoint(NULL)
	, m_bDeleteOnArrival(false)
	, m_bIsInLayer(false)
	, m_LayerID(0)
	, m_width(STYLE_UNDEFINED)
	, m_style(STYLE_UNDEFINED)
	, m_lastMousePointIndex(0)
	, m_NextLegGreatCircle(false)
	, m_PlannedSpeed(DEFAULT_SPEED)
	, m_PlannedDeparture(RTE_UNDEF_DEPARTURE)
	, m_TimeDisplayFormat(RTE_TIME_DISP_PC)
	, m_Colour(wxEmptyString)
	, m_btemp(false)
	, m_nPoints(0)
	, m_nm_sequence(1)
	, m_bVisible(true)
	, m_bListed(true)
	, m_bcrosses_idl(false)
{
	pRoutePointList = new RoutePointList;
	m_GUID = wxString(util::uuid().c_str(), wxConvUTF8);
	m_ArrivalRadius = global::OCPN::get().nav().route().arrival_circle_radius;
	RBBox.Reset();
}

Route::~Route(void)
{
	pRoutePointList->clear();
	delete pRoutePointList;
	pRoutePointList = NULL;
}

bool Route::CrossesIDL() const
{
	return m_bcrosses_idl;
}

bool Route::IsVisible() const
{
	return m_bVisible;
}

bool Route::IsListed() const
{
	return m_bListed;
}

bool Route::IsActive() const
{
	return m_bRtIsActive;
}

bool Route::IsSelected() const
{
	return m_bRtIsSelected;
}

bool Route::IsTrack() const
{
	return m_bIsTrack;
}

double Route::GetRouteArrivalRadius(void) const
{
	return m_ArrivalRadius;
}

void Route::SetRouteArrivalRadius(double radius)
{
	m_ArrivalRadius = radius;
}

int Route::GetnPoints(void) const
{
	return m_nPoints;
}

void Route::SetnPoints(void)
{
	m_nPoints = pRoutePointList->size();
}

const RoutePointList& Route::routepoints() const
{
	return *pRoutePointList;
}

RoutePointList& Route::routepoints()
{
	return *pRoutePointList;
}

const wxString& Route::get_name() const
{
	return m_RouteNameString;
}

void Route::set_name(const wxString& value)
{
	m_RouteNameString = value;
}

const wxString& Route::get_startString() const
{
	return m_RouteStartString;
}

void Route::set_startString(const wxString& value)
{
	m_RouteStartString = value;
}

const wxString& Route::get_endString() const
{
	return m_RouteEndString;
}

void Route::set_endString(const wxString& value)
{
	m_RouteEndString = value;
}

const wxString& Route::get_description() const
{
	return m_RouteDescription;
}

void Route::set_description(const wxString& value)
{
	m_RouteDescription = value;
}

// The following is used only for route splitting, assumes just created, empty route
void Route::CloneRoute(Route* psourceroute, int start_nPoint, int end_nPoint,
					   const wxString& suffix)
{
	m_bIsTrack = psourceroute->m_bIsTrack;

	m_RouteNameString = psourceroute->m_RouteNameString + suffix;
	m_RouteStartString = psourceroute->m_RouteStartString;
	m_RouteEndString = psourceroute->m_RouteEndString;

	for (int i = start_nPoint; i <= end_nPoint; i++) {
		if (!psourceroute->m_bIsInLayer)
			AddPoint(psourceroute->GetPoint(i), false);
		else {
			RoutePoint* psourcepoint = psourceroute->GetPoint(i);
			RoutePoint* ptargetpoint
				= new RoutePoint(psourcepoint->get_position(), psourcepoint->icon_name(),
								 psourcepoint->GetName(), _T(""), false);

			AddPoint(ptargetpoint, false);
			CloneAddedRoutePoint(m_pLastAddedPoint, psourcepoint);
		}
	}

	CalculateBBox();
}

void Route::CloneTrack(Route* psourceroute, int start_nPoint, int end_nPoint,
					   const wxString& suffix)
{
	if (psourceroute->m_bIsInLayer)
		return;

	m_bIsTrack = psourceroute->m_bIsTrack;

	m_RouteNameString = psourceroute->m_RouteNameString + suffix;
	m_RouteStartString = psourceroute->m_RouteStartString;
	m_RouteEndString = psourceroute->m_RouteEndString;

	bool b_splitting = GetnPoints() == 0;

	int startTrkSegNo;
	if (b_splitting)
		startTrkSegNo = psourceroute->GetPoint(start_nPoint)->m_GPXTrkSegNo;
	else
		startTrkSegNo = this->GetLastPoint()->m_GPXTrkSegNo;

	for (int i = start_nPoint; i <= end_nPoint; i++) {

		RoutePoint* psourcepoint = psourceroute->GetPoint(i);
		RoutePoint* ptargetpoint
			= new RoutePoint(psourcepoint->get_position(), psourcepoint->icon_name(),
							 psourcepoint->GetName(), _T(""), false);

		AddPoint(ptargetpoint, false);
		CloneAddedTrackPoint(m_pLastAddedPoint, psourcepoint);

		int segment_shift = psourcepoint->m_GPXTrkSegNo;

		if (start_nPoint == 2) {
			// continue first segment if tracks share the first point
			segment_shift = psourcepoint->m_GPXTrkSegNo - 1;
		}

		if (b_splitting)
			m_pLastAddedPoint->m_GPXTrkSegNo = (psourcepoint->m_GPXTrkSegNo - startTrkSegNo) + 1;
		else
			m_pLastAddedPoint->m_GPXTrkSegNo = startTrkSegNo + segment_shift;
	}

	CalculateBBox();
}

void Route::CloneAddedRoutePoint(RoutePoint* ptargetpoint, RoutePoint* psourcepoint)
{
	ptargetpoint->set_description(psourcepoint->get_description());
	ptargetpoint->m_bKeepXRoute = psourcepoint->m_bKeepXRoute;
	ptargetpoint->set_visible(psourcepoint->is_visible());
	ptargetpoint->m_bPtIsSelected = false;
	ptargetpoint->m_pbmIcon = psourcepoint->m_pbmIcon;
	ptargetpoint->set_show_name(psourcepoint->is_show_name());
	ptargetpoint->set_blink(psourcepoint->is_blink());
	ptargetpoint->set_dynamic_name(psourcepoint->is_dynamic_name());
	ptargetpoint->CurrentRect_in_DC = psourcepoint->CurrentRect_in_DC;
	ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetX;
	ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetY;
	ptargetpoint->SetCreateTime(psourcepoint->GetCreateTime());

	ptargetpoint->m_HyperlinkList = psourcepoint->m_HyperlinkList;
}

void Route::CloneAddedTrackPoint(RoutePoint* ptargetpoint, RoutePoint* psourcepoint)
{
	// This is a hack, need to undo the action of Route::AddPoint
	ptargetpoint->m_bIsInRoute = false;
	ptargetpoint->m_bIsInTrack = true;
	ptargetpoint->set_description(psourcepoint->get_description());
	ptargetpoint->m_bKeepXRoute = psourcepoint->m_bKeepXRoute;
	ptargetpoint->set_visible(psourcepoint->is_visible());
	ptargetpoint->m_bPtIsSelected = false;
	ptargetpoint->m_pbmIcon = psourcepoint->m_pbmIcon;
	ptargetpoint->set_show_name(psourcepoint->is_show_name());
	ptargetpoint->set_blink(psourcepoint->is_blink());
	ptargetpoint->set_dynamic_name(psourcepoint->is_dynamic_name());
	ptargetpoint->CurrentRect_in_DC = psourcepoint->CurrentRect_in_DC;
	ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetX;
	ptargetpoint->m_NameLocationOffsetX = psourcepoint->m_NameLocationOffsetY;
	ptargetpoint->SetCreateTime(psourcepoint->GetCreateTime());
	ptargetpoint->m_HyperlinkList.clear();
}

void Route::AddPoint(RoutePoint* pNewPoint, bool b_rename_in_sequence, bool b_deferBoxCalc)
{
	if (pNewPoint->is_isolated()) {
		pNewPoint->m_bKeepXRoute = true;
	}
	pNewPoint->m_bIsolatedMark = false; // definitely no longer isolated
	pNewPoint->m_bIsInRoute = true;

	pRoutePointList->push_back(pNewPoint);

	m_nPoints++;

	if (!b_deferBoxCalc)
		CalculateBBox();

	if (m_pLastAddedPoint)
		pNewPoint->m_seg_len
			= geo::DistGreatCircle(m_pLastAddedPoint->get_position(), pNewPoint->get_position());

	m_route_length += pNewPoint->m_seg_len;

	m_pLastAddedPoint = pNewPoint;

	if (b_rename_in_sequence && pNewPoint->GetName().IsEmpty() && !pNewPoint->m_bKeepXRoute) {
		pNewPoint->SetName(wxString::Format(_T("%03d"), m_nPoints));
		pNewPoint->set_dynamic_name(true);
	}
}

void Route::AddTentativePoint(const wxString& GUID)
{
	routePointGUIDs.push_back(GUID);
}

RoutePoint* Route::GetPoint(int nWhichPoint)
{
	// FIXME: once the container for routepoints is a std::vector, this indexed access will be easier
	int index = 1;
	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end();
		 ++i, ++index) {
		if (index == nWhichPoint)
			return *i;
	}
	return NULL;
}

RoutePoint* Route::GetPoint(const wxString& guid)
{
	// FIXME: use find_if
	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
		if (guid == (*i)->guid())
			return *i;
	}
	return NULL;
}

void Route::DrawPointWhich(ocpnDC& dc, int iPoint, wxPoint* rpn)
{
	if (iPoint <= GetnPoints())
		GetPoint(iPoint)->Draw(dc, rpn);
}

void Route::DrawSegment(ocpnDC& dc, wxPoint* rp1, wxPoint* rp2, const ViewPort& VP, bool bdraw_arrow)
{
	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	if (m_bRtIsSelected)
		dc.SetPen(routemanager.GetSelectedRoutePen());
	else if (m_bRtIsActive)
		dc.SetPen(routemanager.GetActiveRoutePen());
	else
		dc.SetPen(routemanager.GetRoutePen());

	RenderSegment(dc, rp1->x, rp1->y, rp2->x, rp2->y, VP, bdraw_arrow);
}

void Route::Draw(ocpnDC& dc, const ViewPort& VP)
{
	if (m_nPoints == 0)
		return;

	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	if (m_bVisible && m_bRtIsSelected) {
		dc.SetPen(routemanager.GetSelectedRoutePen());
		dc.SetBrush(routemanager.GetSelectedRouteBrush());
	} else if (m_bVisible) {
		int style = wxSOLID;
		int width = global::OCPN::get().gui().view().route_line_width;
		wxColour col;
		if (m_style != STYLE_UNDEFINED)
			style = m_style;
		if (m_width != STYLE_UNDEFINED)
			width = m_width;
		if (m_Colour == wxEmptyString) {
			col = routemanager.GetRoutePen().GetColour();
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
	}

	if (m_bVisible && m_bRtIsActive) {
		dc.SetPen(routemanager.GetActiveRoutePen());
		dc.SetBrush(routemanager.GetActiveRouteBrush());
	}

	wxPoint rpt1;
	wxPoint rpt2;
	if (m_bVisible)
		DrawPointWhich(dc, 1, &rpt1);

	RoutePointList::iterator node = pRoutePointList->begin();
	RoutePoint* prp1 = *node;
	++node;

	if (!m_bVisible && prp1->m_bKeepXRoute)
		prp1->Draw(dc);

	while (node != pRoutePointList->end()) {
		RoutePoint* prp2 = *node;
		if (!m_bVisible && prp2->m_bKeepXRoute)
			prp2->Draw(dc);
		else if (m_bVisible)
			prp2->Draw(dc, &rpt2);

		if (m_bVisible) {
			// Handle offscreen points
			bool b_2_on = VP.GetBBox().PointInBox(prp2->longitude(), prp2->latitude(), 0);
			bool b_1_on = VP.GetBBox().PointInBox(prp1->longitude(), prp1->latitude(), 0);

			// TODO This logic could be simpliifed
			// Simple case
			if (b_1_on && b_2_on)
				RenderSegment(dc, rpt1.x, rpt1.y, rpt2.x, rpt2.y, VP, true); // with arrows

			// In the cases where one point is on, and one off
			// we must decide which way to go in longitude
			// Arbitrarily, we will go the shortest way

			double pix_full_circle = geo::WGS84_semimajor_axis_meters * geo::mercator_k0 * 2.0 * M_PI
									 * VP.view_scale();
			double dp = util::sqr(static_cast<double>(rpt1.x - rpt2.x))
						+ util::sqr(static_cast<double>(rpt1.y - rpt2.y));
			double dtest;
			int adder = 0;
			if (b_1_on && !b_2_on) {
				if (rpt2.x < rpt1.x)
					adder = static_cast<int>(pix_full_circle);
				else
					adder = -static_cast<int>(pix_full_circle);

				dtest = util::sqr(static_cast<double>(rpt1.x - (rpt2.x + adder)))
						+ util::sqr(static_cast<double>(rpt1.y - rpt2.y));

				if (dp < dtest)
					adder = 0;

				RenderSegment(dc, rpt1.x, rpt1.y, rpt2.x + adder, rpt2.y, VP, true);
			} else if (!b_1_on && b_2_on) {
				if (rpt1.x < rpt2.x)
					adder = static_cast<int>(pix_full_circle);
				else
					adder = -static_cast<int>(pix_full_circle);

				dtest = util::sqr(static_cast<double>(rpt2.x - (rpt1.x + adder)))
						+ util::sqr(static_cast<double>(rpt1.y - rpt2.y));

				if (dp < dtest)
					adder = 0;

				RenderSegment(dc, rpt1.x + adder, rpt1.y, rpt2.x, rpt2.y, VP, true);
			} else if (!b_1_on && !b_2_on) {
				// Both off, need to check shortest distance
				if (rpt1.x < rpt2.x)
					adder = static_cast<int>(pix_full_circle);
				else
					adder = -static_cast<int>(pix_full_circle);

				dtest = util::sqr(static_cast<double>(rpt2.x - (rpt1.x + adder)))
						+ util::sqr(static_cast<double>(rpt1.y - rpt2.y));

				if (dp < dtest)
					adder = 0;

				RenderSegment(dc, rpt1.x + adder, rpt1.y, rpt2.x, rpt2.y, VP, true);
			}
		}
		rpt1 = rpt2;
		prp1 = prp2;

		++node;
	}
}

void Route::RenderSegment(ocpnDC& dc, int xa, int ya, int xb, int yb, const ViewPort& VP,
						  bool bdraw_arrow, int hilite_width)
{
	static int s_arrow_icon[] = { 0, 0, 5, 2, 18, 6, 12, 0, 18, -6, 5, -2, 0, 0 };
	// Get the dc boundary
	int sx;
	int sy;
	dc.GetSize(&sx, &sy);

	// Try to exit early if the segment is nowhere near the screen
	wxRect r(0, 0, sx, sy);
	wxRect s(xa, ya, 1, 1);
	wxRect t(xb, yb, 1, 1);
	s.Union(t);
	if (!r.Intersects(s))
		return;

	// Clip the line segment to the dc boundary
	int x0 = xa;
	int y0 = ya;
	int x1 = xb;
	int y1 = yb;

	// If hilite is desired, use a Native Graphics context to render alpha colours
	// That is, if wxGraphicsContext is available.....

	if (hilite_width) {
		if (geo::Visible == geo::cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, 0, sx, 0, sy)) {
			wxPen psave = dc.GetPen();

			wxColour y = global::OCPN::get().color().get_color(_T("YELO1"));
			wxColour hilt(y.Red(), y.Green(), y.Blue(), 128);

			wxPen HiPen(hilt, hilite_width, wxSOLID);

			dc.SetPen(HiPen);
			dc.StrokeLine(x0, y0, x1, y1);

			dc.SetPen(psave);
			dc.StrokeLine(x0, y0, x1, y1);
		}
	} else {
		if (geo::Visible == geo::cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, 0, sx, 0, sy))
			dc.StrokeLine(x0, y0, x1, y1);
	}

	if (bdraw_arrow) {
		// Draw a direction arrow

		double theta = atan2(static_cast<double>(yb - ya), static_cast<double>(xb - xa));
		theta -= M_PI / 2;

		wxPoint icon[10];
		double icon_scale_factor = 100 * VP.view_scale();
		icon_scale_factor = fmin(icon_scale_factor, 1.5); // Sets the max size
		icon_scale_factor = fmax(icon_scale_factor, 0.10);

		// Get the absolute line length
		// and constrain the arrow to be no more than xx% of the line length
		double nom_arrow_size = 20.;
		double max_arrow_to_leg = .20;
		double lpp = sqrt(util::sqr(static_cast<double>(xa - xb))
						  + util::sqr(static_cast<double>(ya - yb)));

		double icon_size = icon_scale_factor * nom_arrow_size;
		if (icon_size > (lpp * max_arrow_to_leg))
			icon_scale_factor = (lpp * max_arrow_to_leg) / nom_arrow_size;

		for (int i = 0; i < 7; i++) {
			int j = i * 2;
			double pxa = static_cast<double>(s_arrow_icon[j]);
			double pya = static_cast<double>(s_arrow_icon[j + 1]);

			pya *= icon_scale_factor;
			pxa *= icon_scale_factor;

			double px = (pxa * sin(theta)) + (pya * cos(theta));
			double py = (pya * sin(theta)) - (pxa * cos(theta));

			icon[i].x = static_cast<int>(px) + xb;
			icon[i].y = static_cast<int>(py) + yb;
		}
		wxPen savePen = dc.GetPen();
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.StrokePolygon(6, &icon[0], 0, 0);
		dc.SetPen(savePen);
	}
}

void Route::ClearHighlights(void)
{
	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
		if (*i)
			(*i)->m_bPtIsSelected = false;
	}
}

RoutePoint* Route::InsertPointBefore(RoutePoint* pRP, double rlat, double rlon, bool bRenamePoints)
{
	RoutePoint* newpoint
		= new RoutePoint(geo::Position(rlat, rlon), wxString(_T("diamond")), GetNewMarkSequenced());
	newpoint->m_bIsInRoute = true;
	newpoint->set_dynamic_name(true);
	newpoint->SetNameShown(false);

	RoutePointList::iterator i = std::find(pRoutePointList->begin(), pRoutePointList->end(), pRP);
	pRoutePointList->insert(i, newpoint);

	routePointGUIDs.insert(routePointGUIDs.begin() + (i - pRoutePointList->begin()), pRP->guid());

	m_nPoints++;

	if (bRenamePoints)
		RenameRoutePoints();

	CalculateBBox();
	UpdateSegmentDistances();

	return newpoint;
}

wxString Route::GetNewMarkSequenced(void)
{
	wxString ret = wxString::Format(_T("NM%03d"), m_nm_sequence);
	m_nm_sequence++;
	return ret;
}

RoutePoint* Route::GetLastPoint()
{
	return pRoutePointList->back();
}

int Route::GetIndexOf(RoutePoint* prp)
{
	RoutePointList::iterator i = std::find(pRoutePointList->begin(), pRoutePointList->end(), prp);
	if (i == pRoutePointList->end())
		return 0;

	return 1 + (i - pRoutePointList->begin());
}

const wxString& Route::guid() const
{
	return m_GUID;
}

void Route::set_guid(const wxString& value)
{
	m_GUID = value;
}

void Route::DeletePoint(RoutePoint* rp, bool bRenamePoints)
{
	// n.b. must delete Selectables  and update config before deleting the point
	if (rp->m_bIsInLayer)
		return;

	pSelect->DeleteAllSelectableRoutePoints(this);
	pSelect->DeleteAllSelectableRouteSegments(this);
	pConfig->DeleteWayPoint(rp);

	pRoutePointList->erase(std::find(pRoutePointList->begin(), pRoutePointList->end(), rp));

	if (rp->guid().Len() > 0) {
		GUIDs::iterator i = std::find(routePointGUIDs.begin(), routePointGUIDs.end(), rp->guid());
		if (i != routePointGUIDs.end())
			routePointGUIDs.erase(i);
	}

	delete rp;

	m_nPoints -= 1;

	if (bRenamePoints)
		RenameRoutePoints();

	if (m_nPoints > 1) {
		pSelect->AddAllSelectableRouteSegments(this);
		pSelect->AddAllSelectableRoutePoints(this);

		pConfig->UpdateRoute(this);
		RebuildGUIDList(); // ensure the GUID list is intact and good

		CalculateBBox();
		UpdateSegmentDistances();
	}
}

void Route::RemovePoint(RoutePoint* rp, bool bRenamePoints)
{
	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	if (rp->m_bIsActive && this->IsActive())
		routemanager.DeactivateRoute();

	pSelect->DeleteAllSelectableRoutePoints(this);
	pSelect->DeleteAllSelectableRouteSegments(this);

	pRoutePointList->erase(std::find(pRoutePointList->begin(), pRoutePointList->end(), rp));
	GUIDs::iterator index
		= std::find(routePointGUIDs.begin(), routePointGUIDs.end(), rp->guid());
	if (index != routePointGUIDs.end())
		routePointGUIDs.erase(index);
	m_nPoints -= 1;

	// check all other routes to see if this point appears in any other route
	Route* pcontainer_route = routemanager.FindRouteContainingWaypoint(rp);

	if (pcontainer_route == NULL) {
		rp->m_bIsInRoute = false; // Take this point out of this (and only) route
		rp->set_dynamic_name(false);
		rp->m_bIsolatedMark = true; // This has become an isolated mark
	}

	if (bRenamePoints)
		RenameRoutePoints();

	pSelect->AddAllSelectableRouteSegments(this);
	pSelect->AddAllSelectableRoutePoints(this);

	pConfig->UpdateRoute(this);
	RebuildGUIDList(); // ensure the GUID list is intact and good

	CalculateBBox();
	UpdateSegmentDistances();
}

void Route::DeSelectRoute()
{
	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i)
		(*i)->m_bPtIsSelected = false;
}

void Route::ReloadRoutePointIcons()
{
	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i)
		(*i)->ReLoadIcon();
}

void Route::CalculateBBox()
{
	double bbox_xmin = 180.0;
	double bbox_ymin = 90.0;
	double bbox_xmax = -180.0;
	double bbox_ymax = -90.0;

	RBBox.Reset();
	m_bcrosses_idl = CalculateCrossesIDL();

	if (!m_bcrosses_idl) {
		for (RoutePointList::const_iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
			const RoutePoint* point = *i;

			if (point->longitude() > bbox_xmax)
				bbox_xmax = point->longitude();
			if (point->longitude() < bbox_xmin)
				bbox_xmin = point->longitude();
			if (point->latitude() > bbox_ymax)
				bbox_ymax = point->latitude();
			if (point->latitude() < bbox_ymin)
				bbox_ymin = point->latitude();
		}
	} else {
		// For Routes that cross the IDL, we compute and store
		// the bbox as positive definite
		for (RoutePointList::const_iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
			const RoutePoint* point = *i;
			double lon = point->longitude();
			if (lon < 0.0)
				lon += 360.0;

			if (lon > bbox_xmax)
				bbox_xmax = lon;
			if (lon < bbox_xmin)
				bbox_xmin = lon;
			if (point->latitude() > bbox_ymax)
				bbox_ymax = point->latitude();
			if (point->latitude() < bbox_ymin)
				bbox_ymin = point->latitude();
		}
	}

	RBBox.Expand(bbox_xmin, bbox_ymin);
	RBBox.Expand(bbox_xmax, bbox_ymax);
}

bool Route::CalculateCrossesIDL(void)
{
	RoutePointList::iterator route_point = pRoutePointList->begin();
	if (route_point == pRoutePointList->end())
		return false;

	bool idl_cross = false;
	RoutePoint* data = *route_point;

	double lon0 = data->longitude();

	++route_point;
	for(; route_point != pRoutePointList->end(); ++route_point) {
		data = *route_point;
		if ((lon0 < -150.0) && (data->longitude() > 150.0)) {
			idl_cross = true;
			break;
		}

		if ((lon0 > 150.0) && (data->longitude() < -150.0)) {
			idl_cross = true;
			break;
		}

		lon0 = data->longitude();
	}

	return idl_cross;
}

void Route::CalculateDCRect(wxDC& dc_route, wxRect& prect, const ViewPort&)
{
	dc_route.ResetBoundingBox();
	dc_route.DestroyClippingRegion();

	// Draw the route in skeleton form on the dc
	// That is, draw only the route points, assuming that the segements will
	// always be fully contained within the resulting rectangle.
	// Can we prove this?
	if (m_bVisible) {
		for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end();
			 ++i) {
			RoutePoint* prp2 = *i;
			bool blink_save = prp2->is_blink();
			prp2->set_blink(false);
			ocpnDC odc_route(dc_route);
			prp2->Draw(odc_route, NULL);
			prp2->set_blink(blink_save);
		}
	}

	// Retrieve the drawing extents
	prect.x = dc_route.MinX() - 1;
	prect.y = dc_route.MinY() - 1;
	prect.width = dc_route.MaxX() - dc_route.MinX() + 2;
	prect.height = dc_route.MaxY() - dc_route.MinY() + 2;
}

//  Update the route segment lengths, storing each segment length in <destination> point.
//  Also, compute total route length by summing segment distances.
void Route::UpdateSegmentDistances(double planspeed)
{
	wxPoint rpt, rptn;
	double slat1;
	double slon1;
	double slat2;
	double slon2;

	double route_len = 0.0;
	double route_time = 0.0;

	RoutePointList::iterator node = pRoutePointList->begin();

	if (node != pRoutePointList->end()) {
		RoutePoint* prp0 = *node;
		slat1 = prp0->latitude();
		slon1 = prp0->longitude();

		++node;

		while (node != pRoutePointList->end()) {
			RoutePoint* prp = *node;
			slat2 = prp->latitude();
			slon2 = prp->longitude();

			// Calculate the absolute distance from 1->2

			double brg;
			double dd;
			geo::DistanceBearingMercator(geo::Position(slat1, slon1), geo::Position(slat2, slon2), &brg, &dd);

			// And store in Point 2
			prp->m_seg_len = dd;

			route_len += dd;

			slat1 = slat2;
			slon1 = slon2;

			// If Point1 Description contains VMG, store it for Properties Dialog in Point2
			// If Point1 Description contains ETD, store it in Point1

			if (planspeed > 0.0) {
				double vmg = 0.0;
				wxDateTime etd;

				if (prp0->get_description().Find(_T("VMG=")) != wxNOT_FOUND) {
					wxString s_vmg
						= (prp0->get_description().Mid(prp0->get_description().Find(_T("VMG=")) + 4))
							  .BeforeFirst(';');
					if (!s_vmg.ToDouble(&vmg))
						vmg = planspeed;
				}

				double legspeed = planspeed;
				if (vmg > 0.1 && vmg < 1000.0)
					legspeed = vmg;
				if (legspeed > 0.1 && legspeed < 1000.0) {
					route_time += dd / legspeed;
					prp->m_seg_vmg = legspeed;
				}

				prp0->m_seg_etd = wxInvalidDateTime;
				if (prp0->get_description().Find(_T("ETD=")) != wxNOT_FOUND) {
					wxString s_etd
						= (prp0->get_description().Mid(prp0->get_description().Find(_T("ETD=")) + 4))
							  .BeforeFirst(';');
					const wxChar* parse_return = etd.ParseDateTime(s_etd);
					if (parse_return) {
						wxString tz(parse_return);

						if (tz.Find(_T("UT")) != wxNOT_FOUND)
							prp0->m_seg_etd = etd;
						else if (tz.Find(_T("LMT")) != wxNOT_FOUND) {
							prp0->m_seg_etd = etd;
							long lmt_offset = static_cast<long>((prp0->longitude() * 3600.0) / 15.0);
							wxTimeSpan lmt(0, 0, static_cast<int>(lmt_offset), 0);
							prp0->m_seg_etd -= lmt;
						} else
							prp0->m_seg_etd = etd.ToUTC();
					}
				}
			}

			prp0 = prp;
			++node;
		}
	}

	m_route_length = route_len;
	m_route_time = route_time * 3600.0;
}

void Route::Reverse(bool bRenamePoints)
{
	RebuildGUIDList(); // ensure the GUID list is intact and good

	std::reverse(routePointGUIDs.begin(), routePointGUIDs.end());

	pRoutePointList->clear();
	m_nPoints = 0;
	m_route_length = 0.0;

	AssembleRoute(); // Rebuild the route points from the GUID list

	if (bRenamePoints)
		RenameRoutePoints();

	// Switch start/end strings. anders, 2010-01-29
	wxString tmp = m_RouteStartString;
	m_RouteStartString = m_RouteEndString;
	m_RouteEndString = tmp;
}

void Route::RebuildGUIDList(void)
{
	routePointGUIDs.clear();
	for (RoutePointList::const_iterator i = pRoutePointList->begin(); i != pRoutePointList->end();
		 ++i) {
		routePointGUIDs.push_back((*i)->guid());
	}
}

void Route::SetVisible(bool visible, bool includeWpts)
{
	m_bVisible = visible;

	if (!includeWpts)
		return;

	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
		if ((*i)->m_bKeepXRoute) {
			(*i)->SetVisible(visible);
		}
	}
}

void Route::SetListed(bool visible)
{
	m_bListed = visible;
}

void Route::AssembleRoute(void)
{
	navigation::WaypointManager& waypointmanager = global::OCPN::get().waypointman();
	for (GUIDs::const_iterator ip = routePointGUIDs.begin(); ip != routePointGUIDs.end(); ++ip) {
		RoutePoint* point = waypointmanager.find(*ip);
		if (point)
			AddPoint(point);
	}
}

void Route::RenameRoutePoints(void)
{
	// iterate on the route points.
	// If dynamically named, rename according to current list position

	int count = 1;
	for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
		RoutePoint* prp = *i;
		if (prp->is_dynamic_name()) {
			prp->SetName(wxString::Format(_T("%03d"), count));
		}
		++count;
	}
}

bool Route::SendToGPS(const wxString& com_name, bool bsend_waypoints, wxGauge* pProgress)
{
	bool result = false;

	if (g_pMUX) {
		::wxBeginBusyCursor();
		result = g_pMUX->SendRouteToGPS(this, com_name, bsend_waypoints, pProgress);
		::wxEndBusyCursor();
	}

	wxString msg;
	if (result)
		msg = _("Route Uploaded successfully.");
	else
		msg = _("Error on Route Upload.  Please check logfiles...");

	OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION);

	return result;
}

// Is this route equal to another, meaning,
// Do all routepoint positions and names match?
bool Route::IsEqualTo(Route* ptargetroute)
{
	RoutePointList::const_iterator point_a = this->pRoutePointList->begin();
	RoutePointList::const_iterator point_b = ptargetroute->pRoutePointList->begin();

	if (point_a == this->pRoutePointList->end())
		return false;

	if (this->m_bIsInLayer || ptargetroute->m_bIsInLayer)
		return false;

	if (this->GetnPoints() != ptargetroute->GetnPoints())
		return false;

	while (point_a != this->pRoutePointList->end()) {
		if (point_b == ptargetroute->pRoutePointList->end())
			return false;

		const RoutePoint* pthisrp = *point_a;
		const RoutePoint* pthatrp = *point_b;

		if ((fabs(pthisrp->latitude() - pthatrp->latitude()) > 1.0e-6) // FIXME
			|| (fabs(pthisrp->longitude() - pthatrp->longitude()) > 1.0e-6))
			return false;

		if (!pthisrp->GetName().IsSameAs(pthatrp->GetName()))
			return false;

		++point_a;
		++point_b;
	}

	return true;
}

/// Walk through all route points to find the best fitting (nearest).
/// This method returns NULL if no routepoints are found.
RoutePoint* Route::FindBestActivatePoint(const geo::Position& pos, double cog)
{
	RoutePoint* best_point = NULL;
	double min_time_found = 1e6;

	for (RoutePointList::iterator i = routepoints().begin(); i != routepoints().end(); ++i) {
		RoutePoint* pn = *i;

		double brg;
		double dist;
		geo::DistanceBearingMercator(pn->get_position(), pos, &brg, &dist);

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

