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

#include "StatWin.h"
#include <dychart.h>
#include <PianoWin.h>
#include <TextStatWin.h>

#include <gui/StyleManager.h>
#include <gui/Style.h>

#include <chart/ChartDB.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>

extern chart::ChartDB* ChartData;

BEGIN_EVENT_TABLE(StatWin, wxDialog)
	EVT_PAINT(StatWin::OnPaint)
	EVT_SIZE(StatWin::OnSize)
END_EVENT_TABLE()

StatWin::StatWin(wxWindow* win)
{
	long wstyle = wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
	wstyle |= wxFRAME_SHAPED;
#endif
#ifdef __WXMAC__
	wstyle |= wxSTAY_ON_TOP;
#endif

	wxDialog::Create(win, wxID_ANY, _T(""), wxPoint(20, 20), wxSize(5, 5), wstyle);

	int x;
	int y;
	GetClientSize(&x, &y);

	const global::ColorManager& colors = global::OCPN::get().color();
	m_backBrush = wxBrush(colors.get_color(_T("UIBDR")), wxSOLID);
	SetBackgroundColour(colors.get_color(_T("UIBDR")));

	// on WXMSW, this prevents flashing on color scheme change
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	m_rows = 1;

	// Create the Children
	pPiano = new PianoWin((wxFrame*)this);
}

StatWin::~StatWin()
{
	pPiano->Close();
}

void StatWin::RePosition()
{
	wxPoint position;
	position.x = 0;
	position.y = GetParent()->GetClientSize().y - GetSize().y;

	wxPoint screen_pos = GetParent()->ClientToScreen(position);
	Move(screen_pos);
}

void StatWin::ReSize()
{
	wxSize new_size;
	new_size.x = GetParent()->GetClientSize().x;
	new_size.y = 22 * GetRows();
	SetSize(new_size);
}

void StatWin::OnPaint(wxPaintEvent &)
{
	if (global::OCPN::get().styleman().current().isChartStatusWindowTransparent())
		return;

	wxPaintDC dc(this);
	dc.SetBackground(m_backBrush);
	dc.Clear();
}

void StatWin::OnSize(wxSizeEvent&)
{
	int width;
	int height;
	GetClientSize(&width, &height);

	if (width) {
		pPiano->SetSize(0, 0, width * 6 / 10, height * 1 / m_rows);
		pPiano->FormatKeys();
	}
}

void StatWin::FormatStat(void)
{
	pPiano->FormatKeys();
}

int StatWin::GetFontHeight()
{
	wxCoord w;
	wxCoord h;
	GetTextExtent(_T("TEST"), &w, &h);
	return h;
}

void StatWin::SetColorScheme(global::ColorScheme cs)
{
	m_backBrush = wxBrush(global::OCPN::get().color().get_color(_T("UIBDR")), wxSOLID);

	// Also apply color scheme to all known children
	pPiano->SetColorScheme(cs);

	Refresh();
}

int StatWin::GetRows() const
{
	return m_rows;
}

