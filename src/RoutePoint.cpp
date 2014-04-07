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

#include "RoutePoint.h"
#include <Multiplexer.h>
#include <MessageBox.h>
#include <ocpnDC.h>
#include <ChartCanvas.h>
#include <MainFrame.h>

#include <gui/FontManager.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>

#include <util/uuid.h>

#include <navigation/RouteManager.h>
#include <navigation/WaypointManager.h>

#include <gpx/ParseGPXDateTime.h>

#include <wx/dcscreen.h>
#include <wx/tokenzr.h>

extern bool g_bIsNewLayer;
extern int g_LayerIdx;
extern ChartCanvas* cc1;
extern wxRect g_blink_rect;
extern Multiplexer* g_pMUX;
extern MainFrame* gFrame;


RoutePoint::SameGUID::SameGUID(const wxString& guid)
	: guid(guid)
{
}

bool RoutePoint::SameGUID::operator()(const RoutePoint* point) const
{
	return point && (guid == point->guid());
}

RoutePoint::Segment::Segment()
	: length(0.0)
	, vmg(0.0)
	, etd(wxInvalidDateTime)
{
}

RoutePoint::RoutePoint()
	: m_bPtIsSelected(false)
	, m_bIsBeingEdited(false)
	, m_bIsInRoute(false)
	, m_bIsInTrack(false)
	, m_bIsolatedMark(false)
	, m_bKeepXRoute(false)
	, m_bIsListed(true)
	, m_bIsActive(false)
	, m_pbmIcon(NULL)
	, CurrentRect_in_DC(0, 0, 0, 0)
	, m_NameLocationOffsetX(-10)
	, m_NameLocationOffsetY(8)
	, m_GPXTrkSegNo(1)
	, m_bIsInLayer(false)
	, m_routeprop_course(0.0)
	, m_routeprop_distance(0.0)
	, m_btemp(false)
	, m_LayerID(0)
	, m_bBlink(false)
	, m_bIsVisible(true)
	, m_bShowName(true)
	, m_bDynamicName(false)
	, m_MarkName(wxEmptyString)
	, m_IconName(wxEmptyString)
	, m_GUID(wxEmptyString)
	, m_pMarkFont(NULL)
{
	m_CreateTimeX = wxDateTime::Now();
	m_GUID = wxString(util::uuid().c_str(), wxConvUTF8);
	ReLoadIcon();
}

RoutePoint::RoutePoint(const RoutePoint& orig)
{
	m_MarkName = orig.GetName();
	position = orig.position;
	segment = orig.segment;
	m_bDynamicName = orig.m_bDynamicName;
	m_bPtIsSelected = orig.m_bPtIsSelected;
	m_bIsActive = orig.m_bIsActive;
	m_bBlink = orig.m_bBlink;
	m_bIsInRoute = orig.m_bIsInRoute;
	m_bIsInTrack = orig.m_bIsInTrack;
	m_CreateTimeX = orig.m_CreateTimeX;
	m_GPXTrkSegNo = orig.m_GPXTrkSegNo;
	m_bIsolatedMark = orig.m_bIsolatedMark;
	m_bShowName = orig.m_bShowName;
	m_bKeepXRoute = orig.m_bKeepXRoute;
	m_bIsVisible = orig.m_bIsVisible;
	m_bIsListed = orig.m_bIsListed;
	CurrentRect_in_DC = orig.CurrentRect_in_DC;
	m_NameLocationOffsetX = orig.m_NameLocationOffsetX;
	m_NameLocationOffsetY = orig.m_NameLocationOffsetY;
	m_pMarkFont = orig.m_pMarkFont;
	m_MarkDescription = orig.m_MarkDescription;
	m_btemp = orig.m_btemp;

	m_IconName = orig.m_IconName;
	ReLoadIcon();

	m_bIsInLayer = orig.m_bIsInLayer;
	m_GUID = wxString(util::uuid().c_str(), wxConvUTF8);
}

