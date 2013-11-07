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

#include "SentenceListDlg.h"
#include "nmea0183.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

extern wxString StringArrayToString(wxArrayString arr); // FIXME: cleanup from options

SentenceListDlg::SentenceListDlg(
		ConnectionParams::FilterDirection dir,
		wxWindow * parent,
		wxWindowID id,
		const wxString & title,
		const wxPoint & pos,
		const wxSize & size,
		long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	m_dir = dir;
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );

	NMEA0183 nmea;
	standard_sentences = nmea.GetRecognizedArray();
	if(m_dir == ConnectionParams::FILTER_OUTPUT) {
		standard_sentences.Add(_T("ECRMB"));
		standard_sentences.Add(_T("ECRMC"));
		standard_sentences.Add(_T("ECAPB"));
	}

	standard_sentences.Add(_T("AIVDM"));
	standard_sentences.Add(_T("AIVDO"));
	standard_sentences.Add(_T("FRPOS"));
	standard_sentences.Add(_T("CD"));

	m_pclbBox = new wxStaticBox( this,  wxID_ANY, _T("")) ;

	wxStaticBoxSizer* sbSizerclb;
	sbSizerclb = new wxStaticBoxSizer( m_pclbBox , wxVERTICAL );
	bSizer17->Add( sbSizerclb, 1, wxALL|wxEXPAND, 5 );

	m_clbSentences = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, standard_sentences );


	sbSizerclb->Add( m_clbSentences, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );

	m_btnCheckAll = new wxButton( this, wxID_ANY, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_btnCheckAll, 0, wxALL, 5 );

	m_btnClearAll = new wxButton( this, wxID_ANY, _("Clear All"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_btnClearAll, 0, wxALL, 5 );

	bSizer18->AddSpacer(1);

	m_btnAdd = new wxButton( this, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_btnAdd, 0, wxALL, 5 );

	m_btnDel = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnDel->Enable( false );

	bSizer18->Add( m_btnDel, 0, wxALL, 5 );


	bSizer17->Add( bSizer18, 0, wxALL|wxEXPAND, 5 );


	bSizer16->Add( bSizer17, 1, wxEXPAND, 5 );

	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( this, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
	m_sdbSizer4->Realize();

	bSizer16->Add( m_sdbSizer4, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizer16 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	m_btnAdd->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnAddClick ), NULL, this );
	m_btnDel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnDeleteClick ), NULL, this );
	m_sdbSizer4Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCancelClick ), NULL, this );
	m_sdbSizer4OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnOkClick ), NULL, this );
	m_clbSentences->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( SentenceListDlg::OnCLBToggle ), NULL, this );
	m_clbSentences->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( SentenceListDlg::OnCLBSelect ), NULL, this );
	m_btnCheckAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCheckAllClick ), NULL, this );
	m_btnClearAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnClearAllClick ), NULL, this );

}

SentenceListDlg::~SentenceListDlg()
{
	// Disconnect Events
	m_btnAdd->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnAddClick ), NULL, this );
	m_btnDel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnDeleteClick ), NULL, this );
	m_sdbSizer4Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCancelClick ), NULL, this );
	m_sdbSizer4OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnOkClick ), NULL, this );
	m_clbSentences->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( SentenceListDlg::OnCLBToggle ), NULL, this );
	m_clbSentences->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( SentenceListDlg::OnCLBSelect ), NULL, this );
	m_btnCheckAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCheckAllClick ), NULL, this );
	m_btnClearAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnClearAllClick ), NULL, this );
}

void SentenceListDlg::SetSentenceList(wxArrayString sentences)
{
	m_sentences = sentences;

	if ((m_sentences.Count() == 0) && (m_type == ConnectionParams::WHITELIST)) {
		for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
			m_clbSentences->Check(i, true);
	}
	if ((m_sentences.Count() == 0) && (m_type == ConnectionParams::BLACKLIST)) {
		for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
			m_clbSentences->Check(i, false);
	}

	FillSentences();
}

wxString SentenceListDlg::GetSentencesAsText()
{
	return StringArrayToString(m_sentences);
}

