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
#include <Routeman.h>
#include <WayPointman.h>
#include <Multiplexer.h>
#include <FontMgr.h>
#include <MessageBox.h>
#include <ocpnDC.h>
#include <ChartCanvas.h>
#include <MainFrame.h>

#include <gpx/ParseGPXDateTime.h>

#include <wx/dcscreen.h>
#include <wx/tokenzr.h>

extern WayPointman* pWayPointMan;
extern bool g_bIsNewLayer;
extern int g_LayerIdx;
extern ChartCanvas* cc1;
extern Routeman* g_pRouteMan;
extern wxRect g_blink_rect;
extern Multiplexer* g_pMUX;
extern MainFrame* gFrame;

RoutePoint::RoutePoint()
	: m_seg_len(0.0)
	, m_seg_vmg(0.0)
	, m_seg_etd(wxInvalidDateTime)
	, m_bPtIsSelected(false)
	, m_bIsBeingEdited(false)
	, m_bIsInRoute(false)
	, m_bIsInTrack(false)
	, m_bIsolatedMark(false)
	, m_bKeepXRoute(false)
	, m_bIsVisible(true)
	, m_bIsListed(true)
	, m_bIsActive(false)
	, m_IconName(wxEmptyString)
	, m_pMarkFont(NULL)
	, m_pbmIcon(NULL)
	, m_bBlink(false)
	, m_bDynamicName(false)
	, m_bShowName(true)
	, CurrentRect_in_DC(0, 0, 0, 0)
	, m_NameLocationOffsetX(-10)
	, m_NameLocationOffsetY(8)
	, m_GPXTrkSegNo(1)
	, m_bIsInLayer(false)
	, m_LayerID(0)
	, m_btemp(false)
	, m_MarkName(wxEmptyString)
{
	m_CreateTimeX = wxDateTime::Now();
	m_GUID = pWayPointMan->CreateGUID(this);
	ReLoadIcon();
}

RoutePoint::RoutePoint(RoutePoint* orig)
{
	m_MarkName = orig->GetName();
	m_lat = orig->m_lat;
	m_lon = orig->m_lon;
	m_seg_len = orig->m_seg_len;
	m_seg_vmg = orig->m_seg_vmg;
	m_seg_etd = orig->m_seg_etd;
	m_bDynamicName = orig->m_bDynamicName;
	m_bPtIsSelected = orig->m_bPtIsSelected;
	m_bIsBeingEdited = orig->m_bIsBeingEdited;
	m_bIsActive = orig->m_bIsActive;
	m_bBlink = orig->m_bBlink;
	m_bIsInRoute = orig->m_bIsInRoute;
	m_bIsInTrack = orig->m_bIsInTrack;
	m_CreateTimeX = orig->m_CreateTimeX;
	m_GPXTrkSegNo = orig->m_GPXTrkSegNo;
	m_bIsolatedMark = orig->m_bIsolatedMark;
	m_bShowName = orig->m_bShowName;
	m_bKeepXRoute = orig->m_bKeepXRoute;
	m_bIsVisible = orig->m_bIsVisible;
	m_bIsListed = orig->m_bIsListed;
	CurrentRect_in_DC = orig->CurrentRect_in_DC;
	m_NameLocationOffsetX = orig->m_NameLocationOffsetX;
	m_NameLocationOffsetY = orig->m_NameLocationOffsetY;
	m_pMarkFont = orig->m_pMarkFont;
	m_MarkDescription = orig->m_MarkDescription;
	m_btemp = orig->m_btemp;

	m_IconName = orig->m_IconName;
	ReLoadIcon();

	m_bIsInLayer = orig->m_bIsInLayer;
	m_GUID = pWayPointMan->CreateGUID(this);
}

RoutePoint::RoutePoint(double lat, double lon, const wxString& icon_ident, const wxString& name,
					   const wxString& pGUID, bool bAddToList)
	: m_lat(lat)
	, m_lon(lon)
	, m_seg_len(0)
	, m_seg_vmg(0.0)
	, m_seg_etd(wxInvalidDateTime)
	, m_bPtIsSelected(false)
	, m_bIsBeingEdited(false)
	, m_bIsInRoute(false)
	, m_bIsInTrack(false)
	, m_bIsolatedMark(false)
	, m_bKeepXRoute(false)
	, m_bIsVisible(true)
	, m_bIsListed(true)
	, m_bIsActive(false)
	, m_IconName(wxEmptyString)
	, m_pMarkFont(NULL)
	, m_pbmIcon(NULL)
	, m_bBlink(false)
	, m_bDynamicName(false)
	, m_bShowName(true)
	, CurrentRect_in_DC(0, 0, 0, 0)
	, m_NameLocationOffsetX(-10)
	, m_NameLocationOffsetY(8)
	, m_GPXTrkSegNo(1)
	, m_bIsInLayer(false)
	, m_LayerID(0)
	, m_btemp(false)
	, m_MarkName(wxEmptyString)
{
	// Normalize the longitude, to fix any old poorly formed points
	if (m_lon < -180.0)
		m_lon += 360.0;
	else if (m_lon > 180.0)
		m_lon -= 360.0;

	if (!pGUID.IsEmpty())
		m_GUID = pGUID;
	else
		m_GUID = pWayPointMan->CreateGUID(this);

	// Get Icon bitmap
	m_IconName = icon_ident;
	ReLoadIcon();

	SetName(name);

	// Possibly add the waypoint to the global list maintained by the waypoint manager

	if (bAddToList && NULL != pWayPointMan)
		pWayPointMan->push_back(this);

	m_bIsInLayer = g_bIsNewLayer;
	if (m_bIsInLayer) {
		m_LayerID = g_LayerIdx;
		m_bIsListed = false;
	} else
		m_LayerID = 0;
}