RoutePoint::RoutePoint(const geo::Position& pos, const wxString& icon_ident, const wxString& name,
					   const wxString& pGUID, bool bAddToList)
	: m_bPtIsSelected(false)
	, m_bIsInRoute(false)
	, m_bIsInTrack(false)
	, m_bIsolatedMark(false)
	, m_bKeepXRoute(false)
	, m_bIsListed(true)
	, m_bIsActive(false)
	, m_pbmIcon(NULL)
	, CurrentRect_in_DC(0, 0, 0, 0)
	, m_NameLocationOffsetX(-10)
	, m_NameLocationOffsetY(8)
	, m_GPXTrkSegNo(1)
	, m_bIsInLayer(false)
	, m_routeprop_course(0.0)
	, m_routeprop_distance(0.0)
	, m_btemp(false)
	, position(pos)
	, m_LayerID(0)
	, m_bBlink(false)
	, m_bIsVisible(true)
	, m_bShowName(true)
	, m_bDynamicName(false)
	, m_MarkName(wxEmptyString)
	, m_IconName(icon_ident)
	, m_GUID(wxEmptyString)
	, m_pMarkFont(NULL)
{
	position.normalize_lon();

	if (!pGUID.IsEmpty())
		m_GUID = pGUID;
	else
		m_GUID = wxString(util::uuid().c_str(), wxConvUTF8);

	// Get Icon bitmap
	ReLoadIcon();

	SetName(name);

	// Possibly add the waypoint to the global list maintained by the waypoint manager

	if (bAddToList)
		global::OCPN::get().waypointman().push_back(this); // FIXME: refactoring

	m_bIsInLayer = g_bIsNewLayer;
	if (m_bIsInLayer) {
		m_LayerID = g_LayerIdx;
		m_bIsListed = false;
	} else {
		m_LayerID = 0;
	}
}

RoutePoint::~RoutePoint(void)
{
	global::OCPN::get().waypointman().remove(this); // FIXME: refactoring
	m_HyperlinkList.clear();
}

bool RoutePoint::is_isolated() const
{
	return m_bIsolatedMark;
}

bool RoutePoint::is_in_route() const
{
	return m_bIsInRoute;
}

bool RoutePoint::is_in_track() const
{
	return m_bIsInTrack;
}

int RoutePoint::get_layer_ID() const
{
	return m_LayerID;
}

void RoutePoint::set_layer_ID(int id)
{
	m_LayerID = id;
}

const wxDateTime& RoutePoint::GetCreateTime() const
{
	return m_CreateTimeX;
}

void RoutePoint::set_time_string(const wxString& time_string)
{
	SetCreateTime(wxInvalidDateTime);
	m_timestring = time_string;
	if (!m_CreateTimeX.IsValid()) {
		if (m_timestring.size())
			gpx::ParseGPXDateTime(m_CreateTimeX, m_timestring);
	}
}

const wxString& RoutePoint::get_time_string() const
{
	return m_timestring;
}

void RoutePoint::SetCreateTime(wxDateTime dt)
{
	m_CreateTimeX = dt;
}

void RoutePoint::SetName(const wxString& name)
{
	m_MarkName = name;
	CalculateNameExtents();
}

void RoutePoint::CalculateNameExtents(void)
{
	if (m_pMarkFont) {
		wxScreenDC dc;

		dc.SetFont(*m_pMarkFont);
		m_NameExtents = dc.GetTextExtent(m_MarkName);
	} else
		m_NameExtents = wxSize(0, 0);
}

void RoutePoint::ReLoadIcon(void)
{
	m_pbmIcon = global::OCPN::get().waypointman().GetIconBitmap(icon_name());
}

