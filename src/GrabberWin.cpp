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

#include "GrabberWin.h"
#include <OCPNFloatingToolbarDialog.h>
#include <StyleManager.h>
#include <Style.h>
#include <MainFrame.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>

#include <wx/dcclient.h>

extern MainFrame* gFrame;

BEGIN_EVENT_TABLE(GrabberWin, wxPanel)
	EVT_MOUSE_EVENTS(GrabberWin::MouseEvent)
	EVT_PAINT(GrabberWin::OnPaint)
END_EVENT_TABLE()


GrabberWin::GrabberWin(wxWindow * parent)
	: m_bLeftDown(false)
	, m_bRightDown(false)
{
	m_pbitmap = global::OCPN::get().styleman().current().GetIcon(_T("grabber"));

	Create(parent, -1);

	SetSize(wxSize(m_pbitmap.GetWidth(), m_pbitmap.GetHeight()));
	SetMinSize(wxSize(m_pbitmap.GetWidth(), m_pbitmap.GetHeight()));
}

void GrabberWin::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	dc.DrawBitmap(m_pbitmap, 0, 0, true);
}

void GrabberWin::SetColorScheme(global::ColorScheme)
{
	wxColour back_color = global::OCPN::get().color().get_color(_T("GREY2"));

	SetBackgroundColour(back_color);
	ClearBackground();
	m_pbitmap = global::OCPN::get().styleman().current().GetIcon(_T("grabber"));
}

void GrabberWin::MouseEvent(wxMouseEvent& event)
{
	static wxPoint s_gspt;
	int x;
	int y;

	event.GetPosition(&x, &y);

	wxPoint spt = ClientToScreen(wxPoint(x, y));

#ifdef __WXOSX__
	if (!m_bLeftDown && event.LeftIsDown()) {
		m_bLeftDown = true;
		s_gspt = spt;
		if (!HasCapture())
			CaptureMouse();
	} else if (m_bLeftDown && !event.LeftIsDown()) {
		m_bLeftDown = false;
		if (HasCapture())
			ReleaseMouse();
	}

	if (!m_bRightDown && event.RightIsDown()) {
		m_bRightDown = true;
		if (!HasCapture()) {
			CaptureMouse();
			OCPNFloatingToolbarDialog* pp = wxDynamicCast(GetParent(), OCPNFloatingToolbarDialog);
			pp->ToggleOrientation();
		}
	} else if (m_bRightDown && !event.RightIsDown()) {
		m_bRightDown = false;
		if (HasCapture())
			ReleaseMouse();
	}
#else
	if (event.LeftDown()) {
		s_gspt = spt;
		CaptureMouse();
	}

	if (event.LeftUp()) {
		if (HasCapture())
			ReleaseMouse();
	}

	if (event.RightDown()) {
		OCPNFloatingToolbarDialog* pp = wxDynamicCast(GetParent(), OCPNFloatingToolbarDialog);
		pp->ToggleOrientation();
	}
#endif

	if (event.Dragging()) {
		wxPoint par_pos_old = GetParent()->GetPosition();

		wxPoint par_pos = par_pos_old;
		par_pos.x += spt.x - s_gspt.x;
		par_pos.y += spt.y - s_gspt.y;

		OCPNFloatingToolbarDialog* dp = wxDynamicCast(GetParent(), OCPNFloatingToolbarDialog);
		if (dp)
			dp->MoveDialogInScreenCoords(par_pos, par_pos_old);

		s_gspt = spt;
	}
	gFrame->Raise();
}

