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

#ifndef __AIS__AISTARGETQUERYDIALOG__H__
#define __AIS__AISTARGETQUERYDIALOG__H__

#include <wx/dialog.h>
#include <global/ColorScheme.h>

class wxHtmlWindow;
class wxBoxSizer;
class wxButton;

namespace ais
{

class AISTargetQueryDialog : public wxDialog
{
	DECLARE_CLASS(AISTargetQueryDialog)
	DECLARE_EVENT_TABLE()

public:
	AISTargetQueryDialog();
	AISTargetQueryDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
						 const wxString& caption = _("Object Query"),
						 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
						 long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	virtual ~AISTargetQueryDialog();

	bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
				const wxString& caption = _("Object Query"), const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	void OnClose(wxCloseEvent& event);
	void OnIdOKClick(wxCommandEvent& event);
	void OnIdWptCreateClick(wxCommandEvent& event);
	void OnMove(wxMoveEvent& event);

	void UpdateText(void);
	void SetMMSI(int mmsi);
	int GetMMSI(void) const;

private:
	void CreateControls();
	void SetColorScheme(global::ColorScheme cs);

	int m_MMSI;
	wxHtmlWindow* m_pQueryTextCtl;
	wxBoxSizer* m_pboxSizer;
	wxButton* m_okButton;
};

}

#endif
