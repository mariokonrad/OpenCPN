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
#include <PositionConvert.h>
#include <Units.h>

#include <geo/GeoRef.h>

enum Column {
	COLUMN_LEG = 0,
	COLUMN_DISTANCE = 1,
	COLUMN_BEARING = 2,
	COLUMN_LATITUDE = 3,
	COLUMN_LONGITUDE = 4,
	COLUMN_TIMESTAMP = 5,
	COLUMN_SPEED = 6
};

OCPNTrackListCtrl::OCPNTrackListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
									 const wxSize& size, long style)
	: wxListCtrl(parent, id, pos, size, style)
	, m_pRoute(NULL)
	, m_tz_selection(LTINPUT)
	, m_LMT_Offset(0)
{
}

OCPNTrackListCtrl::~OCPNTrackListCtrl()
{}

wxString OCPNTrackListCtrl::timestamp2s(wxDateTime ts, int tz_selection, long LMT_offset,
										int format) const
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
		case UTCINPUT:
			s.Append(ts.Format(f));
			if (format != INPUT_FORMAT)
				s.Append(_T(" UT"));
			break;
		case LTINPUT:
			s.Append(ts.FromUTC().Format(f));
			break;
		case LMTINPUT:
			wxTimeSpan lmt(0, 0, (int)LMT_offset, 0);
			s.Append(ts.Add(lmt).Format(f));
			if (format != INPUT_FORMAT)
				s.Append(_T(" LMT"));
			break;
	}
	return s;
}

void OCPNTrackListCtrl::set_route(Route* route)
{
	m_pRoute = route;
}

void OCPNTrackListCtrl::set_tz_selection(int value)
{
	m_tz_selection = value;
}

void OCPNTrackListCtrl::set_lmt_offset(int offset)
{
	m_LMT_Offset = offset;
}

wxString OCPNTrackListCtrl::OnGetItemText(long item, long column) const
{
	if (!m_pRoute)
		return wxEmptyString;
	if (m_pRoute->routepoints().size() == 0)
		return wxEmptyString;
	if (item >= static_cast<long>(m_pRoute->routepoints().size()))
		return wxEmptyString;

	RoutePoint* point = m_pRoute->routepoints().at(item);

	double distance = 0.0;
	double bearing = 0.0;
	double speed = 0.0;

	// calculate distance, bearing and speed between the current point and its predecessor
	if ((item > 0) && (item < static_cast<long>(m_pRoute->routepoints().size()))) {
		RoutePoint* previous = m_pRoute->routepoints().at(item - 1);
		geo::DistanceBearingMercator(point->get_position(), previous->get_position(), &bearing,
									 &distance);
		double dt
			= point->GetCreateTime().Subtract(previous->GetCreateTime()).GetSeconds().ToDouble();
		if (dt > 0.0)
			speed = distance / dt * 3600.0;
	}

	switch (static_cast<Column>(column)) {
		case COLUMN_LEG:
			if (item == 0)
				return _T("--");
			return wxString::Format(_T("%ld"), item);

		case COLUMN_DISTANCE:
			if (item == 0)
				return _T("--");
			return wxString::Format(_T("%6.2f ") + getUsrDistanceUnit(), toUsrDistance(distance));

		case COLUMN_BEARING:
			if (item == 0)
				return _T("--");
			return wxString::Format(_T("%03.0f \u00B0T"), bearing);

		case COLUMN_LATITUDE:
			return PositionConvert::lat(point->latitude(), true);

		case COLUMN_LONGITUDE:
			return PositionConvert::lon(point->longitude(), true);

		case COLUMN_TIMESTAMP:
			if (!point->GetCreateTime().IsValid())
				return _T("----");
			return timestamp2s(point->GetCreateTime(), m_tz_selection, m_LMT_Offset,
							   TIMESTAMP_FORMAT);

		case COLUMN_SPEED:
			if (item == 0)
				return _T("--");
			return wxString::Format(_T("%5.2f"), toUsrSpeed(speed));
	}

	return wxEmptyString;
}

int OCPNTrackListCtrl::OnGetItemColumnImage(long, long) const
{
	return -1;
}