void RoutePoint::Draw(ocpnDC& dc, wxPoint* rpn)
{
	wxRect hilitebox;

	wxPoint r = cc1->GetCanvasPointPix(position);

	// return the home point in this dc to allow "connect the dots"
	if (NULL != rpn)
		*rpn = r;

	if (!m_bIsVisible)
		return;

	// Optimization, especially apparent on tracks in normal cases
	if (m_IconName == _T("empty") && !m_bShowName && !m_bPtIsSelected)
		return;

	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	wxColour route_point_color;
	if (m_bBlink)
		route_point_color = routemanager.GetActiveRoutePointPen().GetColour();
	else
		route_point_color = routemanager.GetRoutePointPen().GetColour();

	// Substitue icon?
	wxBitmap* pbm;
	if ((m_bIsActive) && (m_IconName != _T("mob")))
		pbm = global::OCPN::get().waypointman().GetIconBitmap(_T("activepoint"));
	else
		pbm = m_pbmIcon;

	int sx2 = pbm->GetWidth() / 2;
	int sy2 = pbm->GetHeight() / 2;

	// Calculate the mark drawing extents
	wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2); // the bitmap extents

	// FIXME: late load of name
	if (m_bShowName) {
		if (0 == m_pMarkFont) {
			gui::FontManager& fonts = global::OCPN::get().font();
			m_pMarkFont = fonts.GetFont(_("Marks"));
			m_FontColor = fonts.GetFontColor(_("Marks"));
			CalculateNameExtents();
		}

		if (m_pMarkFont) {
			wxRect r2(r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY, m_NameExtents.x,
					  m_NameExtents.y);
			r1.Union(r2);
		}
	}

	hilitebox = r1;
	hilitebox.x -= r.x;
	hilitebox.y -= r.y;
	hilitebox.Inflate(2);

	wxColour hi_colour = route_point_color;
	unsigned char transparency = 100;
	if (m_bIsBeingEdited) {
		hi_colour = global::OCPN::get().color().get_color(_T("YELO1"));
		transparency = 150;
	}

	// Highlite any selected point
	if (m_bPtIsSelected || m_bIsBeingEdited) {
		dc.AlphaBlending(r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height,
						 0.0, hi_colour, transparency);
	}

	bool bDrawHL = false;

	if (m_bBlink && gFrame->is_route_blink_odd())
		bDrawHL = true;

	if ((!bDrawHL) && (NULL != m_pbmIcon)) {
		dc.DrawBitmap(*pbm, r.x - sx2, r.y - sy2, true);
		// on MSW, the dc Bounding box is not updated on DrawBitmap() method.
		// Do it explicitely here for all platforms.
		dc.CalcBoundingBox(r.x - sx2, r.y - sy2);
		dc.CalcBoundingBox(r.x + sx2, r.y + sy2);
	}

	if (m_bShowName) {
		if (m_pMarkFont) {
			dc.SetFont(*m_pMarkFont);
			dc.SetTextForeground(m_FontColor);

			dc.DrawText(m_MarkName, r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY);
		}
	}

	// Save the current draw rectangle in the current DC
	// This will be useful for fast icon redraws FIXME:there must be another solution for this
	CurrentRect_in_DC.x = r.x + hilitebox.x;
	CurrentRect_in_DC.y = r.y + hilitebox.y;
	CurrentRect_in_DC.width = hilitebox.width;
	CurrentRect_in_DC.height = hilitebox.height;

	if (m_bBlink)
		g_blink_rect = CurrentRect_in_DC; // also save for global blinker FIXME: sideeffect
}

const geo::Position& RoutePoint::get_position() const
{
	return position;
}

void RoutePoint::set_position(const geo::Position& pos)
{
	position = pos;
}

double RoutePoint::latitude() const
{
	return position.lat();
}

double RoutePoint::longitude() const
{
	return position.lon();
}

