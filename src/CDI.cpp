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

#include "CDI.h"
#include <Routeman.h>

#include <global/OCPN.h>
#include <global/Navigation.h>
#include <global/ColorManager.h>

#include <cmath>

#include <wx/dcmemory.h>

extern Routeman* g_pRouteMan;

BEGIN_EVENT_TABLE(CDI, wxWindow)
	EVT_PAINT(CDI::OnPaint)
END_EVENT_TABLE()

CDI::CDI(
		wxWindow* parent,
		wxWindowID id,
		long style,
		const wxString& name)
	: wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, style, name)
{
	SetMinSize(wxSize(10, 150));
}

void CDI::SetColorScheme(global::ColorScheme)
{
	const global::ColorManager& colors = global::OCPN::get().color();

	m_pbackBrush = wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("DILG2")), wxSOLID);
	m_proadBrush = wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("DILG1")), wxSOLID);
	m_proadPen = wxThePenList->FindOrCreatePen(colors.get_color(_T("CHBLK")), 1, wxSOLID);
}

void CDI::OnPaint(wxPaintEvent&)
{
	int sx;
	int sy;
	GetClientSize(&sx, &sy);

	// Do the drawing on an off-screen memory DC, and blit into place
	// to avoid objectionable flashing
	wxMemoryDC mdc;

	wxBitmap m_bitmap(sx, sy, -1);
	mdc.SelectObject(m_bitmap);
	mdc.SetBackground(*m_pbackBrush);
	mdc.Clear();

	int xp = sx / 2;
	int yp = sy * 9 / 10;

	int path_length = sy * 3;
	int pix_per_xte = 120;

	if (g_pRouteMan->GetpActiveRoute()) {
		double angle = 90 - (g_pRouteMan->GetCurrentSegmentCourse()
							 - global::OCPN::get().nav().get_data().cog);

		double dy = path_length * sin(angle * M_PI / 180.0);
		double dx = path_length * cos(angle * M_PI / 180.0);

		int xtedir;
		xtedir = g_pRouteMan->GetXTEDir();
		double xte = g_pRouteMan->GetCurrentXTEToActivePoint();

		double ddy = xtedir * pix_per_xte * xte * sin((90 - angle) * M_PI / 180.0);
		double ddx = xtedir * pix_per_xte * xte * cos((90 - angle) * M_PI / 180.0);

		int ddxi = (int)ddx;
		int ddyi = (int)ddy;

		int xc1 = xp - (int)(dx / 2) + ddxi;
		int yc1 = yp + (int)(dy / 2) + ddyi;
		int xc2 = xp + (int)(dx / 2) + ddxi;
		int yc2 = yp - (int)(dy / 2) + ddyi;

		wxPoint road[4];

		int road_top_width = 10;
		int road_bot_width = 40;

		road[0].x = xc1 - (int)(road_bot_width * cos((90 - angle) * M_PI / 180.0));
		road[0].y = yc1 - (int)(road_bot_width * sin((90 - angle) * M_PI / 180.0));

		road[1].x = xc2 - (int)(road_top_width * cos((90 - angle) * M_PI / 180.0));
		road[1].y = yc2 - (int)(road_top_width * sin((90 - angle) * M_PI / 180.0));

		road[2].x = xc2 + (int)(road_top_width * cos((90 - angle) * M_PI / 180.0));
		road[2].y = yc2 + (int)(road_top_width * sin((90 - angle) * M_PI / 180.0));

		road[3].x = xc1 + (int)(road_bot_width * cos((90 - angle) * M_PI / 180.0));
		road[3].y = yc1 + (int)(road_bot_width * sin((90 - angle) * M_PI / 180.0));

		mdc.SetBrush(*m_proadBrush);
		mdc.SetPen(*m_proadPen);
		mdc.DrawPolygon(4, road, 0, 0, wxODDEVEN_RULE);

		mdc.DrawLine(xc1, yc1, xc2, yc2);

		mdc.DrawLine(0, yp, sx, yp);
		mdc.DrawCircle(xp, yp, 6);
		mdc.DrawLine(xp, yp + 5, xp, yp - 5);
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, sx, sy, &mdc, 0, 0);
}

