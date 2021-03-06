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

#include "ChInfoWin.h"

#include <global/OCPN.h>
#include <global/ColorManager.h>

#include <wx/dcclient.h>

BEGIN_EVENT_TABLE(ChInfoWin, wxWindow)
	EVT_PAINT(ChInfoWin::OnPaint)
	EVT_ERASE_BACKGROUND(ChInfoWin::OnEraseBackground)
END_EVENT_TABLE()

ChInfoWin::ChInfoWin(wxWindow* parent)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	int ststyle = wxALIGN_LEFT | wxST_NO_AUTORESIZE;
	m_pInfoTextCtl
		= new wxStaticText(this, -1, _T ( "" ), wxDefaultPosition, wxDefaultSize, ststyle);
	Hide();
}

ChInfoWin::~ChInfoWin()
{
	delete m_pInfoTextCtl;
}

void ChInfoWin::OnEraseBackground(wxEraseEvent&)
{
}

void ChInfoWin::OnPaint(wxPaintEvent&)
{
	int width;
	int height;
	GetClientSize(&width, &height);
	wxPaintDC dc(this);

	const global::ColorManager& colors = global::OCPN::get().color();

	dc.SetBrush(wxBrush(colors.get_color(_T("UIBCK"))));
	dc.SetPen(wxPen(colors.get_color(_T("UITX1"))));
	dc.DrawRectangle(0, 0, width, height);
}

void ChInfoWin::SetBitmap()
{
	const global::ColorManager& colors = global::OCPN::get().color();

	SetBackgroundColour(colors.get_color(_T("UIBCK")));

	m_pInfoTextCtl->SetBackgroundColour(colors.get_color(_T("UIBCK")));
	m_pInfoTextCtl->SetForegroundColour(colors.get_color(_T("UITX1")));

	m_pInfoTextCtl->SetSize(1, 1, m_size.x - 2, m_size.y - 2);
	m_pInfoTextCtl->SetLabel(m_string);

	SetSize(m_position.x, m_position.y, m_size.x, m_size.y);
}

void ChInfoWin::FitToChars(int char_width, int char_height)
{
	wxSize size;

	int adjust = 1;
#ifdef __WXOSX__
	adjust = 2;
#endif
	size.x = GetCharWidth() * char_width;
	size.y = GetCharHeight() * (char_height + adjust);
	SetWinSize(size);
}

wxSize ChInfoWin::GetWinSize(void) const
{
	return m_size;
}

void ChInfoWin::SetString(const wxString& s)
{
	m_string = s;
}

void ChInfoWin::SetPosition(wxPoint pt)
{
	m_position = pt;
}

void ChInfoWin::SetWinSize(wxSize sz)
{
	m_size = sz;
}