void SentenceListDlg::BuildSentenceArray()
{
	m_sentences.Clear();
	wxString s;
	for (size_t i = 0; i < m_clbSentences->GetCount(); i++) {
		if (m_clbSentences->IsChecked(i))
			m_sentences.Add(m_clbSentences->GetString(i));
	}
}

void SentenceListDlg::FillSentences()
{
	if (m_sentences.Count() == 0)
		return;

	for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
		m_clbSentences->Check(i, false);

	for (size_t i = 0; i < m_sentences.Count(); i++) {
		int item = m_clbSentences->FindString(m_sentences[i]);
		if (wxNOT_FOUND != item)
			m_clbSentences->Check(item);
		else {
			m_clbSentences->Append(m_sentences[i]);
			int item = m_clbSentences->FindString(m_sentences[i]);
			m_clbSentences->Check(item);
		}
	}

	m_btnDel->Enable(false);
}

void SentenceListDlg::OnStcSelect(wxCommandEvent&)
{
	m_btnDel->Enable();
}

void SentenceListDlg::OnCLBSelect(wxCommandEvent&)
{
	// Only active the "Delete" button if the selection is not in the standard list
	int isel = m_clbSentences->GetSelection();
	bool bdelete = true;
	if (isel >= 0) {
		wxString s = m_clbSentences->GetString(isel);
		for (size_t i = 0; i < standard_sentences.Count(); i++) {
			if (standard_sentences[i] == s) {
				bdelete = false;
				break;
			}
		}
	} else
		bdelete = false;
	m_btnDel->Enable(bdelete);
}

void SentenceListDlg::OnCLBToggle(wxCommandEvent&)
{
	BuildSentenceArray();
}

void SentenceListDlg::OnAddClick(wxCommandEvent&)
{
	wxString stc = wxGetTextFromUser(_("Enter the NMEA sentence (2, 3 or 5 characters)"),
									 _("Enter the NMEA sentence"));
	if (stc.Length() == 2 || stc.Length() == 3 || stc.Length() == 5) {
		m_sentences.Add(stc);
		m_clbSentences->Append(stc);
		int item = m_clbSentences->FindString(stc);
		m_clbSentences->Check(item);
	} else
		wxMessageBox(_("An NMEA sentence is generally 3 characters long (like RMC, GGA etc.) It can also have a two letter prefix identifying the source, or TALKER, of the message (The whole sentences then looks like GPGGA or AITXT). You may filter out all the sentences with certain TALKER prefix (like GP, AI etc.). The filter accepts just these three formats."), _("Wrong length of the NMEA filter value"));
}

void SentenceListDlg::OnDeleteClick(wxCommandEvent&)
{
	BuildSentenceArray();

	// One can only delete items that do not appear in the standard sentence list
	int isel = m_clbSentences->GetSelection();
	wxString s = m_clbSentences->GetString(isel);
	bool bdelete = true;
	for (size_t i = 0; i < standard_sentences.Count(); i++) {
		if (standard_sentences[i] == s) {
			bdelete = false;
			break;
		}
	}

	if (bdelete) {
		m_sentences.Remove(s);
		m_clbSentences->Delete(isel);
	}

	FillSentences();
}

void SentenceListDlg::OnClearAllClick(wxCommandEvent&)
{
	for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
		m_clbSentences->Check(i, false);

	BuildSentenceArray();
}

void SentenceListDlg::OnCheckAllClick(wxCommandEvent&)
{
	for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
		m_clbSentences->Check(i, true);

	BuildSentenceArray();
}

void SentenceListDlg::SetType(int io, ConnectionParams::ListType type)
{
	m_type = type;

	switch (io) {
		case 0: // input
		default:
			if (type == ConnectionParams::WHITELIST)
				m_pclbBox->SetLabel(_("Accept Sentences"));
			else
				m_pclbBox->SetLabel(_("Ignore Sentences"));
			break;
		case 1: // output
			if (type == ConnectionParams::WHITELIST)
				m_pclbBox->SetLabel(_("Transmit Sentences"));
			else
				m_pclbBox->SetLabel(_("Drop Sentences"));
			break;
	}
	Refresh();
}

void SentenceListDlg::OnCancelClick(wxCommandEvent& event)
{
	event.Skip();
}

void SentenceListDlg::OnOkClick(wxCommandEvent& event)
{
	event.Skip();
}

