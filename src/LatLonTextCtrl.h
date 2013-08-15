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

#ifndef __LATLONTEXTCTRL__H__
#define __LATLONTEXTCTRL__H__

#include <wx/textctrl.h>

//    LatLonTextCtrl Specification
//    We need a derived wxText control for lat/lon input in the MarkProp dialog
//    Specifically, we need to catch loss-of-focus events and signal the parent dialog
//    to update the mark's lat/lon dynamically.

extern const wxEventType EVT_LLCHANGE;

class LatLonTextCtrl: public wxTextCtrl
{
		DECLARE_EVENT_TABLE()

	public:
		LatLonTextCtrl(
				wxWindow * parent,
				wxWindowID id,
				const wxString & value = _T(""),
				const wxPoint & pos = wxDefaultPosition,
				const wxSize & size = wxDefaultSize,
				long style = 0,
				const wxValidator & validator = wxDefaultValidator,
				const wxString & name = wxTextCtrlNameStr);

		void OnKillFocus(wxFocusEvent &event);

		wxEvtHandler * m_pParentEventHandler;
};

#endif
