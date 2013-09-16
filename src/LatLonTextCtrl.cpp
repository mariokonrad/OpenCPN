/**************************************************************************
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

#include "LatLonTextCtrl.h"

const wxEventType EVT_LLCHANGE = wxNewEventType();

BEGIN_EVENT_TABLE(LatLonTextCtrl, wxWindow)
	EVT_KILL_FOCUS(LatLonTextCtrl::OnKillFocus)
END_EVENT_TABLE()

LatLonTextCtrl::LatLonTextCtrl(
		wxWindow * parent,
		wxWindowID id,
		const wxString & value,
		const wxPoint & pos,
		const wxSize & size,
		long style,
		const wxValidator& validator,
		const wxString & name)
	: wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
	m_pParentEventHandler = parent->GetEventHandler();
}

void LatLonTextCtrl::OnKillFocus(wxFocusEvent &)
{
	// Send an event to the Parent Dialog
	wxCommandEvent up_event( EVT_LLCHANGE, GetId() );
	up_event.SetEventObject( (wxObject *) this );
	m_pParentEventHandler->AddPendingEvent( up_event );
}

