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

#ifndef __PLUGIN__OCPN_MSGEVENT__H__
#define __PLUGIN__OCPN_MSGEVENT__H__

#include <wx/event.h>

class OCPN_MsgEvent : public wxEvent
{
	public:
		OCPN_MsgEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
		OCPN_MsgEvent(const OCPN_MsgEvent & event);
		virtual ~OCPN_MsgEvent();

		wxString GetID();
		wxString GetJSONText();
		void SetID(const wxString &string);
		void SetJSONText(const wxString &string);

		// required for sending with wxPostEvent()
		wxEvent *Clone() const;

	private:
		wxString m_MessageID;
		wxString m_MessageText;
};

#endif
