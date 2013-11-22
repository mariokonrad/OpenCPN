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

#include "OCPNTrackListCtrl.h"
#include <Route.h>
#include <RoutePoint.h>
#include <PositionParser.h>
#include <Units.h>

#include <global/OCPN.h>
#include <global/Navigation.h>

#include <geo/GeoRef.h>

// FIXME: fix this crap: it makes the class essentially a singleton
static wxRoutePointListNode* g_this_point_node = NULL;
static wxRoutePointListNode* g_prev_point_node = NULL;
static RoutePoint* g_this_point;
static RoutePoint* g_prev_point;
static int g_prev_point_index = -1;
static int g_prev_item = -1;
static double gt_brg = 0.0;
static double gt_leg_dist = 0.0;

#define UTCINPUT         0
#define LTINPUT          1    // i.e. this PC local time
#define LMTINPUT         2    // i.e. the remote location LMT time
#define INPUT_FORMAT     1
#define DISPLAY_FORMAT   2
#define TIMESTAMP_FORMAT 3

static wxString timestamp2s(wxDateTime ts, int tz_selection, long LMT_offset, int format)
{
	wxString s = _T("");
	wxString f;

	if (format == INPUT_FORMAT)
		f = _T("%m/%d/%Y %H:%M");
	else if (format == TIMESTAMP_FORMAT)
		f = _T("%m/%d/%Y %H:%M:%S");
	else
		f = _T(" %m/%d %H:%M");

	switch (tz_selection) {
		case 0:
			s.Append(ts.Format(f));
			if (format != INPUT_FORMAT)
				s.Append(_T(" UT"));
			break;
		case 1:
			s.Append(ts.FromUTC().Format(f));
			break;
		case 2:
			wxTimeSpan lmt(0, 0, (int)LMT_offset, 0);
			s.Append(ts.Add(lmt).Format(f));
			if (format != INPUT_FORMAT)
				s.Append(_T(" LMT"));
			break;
	}
	return s;
}

OCPNTrackListCtrl::OCPNTrackListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
									 const wxSize& size, long style)
	: wxListCtrl(parent, id, pos, size, style)
	, m_pRoute(NULL)
	, m_tz_selection(0)
	, m_LMT_Offset(0)
{
	g_prev_item = -1;
}

OCPNTrackListCtrl::~OCPNTrackListCtrl()
{}

void OCPNTrackListCtrl::set_route(Route* route)
{
	m_pRoute = route;
}

wxString OCPNTrackListCtrl::leg_id(long item) const
{
	if (item == 0)
		return _T("---");
	return wxString::Format(_T("%ld"), item);
}

wxString OCPNTrackListCtrl::latitude(const RoutePoint& point) const
{
	return toSDMM(1, point.m_lat, 1);
}

wxString OCPNTrackListCtrl::longitude(const RoutePoint& point) const
{
	return toSDMM(2, point.m_lon, 1);
}

wxString OCPNTrackListCtrl::timestamp(const RoutePoint& point) const
{
	wxDateTime t = point.GetCreateTime();
	if (t.IsValid())
		return timestamp2s(t, m_tz_selection, m_LMT_Offset, TIMESTAMP_FORMAT);
	return _T("----");
}

wxString OCPNTrackListCtrl::get_speed(long item, double dist, const RoutePoint& point_new,
									  const RoutePoint& point_prev) const
{
	if ((item > 0) && point_new.GetCreateTime().IsValid() && point_prev.GetCreateTime().IsValid()) {
		double speed = 0.0;
		double seconds = point_new.GetCreateTime()
							 .Subtract(point_prev.GetCreateTime())
							 .GetSeconds()
							 .ToDouble();

		if (seconds > 0.0)
			speed = dist / seconds * 3600;

		return wxString::Format(_T("%5.2f"), toUsrSpeed(speed));
	}
	return _("--");
}

wxString OCPNTrackListCtrl::OnGetItemText(long item, long column) const
{
	wxString ret;

	if (item != g_prev_item) {
		if (g_prev_point_index == (item - 1)) {
			if (!g_prev_point_node)
				return wxEmptyString;
			g_prev_point = g_this_point;
			g_this_point_node = g_prev_point_node->GetNext();
			if (g_this_point_node)
				g_this_point = g_this_point_node->GetData();
			else
				g_this_point = NULL;
		} else {
			wxRoutePointListNode* node = m_pRoute->pRoutePointList->GetFirst();
			if (node) {
				if (item > 0) {
					int i = 0;
					while (node && (i < (item - 1))) {
						node = node->GetNext();
						i++;
					}
					g_prev_point_node = node;
					if (!node)
						return wxEmptyString;
					g_prev_point = g_prev_point_node->GetData();

					g_this_point_node = g_prev_point_node->GetNext();
					if (g_this_point_node)
						g_this_point = g_this_point_node->GetData();
					else
						g_this_point = NULL;
				} else {
					g_prev_point_node = NULL;
					g_prev_point = NULL;

					g_this_point_node = node;
					if (g_this_point_node)
						g_this_point = g_this_point_node->GetData();
					else
						g_this_point = NULL;
				}
			} else {
				g_prev_point_node = NULL;
				g_prev_point = NULL;
				g_this_point_node = NULL;
				g_this_point = NULL;
			}
		}

		// Update for next time
		g_prev_point_node = g_this_point_node;
		g_prev_point_index = item;

		g_prev_item = item;
	}

	if (!g_this_point)
		return wxEmptyString;

	switch (column) {
		case 0:
			return leg_id(item);

		case 1:
			double slat;
			double slon;
			if (item == 0) {
				const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
				slat = nav.lat;
				slon = nav.lon;
			} else {
				slat = g_prev_point->m_lat;
				slon = g_prev_point->m_lon;
			}

			geo::DistanceBearingMercator(g_this_point->m_lat, g_this_point->m_lon, slat, slon,
										 &gt_brg, &gt_leg_dist);

			ret.Printf(_T("%6.2f ") + getUsrDistanceUnit(), toUsrDistance(gt_leg_dist));
			break;

		case 2:
			ret.Printf(_T("%03.0f \u00B0T"), gt_brg);
			break;

		case 3:
			return latitude(*g_this_point);

		case 4:
			return longitude(*g_this_point);

		case 5:
			return timestamp(*g_this_point);

		case 6:
			return get_speed(item, gt_leg_dist, *g_this_point, *g_prev_point);

		default:
			break;
	}

	return ret;
}

int OCPNTrackListCtrl::OnGetItemColumnImage(long, long) const
{
	return -1;
}

