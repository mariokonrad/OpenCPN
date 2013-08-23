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

#ifndef __ROUTEPRINTSELECTION__H__
#define __ROUTEPRINTSELECTION__H__

#include <wx/dialog.h>

#include "Route.h"
#include "ColorScheme.h"

#define ID_ROUTEPRINTSELECTION 9000
#define ID_ROUTEPRINT_SELECTION_OK 9001
#define ID_ROUTEPRINT_SELECTION_CANCEL 9002

class wxButton;
class wxCheckBox;

class RoutePrintSelection : public wxDialog
{
		DECLARE_DYNAMIC_CLASS(RoutePrintSelection)
		DECLARE_EVENT_TABLE()

	public:
		RoutePrintSelection();

		RoutePrintSelection(
				wxWindow * parent,
				Route * route,
				wxWindowID id = ID_ROUTEPRINTSELECTION,
				const wxString & caption = _("Print Route Selection"),
				const wxPoint & pos = wxDefaultPosition,
				const wxSize & size = wxSize(750, 300),
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

		virtual ~RoutePrintSelection();

		bool Create(
				wxWindow * parent,
				wxWindowID id = ID_ROUTEPRINTSELECTION,
				const wxString & caption = _("Print Route Selection"),
				const wxPoint & pos = wxDefaultPosition,
				const wxSize & size = wxSize(750, 300),
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

		void CreateControls();

		void SetColorScheme(ColorScheme cs);
		void SetDialogTitle(const wxString & title);
		void OnRoutepropCancelClick(wxCommandEvent& event);
		void OnRoutepropOkClick(wxCommandEvent& event);

		static bool ShowToolTips();

		wxButton * m_CancelButton;
		wxButton * m_OKButton;
		wxCheckBox * m_checkBoxWPName;
		wxCheckBox * m_checkBoxWPPosition;
		wxCheckBox * m_checkBoxWPCourse;
		wxCheckBox * m_checkBoxWPDistanceToNext;
		wxCheckBox * m_checkBoxWPDescription;
		Route * route;
};

#endif
