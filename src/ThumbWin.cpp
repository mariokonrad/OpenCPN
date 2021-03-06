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

#include "ThumbWin.h"
#include <chart/ChartBase.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>

#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>

#include <wx/dcclient.h>


BEGIN_EVENT_TABLE(ThumbWin, wxWindow)
	EVT_PAINT(ThumbWin::OnPaint)
END_EVENT_TABLE()

ThumbWin::ThumbWin(wxWindow* parent)
	: wxWindow(parent, wxID_ANY, wxPoint(20, 20), wxSize(5, 5), wxSIMPLE_BORDER)
	, pThumbChart(NULL)
{
	m_max_size.x = 100;
	m_max_size.y = 100;
	Show(false);
}

ThumbWin::~ThumbWin()
{
}

void ThumbWin::Resize(void)
{
	if (!pThumbChart)
		return;

	if (pThumbChart->GetThumbData()->pDIBThumb) {
		int newheight = std::min(m_max_size.y, pThumbChart->GetThumbData()->pDIBThumb->GetHeight());
		int newwidth = std::min(m_max_size.x, pThumbChart->GetThumbData()->pDIBThumb->GetWidth());
		SetSize(0, 0, newwidth, newheight);
	}
}

void ThumbWin::SetMaxSize(wxSize const& max_size)
{
	m_max_size = max_size;
}

void ThumbWin::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);

	if (!pThumbChart)
		return;

	if (pThumbChart->GetThumbData()) {
		if (pThumbChart->GetThumbData()->pDIBThumb) {
			dc.DrawBitmap(*(pThumbChart->GetThumbData()->pDIBThumb), 0, 0, false);
		}

		const global::ColorManager& colors = global::OCPN::get().color();

		wxPen ppPen(colors.get_color(_T("CHBLK")), 1, wxSOLID);
		dc.SetPen(ppPen);
		wxBrush yBrush(colors.get_color(_T("CHYLW")), wxSOLID);
		dc.SetBrush(yBrush);
		dc.DrawCircle(pThumbChart->GetThumbData()->ShipX, pThumbChart->GetThumbData()->ShipY, 6);
	}
}

const wxBitmap& ThumbWin::GetBitmap(void)
{
	if (pThumbChart) {
		if (pThumbChart->GetThumbData()) {
			if (pThumbChart->GetThumbData()->pDIBThumb)
				m_bitmap = *(pThumbChart->GetThumbData()->pDIBThumb);
		}
	}

	return m_bitmap;
}

