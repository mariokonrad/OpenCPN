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

#ifndef __SENDTOGPSDLG_H__
#define __SENDTOGPSDLG_H__

#include <wx/dialog.h>
#include <wx/gauge.h>

#define ID_STGDIALOG 10005

enum {
	ID_STG_CANCEL = 10000,
	ID_STG_OK,
	ID_STG_CHOICE_COMM
};

class wxComboBox;
class wxButton;
class Route;
class RoutePoint;

/// Route "Send to GPS..." Dialog Definition
class SendToGpsDlg : public wxDialog
{
		DECLARE_DYNAMIC_CLASS(SendToGpsDlg)
		DECLARE_EVENT_TABLE()

	public:
		SendToGpsDlg();
		SendToGpsDlg(
				wxWindow * parent,
				wxWindowID id,
				const wxString & caption,
				const wxString & hint,
				const wxPoint & pos,
				const wxSize & size,
				long style);
		virtual ~SendToGpsDlg();

		bool Create(
				wxWindow * parent,
				wxWindowID id = ID_STGDIALOG,
				const wxString & caption = _("Send Route To GPS"),
				const wxString & hint = _("Send Route To GPS"),
				const wxPoint & pos = wxDefaultPosition,
				const wxSize & size = wxSize(500, 500),
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

		void SetRoute(Route * pRoute);
		void SetWaypoint(RoutePoint * pRoutePoint);

	private:
		void CreateControls(const wxString & hint);

		void OnCancelClick(wxCommandEvent & event);
		void OnSendClick(wxCommandEvent & event);

		Route * m_pRoute;
		RoutePoint * m_pRoutePoint;
		wxComboBox * m_itemCommListBox;
		wxGauge * m_pgauge;
		wxButton * m_CancelButton;
		wxButton * m_SendButton;
};

#endif
