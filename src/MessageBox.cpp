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

#include "MessageBox.h"
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/bitmap.h>
#include <wx/statbmp.h>
#include <wx/artprov.h>
#include <wx/timer.h>

class OCPNMessageDialog : public wxDialog
{
	DECLARE_EVENT_TABLE()

public:
	OCPNMessageDialog(wxWindow* parent, const wxString& message,
					  const wxString& caption = wxMessageBoxCaptionStr,
					  long style = wxOK | wxCENTRE, const wxPoint& pos = wxDefaultPosition);

	void OnYes(wxCommandEvent& event);
	void OnNo(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);

private:
	int m_style;
};

BEGIN_EVENT_TABLE(OCPNMessageDialog, wxDialog)
	EVT_BUTTON(wxID_YES, OCPNMessageDialog::OnYes)
	EVT_BUTTON(wxID_NO, OCPNMessageDialog::OnNo)
	EVT_BUTTON(wxID_CANCEL, OCPNMessageDialog::OnCancel)
	EVT_CLOSE(OCPNMessageDialog::OnClose)
END_EVENT_TABLE()

OCPNMessageDialog::OCPNMessageDialog(wxWindow* parent, const wxString& message,
									 const wxString& caption, long style, const wxPoint& pos)
	: wxDialog(parent, wxID_ANY, caption, pos, wxDefaultSize,
			   wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP)
{
	m_style = style;
	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* icon_text = new wxBoxSizer(wxHORIZONTAL);

#if wxUSE_STATBMP
	// 1) icon
	if (style & wxICON_MASK) {
		wxBitmap bitmap;
		switch (style & wxICON_MASK) {
			default:
				wxFAIL_MSG(_T("incorrect log style"));
			// fall through

			case wxICON_ERROR:
				bitmap = wxArtProvider::GetIcon(wxART_ERROR, wxART_MESSAGE_BOX);
				break;

			case wxICON_INFORMATION:
				bitmap = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_MESSAGE_BOX);
				break;

			case wxICON_WARNING:
				bitmap = wxArtProvider::GetIcon(wxART_WARNING, wxART_MESSAGE_BOX);
				break;

			case wxICON_QUESTION:
				bitmap = wxArtProvider::GetIcon(wxART_QUESTION, wxART_MESSAGE_BOX);
				break;
		}
		wxStaticBitmap* icon = new wxStaticBitmap(this, wxID_ANY, bitmap);
		icon_text->Add(icon, 0, wxCENTER);
	}
#endif // wxUSE_STATBMP

#if wxUSE_STATTEXT
	// 2) text
	icon_text->Add(CreateTextSizer(message), 0, wxALIGN_CENTER | wxLEFT, 10);

	topsizer->Add(icon_text, 1, wxCENTER | wxLEFT | wxRIGHT | wxTOP, 10);
#endif // wxUSE_STATTEXT

	// 3) buttons
    int AllButtonSizerFlags = wxOK|wxCANCEL|wxYES|wxNO|wxHELP|wxNO_DEFAULT;
	int center_flag = wxEXPAND;
	if (style & wxYES_NO)
		center_flag = wxALIGN_CENTRE;
	wxSizer* sizerBtn = CreateSeparatedButtonSizer(style & AllButtonSizerFlags);
	if (sizerBtn)
		topsizer->Add(sizerBtn, 0, center_flag | wxALL, 10);

	SetAutoLayout(true);
	SetSizer(topsizer);

	topsizer->SetSizeHints(this);
	topsizer->Fit(this);
	wxSize size(GetSize());
	if (size.x < size.y * 3 / 2) {
		size.x = size.y * 3 / 2;
		SetSize(size);
	}

	Centre(wxBOTH | wxCENTER_FRAME);
}

void OCPNMessageDialog::OnYes(wxCommandEvent& WXUNUSED(event))
{
	EndModal(wxID_YES);
}

void OCPNMessageDialog::OnNo(wxCommandEvent& WXUNUSED(event))
{
	EndModal(wxID_NO);
}

void OCPNMessageDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
	// Allow cancellation via ESC/Close button except if
	// only YES and NO are specified.
	if ((m_style & wxYES_NO) != wxYES_NO || (m_style & wxCANCEL)) {
		EndModal(wxID_CANCEL);
	}
}

void OCPNMessageDialog::OnClose(wxCloseEvent& event)
{
	EndModal(wxID_CANCEL);
}

class TimedMessageBox : wxEvtHandler
{
public:
	TimedMessageBox(wxWindow* parent, const wxString& message,
					const wxString& caption = _T("Message box"), long style = wxOK | wxCANCEL,
					int timeout_sec = -1, const wxPoint& pos = wxDefaultPosition);
	virtual ~TimedMessageBox();

	int GetRetVal(void) const
	{
		return ret_val;
	}

	void OnTimer(wxTimerEvent& evt);

	wxTimer m_timer;
	OCPNMessageDialog* dlg;
	int ret_val;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(TimedMessageBox, wxEvtHandler)
	EVT_TIMER(-1, TimedMessageBox::OnTimer)
END_EVENT_TABLE()

TimedMessageBox::TimedMessageBox(wxWindow* parent, const wxString& message, const wxString& caption,
								 long style, int timeout_sec, const wxPoint& pos)
{
	ret_val = 0;
	m_timer.SetOwner(this, -1);

	if (timeout_sec > 0)
		m_timer.Start(timeout_sec * 1000, wxTIMER_ONE_SHOT);

	dlg = new OCPNMessageDialog(parent, message, caption, style, pos);
	int ret = dlg->ShowModal();

	delete dlg;
	dlg = NULL;

	ret_val = ret;
}

TimedMessageBox::~TimedMessageBox()
{
}

void TimedMessageBox::OnTimer(wxTimerEvent&)
{
	if (dlg)
		dlg->EndModal(wxID_CANCEL);
}

int OCPNMessageBox(
		wxWindow * parent,
		const wxString & message,
		const wxString & caption,
		int style,
		int timeout_sec,
		int x,
		int y )
{
#ifdef __WXOSX__
	long parent_style = 0;
	if (g_FloatingToolbarDialog)
		g_FloatingToolbarDialog->Hide();

	if (g_FloatingCompassDialog)
		g_FloatingCompassDialog->Hide();

	if (stats)
		stats->Hide();

	if (parent) {
		parent_style = parent->GetWindowStyle();
		parent->SetWindowStyle(parent_style & !wxSTAY_ON_TOP);
	}
#endif

	TimedMessageBox tbox(parent, message, caption, style, timeout_sec, wxPoint(x, y));
	int ret = tbox.GetRetVal();

#ifdef __WXOSX__
	if (gFrame)
		gFrame->SurfaceToolbar();

	if (g_FloatingCompassDialog)
		g_FloatingCompassDialog->Show();

	if (stats)
		stats->Show();

	if (parent) {
		parent->Raise();
		parent->SetWindowStyle(parent_style);
	}
#endif

	return ret;
}

