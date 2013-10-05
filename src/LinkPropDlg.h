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

#ifndef _LINKPROPDLG_H_
#define _LINKPROPDLG_H_

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/filesys.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

class LinkPropDialog : public wxDialog
{
	private:
		wxStaticText * m_staticTextLinkDesc;
		wxStaticText * m_staticTextLinkUrl;
		wxButton * m_buttonBrowseLocal;
		wxStdDialogButtonSizer * m_sdbSizerButtons;
		wxButton * m_sdbSizerButtonsOK;
		wxButton * m_sdbSizerButtonsCancel;

	protected:
		virtual void OnLocalFileClick(wxCommandEvent & event);
		virtual void OnCancelClick(wxCommandEvent & event);
		virtual void OnOkClick(wxCommandEvent & event);

	public:
		wxTextCtrl * m_textCtrlLinkDescription;
		wxTextCtrl * m_textCtrlLinkUrl;

		LinkPropDialog(
				wxWindow * parent,
				wxWindowID id = wxID_ANY,
				const wxString & title = _("Link Properties"),
				const wxPoint & pos = wxDefaultPosition,
				const wxSize & size = wxSize( 468,247 ),
				long style = wxDEFAULT_DIALOG_STYLE);

		virtual ~LinkPropDialog();
};

#endif
