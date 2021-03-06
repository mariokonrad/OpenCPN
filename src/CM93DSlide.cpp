/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "CM93DSlide.h"
#include <ChartCanvas.h>

#include <global/OCPN.h>
#include <global/GUI.h>

#include <wx/slider.h>

#define ID_CM93ZOOMG 102

extern CM93DSlide* pCM93DetailSlider;
extern ChartCanvas* cc1;

BEGIN_EVENT_TABLE(CM93DSlide, wxDialog)
	EVT_MOVE(CM93DSlide::OnMove)
	EVT_COMMAND_SCROLL_THUMBRELEASE(-1, CM93DSlide::OnChangeValue)
	EVT_COMMAND_SCROLL_LINEUP(-1, CM93DSlide::OnChangeValue)
	EVT_COMMAND_SCROLL_LINEDOWN(-1, CM93DSlide::OnChangeValue)
	EVT_COMMAND_SCROLL_PAGEUP(-1, CM93DSlide::OnChangeValue)
	EVT_COMMAND_SCROLL_PAGEDOWN(-1, CM93DSlide::OnChangeValue)
	EVT_COMMAND_SCROLL_BOTTOM(-1, CM93DSlide::OnChangeValue)
	EVT_COMMAND_SCROLL_TOP(-1, CM93DSlide::OnChangeValue)
	EVT_CLOSE(CM93DSlide::OnClose)
END_EVENT_TABLE()

CM93DSlide::CM93DSlide(
		wxWindow* parent,
		wxWindowID id,
		int value,
		int minValue,
		int maxValue,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxString& title)
{
	Init();
	Create(parent, ID_CM93ZOOMG, value, minValue, maxValue, pos, size, style, title);
}

CM93DSlide::~CM93DSlide()
{
	delete m_pCM93DetailSlider;
}

void CM93DSlide::Init(void)
{
	m_pCM93DetailSlider = NULL;
}

bool CM93DSlide::Create(
		wxWindow* parent,
		wxWindowID id,
		int value,
		int minValue,
		int maxValue,
		const wxPoint& pos,
		const wxSize& size,
		long,
		const wxString& title)
{
	long wstyle = wxDEFAULT_DIALOG_STYLE;
#ifdef __WXOSX__
	wstyle |= wxSTAY_ON_TOP;
#endif

	if (!wxDialog::Create(parent, id, title, pos, size, wstyle))
		return false;

	m_pparent = parent;

	m_pCM93DetailSlider
		= new wxSlider(this, id, value, minValue, maxValue, wxPoint(0, 0), wxDefaultSize,
					   wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS, wxDefaultValidator, title);

	m_pCM93DetailSlider->SetSize(wxSize(200, -1));

	m_pCM93DetailSlider->InvalidateBestSize();
	wxSize bs = m_pCM93DetailSlider->GetBestSize();

	m_pCM93DetailSlider->SetSize(wxSize(200, bs.y));
	Fit();

	m_pCM93DetailSlider->SetValue(global::OCPN::get().gui().cm93().zoom_factor);

	Hide();

	return true;
}

void CM93DSlide::OnCancelClick(wxCommandEvent &)
{
	global::OCPN::get().gui().set_cm93_show_detail_slider(false);
	Close();
}

void CM93DSlide::OnClose(wxCloseEvent &)
{
	global::OCPN::get().gui().set_cm93_show_detail_slider(false);
	Destroy();
	pCM93DetailSlider = NULL;
}

void CM93DSlide::OnMove(wxMoveEvent & event)
{
	global::OCPN::get().gui().set_cm93_detail_dialog_position(event.GetPosition());
	event.Skip();
}

void CM93DSlide::OnChangeValue(wxScrollEvent &)
{
	global::OCPN::get().gui().set_cm93_zoom_factor(m_pCM93DetailSlider->GetValue());

	::wxBeginBusyCursor();

	cc1->ReloadVP();
	cc1->Refresh(false);

	::wxEndBusyCursor();
}

