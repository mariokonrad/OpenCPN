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

#ifndef __OCPNTRACKLISTCTL__H__
#define __OCPNTRACKLISTCTL__H__

#include <wx/window.h>
#include <wx/listctrl.h>

class Route;
class RoutePoint;

class OCPNTrackListCtrl : public wxListCtrl
{
public:
	OCPNTrackListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
					  long style);
	~OCPNTrackListCtrl();

	wxString OnGetItemText(long item, long column) const;
	int OnGetItemColumnImage(long item, long column) const;

	void set_route(Route* route);

private:
	wxString leg_id(long item) const;
	wxString latitude(const RoutePoint& point) const;
	wxString longitude(const RoutePoint& point) const;
	wxString timestamp(const RoutePoint& point) const;
	wxString get_speed(long item, double dist, const RoutePoint& point_new,
					   const RoutePoint& point_prev) const;

	Route* m_pRoute;
	int m_tz_selection;
	int m_LMT_Offset;
};

#endif
