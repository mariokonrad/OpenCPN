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
	enum {
		UTCINPUT = 0,
		LTINPUT = 1, // i.e. this PC local time
		LMTINPUT = 2, // i.e. the remote location LMT time
		INPUT_FORMAT = 1,
		DISPLAY_FORMAT = 2,
		TIMESTAMP_FORMAT = 3
	};

	OCPNTrackListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
					  long style);
	~OCPNTrackListCtrl();

	wxString OnGetItemText(long item, long column) const;
	int OnGetItemColumnImage(long item, long column) const;

	void set_route(Route* route);
	void set_tz_selection(int);
	void set_lmt_offset(int);

private:
	wxString timestamp2s(wxDateTime ts, int tz_selection, long LMT_offset, int format) const;

	Route* m_pRoute;
	int m_tz_selection;
	int m_LMT_Offset;
};

#endif
