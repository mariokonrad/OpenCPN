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

#include "AnnunText.h"
#include <FontMgr.h>
#include <StyleManager.h>
#include <Style.h>
#include <Routeman.h>
#include <UserColors.h>

#include <wx/dcmemory.h>

extern ocpnStyle::StyleManager* g_StyleManager;

BEGIN_EVENT_TABLE(AnnunText, wxWindow)
	EVT_PAINT(AnnunText::OnPaint)
END_EVENT_TABLE()

AnnunText::AnnunText(
		wxWindow * parent,
		wxWindowID id,
		const wxString & LegendElement,
		const wxString& ValueElement)
	: wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	m_label = _T("Label");
	m_value = _T("-----");

	m_plabelFont = wxTheFontList->FindOrCreateFont(14, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD, FALSE,
												   wxString(_T("Arial Bold")));
	m_pvalueFont
		= wxTheFontList->FindOrCreateFont(24, wxFONTFAMILY_DEFAULT, wxNORMAL, wxBOLD, FALSE,
										  wxString(_T("helvetica")), wxFONTENCODING_ISO8859_1);

	m_LegendTextElement = LegendElement;
	m_ValueTextElement = ValueElement;

	RefreshFonts();
}

AnnunText::~AnnunText()
{}

void AnnunText::CalculateMinSize(void)
{
	// Calculate the minimum required size of the window based on text size

	int wl = 50; // reasonable defaults?
	int hl = 20;
	int wv = 50;
	int hv = 20;

	if (m_plabelFont)
		GetTextExtent(_T("1234"), &wl, &hl, NULL, NULL, m_plabelFont);

	if (m_pvalueFont)
		GetTextExtent(_T("123.456"), &wv, &hv, NULL, NULL, m_pvalueFont);

	wxSize min;
	min.x = wl + wv;
	min.y = (int)((hl + hv) * 1.2);

	SetMinSize(min);
}

void AnnunText::SetColorScheme(ColorScheme)
{
	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
	m_pbackBrush = wxTheBrushList->FindOrCreateBrush(GetGlobalColor(_T("UBLCK")), wxSOLID);

	m_text_color = style->getConsoleFontColor();
}

void AnnunText::RefreshFonts()
{
	m_plabelFont = FontMgr::Get().GetFont(m_LegendTextElement);
	m_pvalueFont = FontMgr::Get().GetFont(m_ValueTextElement);

	CalculateMinSize();
}

void AnnunText::SetLegendElement(const wxString& element)
{
	m_LegendTextElement = element;
}

void AnnunText::SetValueElement(const wxString& element)
{
	m_ValueTextElement = element;
}

void AnnunText::SetALabel(const wxString& l)
{
	m_label = l;
}

void AnnunText::SetAValue(const wxString& v)
{
	m_value = v;
}

void AnnunText::OnPaint(wxPaintEvent &)
{
	int sx;
	int sy;
	GetClientSize(&sx, &sy);
	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

	// Do the drawing on an off-screen memory DC, and blit into place
	// to avoid objectionable flashing
	wxMemoryDC mdc;

	wxBitmap m_bitmap(sx, sy, -1);
	mdc.SelectObject(m_bitmap);
	mdc.SetBackground(*m_pbackBrush);
	mdc.Clear();

	const wxBitmap& background = style->getConsoleTextBackground();
	if (background.IsOk())
		mdc.DrawBitmap(background, 0, 0);

	mdc.SetTextForeground(m_text_color);

	if (m_plabelFont) {
		mdc.SetFont(*m_plabelFont);
		if (m_pbackBrush->GetColour() != FontMgr::Get().GetFontColor(_("Console Legend")))
			mdc.SetTextForeground(FontMgr::Get().GetFontColor(_("Console Legend")));
		mdc.DrawText(m_label, 5, 2);
	}

	if (m_pvalueFont) {
		mdc.SetFont(*m_pvalueFont);
		if (m_pbackBrush->GetColour() != FontMgr::Get().GetFontColor(_("Console Value")))
			mdc.SetTextForeground(FontMgr::Get().GetFontColor(_("Console Value")));

		int w, h;
		mdc.GetTextExtent(m_value, &w, &h);
		int cw, ch;
		mdc.GetSize(&cw, &ch);

		mdc.DrawText(m_value, cw - w - 2, ch - h - 2);
	}

	wxPaintDC dc(this);
	dc.Blit(0, 0, sx, sy, &mdc, 0, 0);
}

