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

#include "FloatingCompassWindow.h"
#include "StyleManager.h"
#include "Style.h"

#include <ChartCanvas.h>
#include <UserColors.h>

#include <wx/toolbar.h>
#include <wx/statbmp.h>

BEGIN_EVENT_TABLE(FloatingCompassWindow, wxWindow)
	EVT_PAINT(FloatingCompassWindow::OnPaint)
END_EVENT_TABLE()

extern ocpnStyle::StyleManager* g_StyleManager;
extern ChartCanvas* cc1;
extern bool bGPSValid;
extern bool g_bSatValid;
extern int g_SatsInView;
extern bool g_bCourseUp;
extern bool g_bskew_comp;

FloatingCompassWindow::FloatingCompassWindow(wxWindow* parent)
{
	m_pparent = parent;
	long wstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
	wstyle |= wxFRAME_SHAPED;
#endif
#ifdef __WXMAC__
	wstyle |= wxSTAY_ON_TOP;
#endif
	wxDialog::Create(parent, -1, _T(""), wxPoint(0, 0), wxSize(-1, -1), wstyle);

	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
	_img_compass = style->GetIcon(_T("CompassRose"));
	_img_gpsRed = style->GetIcon(_T("gpsRed"));

	m_rose_angle = -999; // force a refresh when first used

	m_pStatBoxToolStaticBmp = NULL;

	SetSize(_img_compass.GetWidth() + _img_gpsRed.GetWidth() + style->GetCompassLeftMargin() * 2
			+ style->GetToolSeparation(),
			_img_compass.GetHeight() + style->GetCompassTopMargin()
			+ style->GetCompassBottomMargin());

	m_xoffset = style->GetCompassXOffset();
	m_yoffset = style->GetCompassYOffset();
}

FloatingCompassWindow::~FloatingCompassWindow()
{
	delete m_pStatBoxToolStaticBmp;
}

void FloatingCompassWindow::OnPaint(wxPaintEvent&)
{
	int width, height;
	GetClientSize(&width, &height);
	wxPaintDC dc(this);

	dc.DrawBitmap(m_StatBmp, 0, 0, false);
}

void FloatingCompassWindow::SetColorScheme(global::ColorScheme)
{
	wxColour back_color = GetGlobalColor(_T("GREY2"));

	// Set background
	SetBackgroundColour(back_color);
	ClearBackground();

	UpdateStatus(true);
}

void FloatingCompassWindow::UpdateStatus(bool bnew)
{
	if (bnew)
		m_lastgpsIconName.Clear(); // force an update to occur

	wxBitmap statbmp = CreateBmp(bnew);
	if (statbmp.IsOk())
		m_StatBmp = statbmp;

	Show();
	Refresh(false);
}