RoutePoint::~RoutePoint(void)
{
	// FIXME: what a mess: Remove this point from the global waypoint list
	if (pWayPointMan)
		pWayPointMan->remove(this);


	m_HyperlinkList.clear();
}

wxDateTime RoutePoint::GetCreateTime() // FIXME: fix this brain-dead interface
{
	if (!m_CreateTimeX.IsValid()) {
		if (m_timestring.Len())
			ParseGPXDateTime(m_CreateTimeX, m_timestring);
	}
	return m_CreateTimeX;
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
	m_pbmIcon = pWayPointMan->GetIconBitmap(m_IconName);
}

void RoutePoint::Draw(ocpnDC& dc, wxPoint* rpn)
{
	wxPoint r;
	wxRect hilitebox;
	unsigned char transparency = 100;

	cc1->GetCanvasPointPix(m_lat, m_lon, &r);

	//  return the home point in this dc to allow "connect the dots"
	if (NULL != rpn)
		*rpn = r;

	if (!m_bIsVisible /*&& !m_bIsInTrack*/) // pjotrc 2010.02.13, 2011.02.24
		return;

	//    Optimization, especially apparent on tracks in normal cases
	if (m_IconName == _T("empty") && !m_bShowName && !m_bPtIsSelected)
		return;

	wxPen* pen;
	if (m_bBlink)
		pen = g_pRouteMan->GetActiveRoutePointPen();
	else
		pen = g_pRouteMan->GetRoutePointPen();

	//    Substitue icon?
	wxBitmap* pbm;
	if ((m_bIsActive) && (m_IconName != _T("mob")))
		pbm = pWayPointMan->GetIconBitmap(_T ( "activepoint" ));
	else
		pbm = m_pbmIcon;

	int sx2 = pbm->GetWidth() / 2;
	int sy2 = pbm->GetHeight() / 2;

	//    Calculate the mark drawing extents
	wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2); // the bitmap extents

	if (m_bShowName) {
		if (0 == m_pMarkFont) {
			m_pMarkFont = FontMgr::Get().GetFont(_("Marks"));
			m_FontColor = FontMgr::Get().GetFontColor(_("Marks"));
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

	//  Highlite any selected point
	if (m_bPtIsSelected) {
		dc.AlphaBlending(r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height,
						 0.0, pen->GetColour(), transparency);
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

	//  Save the current draw rectangle in the current DC
	//    This will be useful for fast icon redraws
	CurrentRect_in_DC.x = r.x + hilitebox.x;
	CurrentRect_in_DC.y = r.y + hilitebox.y;
	CurrentRect_in_DC.width = hilitebox.width;
	CurrentRect_in_DC.height = hilitebox.height;

	if (m_bBlink)
		g_blink_rect = CurrentRect_in_DC; // also save for global blinker
}

void RoutePoint::SetPosition(double lat, double lon)
{
	m_lat = lat;
	m_lon = lon;
}

void RoutePoint::CalculateDCRect(wxDC& dc, wxRect* prect)
{
	dc.ResetBoundingBox();
	dc.DestroyClippingRegion();

	// Draw the mark on the dc
	ocpnDC odc(dc);
	Draw(odc, NULL);

	//  Retrieve the drawing extents
	prect->x = dc.MinX() - 1;
	prect->y = dc.MinY() - 1;
	prect->width = dc.MaxX() - dc.MinX() + 2; // Mouse Poop?
	prect->height = dc.MaxY() - dc.MinY() + 2;
}

bool RoutePoint::IsSame(RoutePoint* pOtherRP)
{
	bool IsSame = false;

	if (this->m_MarkName == pOtherRP->m_MarkName) {
		if (fabs(this->m_lat - pOtherRP->m_lat) < 1.e-6 && fabs(this->m_lon - pOtherRP->m_lon)
														   < 1.e-6)
			IsSame = true;
	}
	return IsSame;
}

bool RoutePoint::SendToGPS(const wxString& com_name, wxGauge* pProgress)
{
	bool result = false;
	if (g_pMUX)
		result = g_pMUX->SendWaypointToGPS(this, com_name, pProgress);

	wxString msg;
	if (result)
		msg = _("Waypoint(s) Uploaded successfully.");
	else
		msg = _("Error on Waypoint Upload.  Please check logfiles...");

	OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxOK | wxICON_INFORMATION);

	return result;
}

double RoutePoint::GetLatitude() const
{
	return m_lat;
}

double RoutePoint::GetLongitude() const
{
	return m_lon;
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

wxString RoutePoint::GetName(void) const
{
	return m_MarkName;
}

wxString RoutePoint::GetDescription(void) const
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

