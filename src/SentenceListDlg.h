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

#ifndef __SENTENCELISTDIALOG__H__
#define __SENTENCELISTDIALOG__H__

#include <wx/dialog.h>
#include <wx/string.h>
#include "ConnectionParams.h"

class wxButton;
class wxCheckListBox;
class wxStaticBox;

class SentenceListDlg : public wxDialog
{
	private:
		wxArrayString m_sentences;
		void FillSentences();
		ConnectionParams::ListType m_type;
		ConnectionParams::FilterDirection m_dir;

	protected:
		wxCheckListBox * m_clbSentences;
		wxButton * m_btnAdd;
		wxButton * m_btnDel;
		wxButton * m_btnCheckAll;
		wxButton * m_btnClearAll;
		wxStdDialogButtonSizer* m_sdbSizer4;
		wxButton * m_sdbSizer4OK;
		wxButton * m_sdbSizer4Cancel;
		wxArrayString standard_sentences;
		wxStaticBox * m_pclbBox;

		// Virtual event handlers, overide them in your derived class
		void OnStcSelect( wxCommandEvent& event );
		void OnAddClick( wxCommandEvent& event );
		void OnDeleteClick( wxCommandEvent& event );
		void OnCancelClick( wxCommandEvent& event );
		void OnOkClick( wxCommandEvent& event );
		void OnCLBSelect( wxCommandEvent& event );
		void OnCLBToggle( wxCommandEvent& event );
		void OnCheckAllClick( wxCommandEvent& event );
		void OnClearAllClick( wxCommandEvent& event );

	public:
		SentenceListDlg(
				ConnectionParams::FilterDirection dir,
				wxWindow * parent,
				wxWindowID id = wxID_ANY,
				const wxString & title = _("Sentence Filter"),
				const wxPoint & pos = wxDefaultPosition,
				const wxSize & size = wxSize( 280,420 ),
				long style = wxDEFAULT_DIALOG_STYLE );
		~SentenceListDlg();
		void SetSentenceList(wxArrayString sentences);
		wxString GetSentencesAsText();
		void BuildSentenceArray();
		void SetType(int io, ConnectionParams::ListType type);
};

#endif