wxBitmap FloatingCompassWindow::CreateBmp(bool newColorScheme)
{
	wxString gpsIconName;
	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

	// In order to draw a horizontal compass window when the toolbar is vertical, we
	// need to save away the sizes and backgrounds for the two icons.

	static wxBitmap compassBg, gpsBg;
	static wxSize toolsize;
	static int topmargin, leftmargin, radius;

	if (!compassBg.IsOk() || newColorScheme) {
		int orient = style->GetOrientation();
		style->SetOrientation(wxTB_HORIZONTAL);
		if (style->HasBackground()) {
			compassBg = style->GetNormalBG();
			style->DrawToolbarLineStart(compassBg);
			compassBg = style->SetBitmapBrightness(compassBg);
			gpsBg = style->GetNormalBG();
			style->DrawToolbarLineEnd(gpsBg);
			gpsBg = style->SetBitmapBrightness(gpsBg);
		}

		leftmargin = style->GetCompassLeftMargin();
		topmargin = style->GetCompassTopMargin();
		toolsize = style->GetToolSize();
		toolsize.x *= 2;
		radius = style->GetCompassCornerRadius();

		if (orient)
			style->SetOrientation(wxTB_VERTICAL);
	}

	bool b_need_refresh = false;

	if (bGPSValid) {
		if (g_bSatValid) {
			gpsIconName = _T("gps3Bar");
			if (g_SatsInView <= 8)
				gpsIconName = _T("gps2Bar");
			if (g_SatsInView <= 4)
				gpsIconName = _T("gps1Bar");
			if (g_SatsInView < 0)
				gpsIconName = _T("gpsGry");

		} else
			gpsIconName = _T("gpsGrn");
	} else
		gpsIconName = _T("gpsRed");

	if (m_lastgpsIconName != gpsIconName)
		b_need_refresh = true;

	double rose_angle = -999.;

	if ((fabs(cc1->GetVPRotation()) > .01) || (fabs(cc1->GetVPSkew()) > .01)) {
		rose_angle = -cc1->GetVPRotation();

		if (!g_bCourseUp && !g_bskew_comp)
			rose_angle = -cc1->GetVPRotation() - cc1->GetVPSkew();

		b_need_refresh = true;
	} else
		rose_angle = 0.;

	if (fabs(m_rose_angle - rose_angle) > .001)
		b_need_refresh = true;

	if (b_need_refresh) {
		wxBitmap StatBmp;

		StatBmp.Create((_img_compass.GetWidth() + _img_gpsRed.GetWidth())
					   + style->GetCompassLeftMargin() * 2 + style->GetToolSeparation(),
					   _img_compass.GetHeight() + style->GetCompassTopMargin()
					   + style->GetCompassBottomMargin());

		if (StatBmp.IsOk()) {

			wxMemoryDC mdc;
			mdc.SelectObject(StatBmp);
			mdc.SetBackground(wxBrush(GetGlobalColor(_T("GREY2")), wxSOLID));
			mdc.Clear();

			mdc.SetPen(wxPen(GetGlobalColor(_T("UITX1")), 1));
			mdc.SetBrush(wxBrush(GetGlobalColor(_T("UITX1")), wxTRANSPARENT));

			mdc.DrawRoundedRectangle(0, 0, StatBmp.GetWidth(), StatBmp.GetHeight(),
									 style->GetCompassCornerRadius());

			wxPoint offset(style->GetCompassLeftMargin(), style->GetCompassTopMargin());

			//    Build Compass Rose, rotated...
			wxBitmap BMPRose;
			wxPoint after_rotate;

			if (g_bCourseUp)
				BMPRose = style->GetIcon(_T("CompassRose"));
			else
				BMPRose = style->GetIcon(_T("CompassRoseBlue"));
			if ((fabs(cc1->GetVPRotation()) > .01) || (fabs(cc1->GetVPSkew()) > .01)) {
				wxPoint rot_ctr(BMPRose.GetWidth() / 2, BMPRose.GetHeight() / 2);
				wxImage rose_img = BMPRose.ConvertToImage();

				wxImage rot_image = rose_img.Rotate(rose_angle, rot_ctr, true, &after_rotate);
				BMPRose = wxBitmap(rot_image).GetSubBitmap(wxRect(
					-after_rotate.x, -after_rotate.y, BMPRose.GetWidth(), BMPRose.GetHeight()));
			}

			wxBitmap iconBm;

			if (style->HasBackground()) {
				iconBm = ocpnStyle::MergeBitmaps(compassBg, BMPRose, wxSize(0, 0));
			} else {
				iconBm = BMPRose;
			}

			mdc.DrawBitmap(iconBm, offset);
			offset.x += iconBm.GetWidth();

			m_rose_angle = rose_angle;

			if (style->HasBackground()) {
				iconBm = ocpnStyle::MergeBitmaps(gpsBg, style->GetIcon(gpsIconName), wxSize(0, 0));
			} else {
				iconBm = style->GetIcon(gpsIconName);
			}
			mdc.DrawBitmap(iconBm, offset);
			mdc.SelectObject(wxNullBitmap);
			m_lastgpsIconName = gpsIconName;
		}

#ifndef __WXMAC__
		if (style->isMarginsInvisible()) {
			m_MaskBmp = wxBitmap(StatBmp.GetWidth(), StatBmp.GetHeight());
			wxMemoryDC sdc(m_MaskBmp);
			sdc.SetBackground(*wxWHITE_BRUSH);
			sdc.Clear();
			sdc.SetBrush(*wxBLACK_BRUSH);
			sdc.SetPen(*wxBLACK_PEN);
			sdc.DrawRoundedRectangle(wxPoint(leftmargin, topmargin), toolsize, radius);
			sdc.SelectObject(wxNullBitmap);
			SetShape(wxRegion(m_MaskBmp, *wxWHITE, 0));
		} else if (radius) {
			m_MaskBmp = wxBitmap(GetSize().x, GetSize().y);
			wxMemoryDC sdc(m_MaskBmp);
			sdc.SetBackground(*wxWHITE_BRUSH);
			sdc.Clear();
			sdc.SetBrush(*wxBLACK_BRUSH);
			sdc.SetPen(*wxBLACK_PEN);
			sdc.DrawRoundedRectangle(0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight(), radius);
			sdc.SelectObject(wxNullBitmap);
			SetShape(wxRegion(m_MaskBmp, *wxWHITE, 0));
		}
#endif

		return StatBmp;
	} else
		return wxNullBitmap;
}

int FloatingCompassWindow::GetXOffset(void) const
{
	return m_xoffset;
}

int FloatingCompassWindow::GetYOffset(void) const
{
	return m_yoffset;
}

