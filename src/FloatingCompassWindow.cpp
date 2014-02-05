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
#include <StyleManager.h>
#include <Style.h>

#include <ChartCanvas.h>

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/Navigation.h>
#include <global/ColorManager.h>

#include <wx/toolbar.h>
#include <wx/statbmp.h>

BEGIN_EVENT_TABLE(FloatingCompassWindow, wxWindow)
	EVT_PAINT(FloatingCompassWindow::OnPaint)
END_EVENT_TABLE()

extern ocpnStyle::StyleManager* g_StyleManager;
extern ChartCanvas* cc1;

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

	ocpnStyle::Style& style = g_StyleManager->current();
	_img_compass = style.GetIcon(_T("CompassRose"));
	_img_gpsRed = style.GetIcon(_T("gpsRed"));

	m_rose_angle = -999; // force a refresh when first used

	m_pStatBoxToolStaticBmp = NULL;

	SetSize(_img_compass.GetWidth() + _img_gpsRed.GetWidth() + style.GetCompassLeftMargin() * 2
			+ style.GetToolSeparation(),
			_img_compass.GetHeight() + style.GetCompassTopMargin()
			+ style.GetCompassBottomMargin());

	m_xoffset = style.GetCompassXOffset();
	m_yoffset = style.GetCompassYOffset();
}

FloatingCompassWindow::~FloatingCompassWindow()
{
	delete m_pStatBoxToolStaticBmp;
}

void FloatingCompassWindow::OnPaint(wxPaintEvent&)
{
	int width;
	int height;
	GetClientSize(&width, &height);
	wxPaintDC dc(this);

	dc.DrawBitmap(m_StatBmp, 0, 0, false);
}

void FloatingCompassWindow::SetColorScheme(global::ColorScheme)
{
	wxColour back_color = global::OCPN::get().color().get_color(_T("GREY2"));

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
	ocpnStyle::Style& style = g_StyleManager->current();

	// In order to draw a horizontal compass window when the toolbar is vertical, we
	// need to save away the sizes and backgrounds for the two icons.

	// FIXME: static data inside method... investigate
	static wxBitmap compassBg;
	static wxBitmap gpsBg;
	static wxSize toolsize;
	static int topmargin;
	static int leftmargin;
	static int radius;

	if (!compassBg.IsOk() || newColorScheme) {
		int orient = style.GetOrientation();
		style.SetOrientation(wxTB_HORIZONTAL);
		if (style.HasBackground()) {
			compassBg = style.GetNormalBG();
			style.DrawToolbarLineStart(compassBg);
			compassBg = style.SetBitmapBrightness(compassBg);
			gpsBg = style.GetNormalBG();
			style.DrawToolbarLineEnd(gpsBg);
			gpsBg = style.SetBitmapBrightness(gpsBg);
		}

		leftmargin = style.GetCompassLeftMargin();
		topmargin = style.GetCompassTopMargin();
		toolsize = style.GetToolSize();
		toolsize.x *= 2;
		radius = style.GetCompassCornerRadius();

		if (orient)
			style.SetOrientation(wxTB_VERTICAL);
	}

	bool b_need_refresh = false;
	const global::Navigation::GPS& gps = global::OCPN::get().nav().gps();
	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

	if (gps.valid) {
		if (gps.SatValid) {
			gpsIconName = _T("gps3Bar");
			if (gps.SatsInView <= 8)
				gpsIconName = _T("gps2Bar");
			if (gps.SatsInView <= 4)
				gpsIconName = _T("gps1Bar");
			if (gps.SatsInView < 0)
				gpsIconName = _T("gpsGry");

		} else
			gpsIconName = _T("gpsGrn");
	} else
		gpsIconName = _T("gpsRed");

	if (m_lastgpsIconName != gpsIconName)
		b_need_refresh = true;

	double rose_angle = -999.0;

	if ((fabs(cc1->GetVPRotation()) > 0.01) || (fabs(cc1->GetVPSkew()) > 0.01)) {
		rose_angle = -cc1->GetVPRotation();

		if (!nav.CourseUp && !global::OCPN::get().gui().view().skew_comp)
			rose_angle = -cc1->GetVPRotation() - cc1->GetVPSkew();

		b_need_refresh = true;
	} else
		rose_angle = 0.0;

	if (fabs(m_rose_angle - rose_angle) > 0.001)
		b_need_refresh = true;

	if (b_need_refresh) {
		wxBitmap StatBmp;

		StatBmp.Create((_img_compass.GetWidth() + _img_gpsRed.GetWidth())
					   + style.GetCompassLeftMargin() * 2 + style.GetToolSeparation(),
					   _img_compass.GetHeight() + style.GetCompassTopMargin()
					   + style.GetCompassBottomMargin());

		if (StatBmp.IsOk()) {
			const global::ColorManager& colors = global::OCPN::get().color();

			wxMemoryDC mdc;
			mdc.SelectObject(StatBmp);
			mdc.SetBackground(wxBrush(colors.get_color(_T("GREY2")), wxSOLID));
			mdc.Clear();

			mdc.SetPen(wxPen(colors.get_color(_T("UITX1")), 1));
			mdc.SetBrush(wxBrush(colors.get_color(_T("UITX1")), wxTRANSPARENT));

			mdc.DrawRoundedRectangle(0, 0, StatBmp.GetWidth(), StatBmp.GetHeight(),
									 style.GetCompassCornerRadius());

			wxPoint offset(style.GetCompassLeftMargin(), style.GetCompassTopMargin());

			// Build Compass Rose, rotated...
			wxBitmap BMPRose;
			wxPoint after_rotate;

			if (nav.CourseUp)
				BMPRose = style.GetIcon(_T("CompassRose"));
			else
				BMPRose = style.GetIcon(_T("CompassRoseBlue"));
			if ((fabs(cc1->GetVPRotation()) > .01) || (fabs(cc1->GetVPSkew()) > 0.01)) {
				wxPoint rot_ctr(BMPRose.GetWidth() / 2, BMPRose.GetHeight() / 2);
				wxImage rose_img = BMPRose.ConvertToImage();

				wxImage rot_image = rose_img.Rotate(rose_angle, rot_ctr, true, &after_rotate);
				BMPRose = wxBitmap(rot_image).GetSubBitmap(wxRect(
					-after_rotate.x, -after_rotate.y, BMPRose.GetWidth(), BMPRose.GetHeight()));
			}

			wxBitmap iconBm;

			if (style.HasBackground()) {
				iconBm = ocpnStyle::MergeBitmaps(compassBg, BMPRose, wxSize(0, 0));
			} else {
				iconBm = BMPRose;
			}

			mdc.DrawBitmap(iconBm, offset);
			offset.x += iconBm.GetWidth();

			m_rose_angle = rose_angle;

			if (style.HasBackground()) {
				iconBm = ocpnStyle::MergeBitmaps(gpsBg, style.GetIcon(gpsIconName), wxSize(0, 0));
			} else {
				iconBm = style.GetIcon(gpsIconName);
			}
			mdc.DrawBitmap(iconBm, offset);
			mdc.SelectObject(wxNullBitmap);
			m_lastgpsIconName = gpsIconName;
		}

#ifndef __WXMAC__
		if (style.isMarginsInvisible()) {
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

