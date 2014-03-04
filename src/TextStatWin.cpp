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

#include "TextStatWin.h"
#include <PianoWin.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>

#include <chart/ChartDB.h>

BEGIN_EVENT_TABLE(TextStatWin, wxWindow)
	EVT_PAINT(TextStatWin::OnPaint)
	EVT_SIZE(TextStatWin::OnSize)
END_EVENT_TABLE()

TextStatWin::TextStatWin(wxFrame* frame)
	: wxWindow(frame, wxID_ANY, wxPoint(20, 20), wxSize(5, 5), wxSIMPLE_BORDER)
	, bTextSet(false)
{
	SetBackgroundColour(global::OCPN::get().color().get_color(_T("UIBDR")));
	pText = new wxString();
}

TextStatWin::~TextStatWin(void)
{
	delete pText;
}

void TextStatWin::OnSize(wxSizeEvent&)
{
}

void TextStatWin::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	dc.DrawText(*pText, 0, 0);
}

void TextStatWin::TextDraw(const wxString& text)
{
	*pText = text;
	bTextSet = true;
	Refresh(true);
}