void RoutePoint::CalculateDCRect(wxDC& dc, wxRect& prect)
{
	dc.ResetBoundingBox();
	dc.DestroyClippingRegion();

	// Draw the mark on the dc
	ocpnDC odc(dc);
	Draw(odc, NULL);

	// Retrieve the drawing extents
	prect.x = dc.MinX() - 1;
	prect.y = dc.MinY() - 1;
	prect.width = dc.MaxX() - dc.MinX() + 2; // Mouse Poop?
	prect.height = dc.MaxY() - dc.MinY() + 2;
}

bool RoutePoint::IsSame(const RoutePoint* pOtherRP) const
{
	if (m_MarkName != pOtherRP->m_MarkName)
		return false;

	// FIXME: this calculates the minimal distance of a position difference, but wrong
	if ((fabs(position.lat() - pOtherRP->position.lat()) < 1.e-6)
		&& (fabs(position.lon() - pOtherRP->position.lon()) < 1.e-6))
		return true;
	return false;
}

bool RoutePoint::SendToGPS(const wxString& com_name, wxGauge* pProgress)
{
	int result = 0;
	if (g_pMUX)
		result = g_pMUX->SendWaypointToGPS(this, com_name, pProgress);

	wxString msg;
	if (result == 0) {
		msg = _("Waypoint(s) Uploaded successfully.");
	} else if (result == ERR_GARMIN_INITIALIZE) {
		msg = _("Error on Waypoint Upload.  Garmin GPS not connected");
	} else {
		msg = _("Error on Waypoint Upload.  Please check logfiles...");
	}

	OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION);

	return result == 0;
}

bool RoutePoint::IsVisible() const
{
	return m_bIsVisible;
}

bool RoutePoint::IsListed() const
{
	return m_bIsListed;
}

bool RoutePoint::IsNameShown() const
{
	return m_bShowName;
}

void RoutePoint::SetVisible(bool viz)
{
	m_bIsVisible = viz;
}

void RoutePoint::SetListed(bool viz)
{
	m_bIsListed = viz;
}

void RoutePoint::SetNameShown(bool viz)
{
	m_bShowName = viz;
}

const wxString& RoutePoint::guid() const
{
	return m_GUID;
}

void RoutePoint::set_guid(const wxString& value)
{
	m_GUID = value;
}

const wxString& RoutePoint::icon_name() const
{
	return m_IconName;
}

void RoutePoint::set_icon_name(const wxString& value)
{
	m_IconName = value;
}

const wxString& RoutePoint::GetName(void) const
{
	return m_MarkName;
}

void RoutePoint::set_description(const wxString& desc)
{
	m_MarkDescription = desc;
}

const wxString& RoutePoint::get_description(void) const
{
	return m_MarkDescription;
}

void RoutePoint::SetCourse(double course)
{
	m_routeprop_course = course;
}

double RoutePoint::GetCourse() const
{
	return m_routeprop_course;
}

void RoutePoint::SetDistance(double distance)
{
	m_routeprop_distance = distance;
}

double RoutePoint::GetDistance() const
{
	return m_routeprop_distance;
}

void RoutePoint::clear_font()
{
	m_pMarkFont = NULL;
}

bool RoutePoint::is_blink() const
{
	return m_bBlink;
}

void RoutePoint::set_blink(bool value)
{
	m_bBlink = value;
}

bool RoutePoint::is_visible() const
{
	return m_bIsVisible;
}

void RoutePoint::set_visible(bool value)
{
	m_bIsVisible = value;
}

bool RoutePoint::is_show_name() const
{
	return m_bShowName;
}

void RoutePoint::set_show_name(bool value)
{
	m_bShowName = value;
}

bool RoutePoint::is_dynamic_name() const
{
	return m_bDynamicName;
}

void RoutePoint::set_dynamic_name(bool value)
{
	m_bDynamicName = value;
}

const Hyperlinks& RoutePoint::get_hyperlinks() const
{
	return m_HyperlinkList;
}

void RoutePoint::add_link(const Hyperlink& link)
{
	m_HyperlinkList.push_back(link);
}

