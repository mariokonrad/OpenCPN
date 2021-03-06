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

#ifndef __AIS__AISTARGETALERTDIALOG__H__
#define __AIS__AISTARGETALERTDIALOG__H__

#include <OCPN_AlertDialog.h>

class wxHtmlWindow;

namespace ais
{

class AIS_Decoder;

class AISTargetAlertDialog : public OCPN_AlertDialog
{
	DECLARE_CLASS(AISTargetAlertDialog)
	DECLARE_EVENT_TABLE()

public:
	AISTargetAlertDialog();
	~AISTargetAlertDialog();
	bool Create(int target_mmsi, wxWindow* parent, AIS_Decoder* pdecoder, bool b_jumpto,
				wxWindowID id = wxID_ANY, const wxString& caption = _("OpenCPN AIS Alert"),
				const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
	void Init();
	int Get_Dialog_MMSI(void) const;
	void UpdateText();

private:
	void CreateControls();
	bool GetAlertText(void);
	void OnClose(wxCloseEvent& event);
	void OnIdAckClick(wxCommandEvent& event);
	void OnMove(wxMoveEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnIdSilenceClick(wxCommandEvent& event);
	void OnIdJumptoClick(wxCommandEvent& event);

	wxHtmlWindow* m_pAlertTextCtl;
	int m_target_mmsi;
	AIS_Decoder* m_pdecoder;
	wxFont* m_pFont;
	wxString m_alert_text;
	bool m_bjumpto;
};

}

#endif
