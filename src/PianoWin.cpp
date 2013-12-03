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

#include "PianoWin.h"
#include <StyleManager.h>
#include <Style.h>
#include <UserColors.h>
#include <chart/ChartDB.h>

BEGIN_EVENT_TABLE(PianoWin, wxWindow)
	EVT_PAINT(PianoWin::OnPaint)
	EVT_SIZE(PianoWin::OnSize)
	EVT_MOUSE_EVENTS(PianoWin::MouseEvent)
END_EVENT_TABLE()

extern ocpnStyle::StyleManager* g_StyleManager;
extern chart::ChartDB* ChartData;
extern MainFrame* gFrame;

PianoWin::PianoWin(wxFrame* frame)
	: wxWindow(frame, wxID_ANY, wxPoint(20, 20), wxSize(5, 5), wxNO_BORDER)
	, m_nRegions(0)
	, m_index_last(-1)
	, m_hover_icon_last(-1)
	, m_hover_last(-1)
	, m_pVizIconBmp(NULL)
	, m_pInVizIconBmp(NULL)
	, m_pTmercIconBmp(NULL)
	, m_pSkewIconBmp(NULL)
	, m_pPolyIconBmp(NULL)
	, m_iactive(-1)
	, m_brounded(false)
{
	// on WXMSW, this prevents flashing on color scheme change
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

PianoWin::~PianoWin()
{
	if (m_pInVizIconBmp)
		delete m_pInVizIconBmp;
	if (m_pPolyIconBmp)
		delete m_pPolyIconBmp;
	if (m_pSkewIconBmp)
		delete m_pSkewIconBmp;
	if (m_pTmercIconBmp)
		delete m_pTmercIconBmp;
	if (m_pVizIconBmp)
		delete m_pVizIconBmp;
}

void PianoWin::OnSize(wxSizeEvent&)
{
}

void PianoWin::SetColorScheme(ColorScheme)
{
	// Recreate the local brushes

	m_backBrush = wxBrush(GetGlobalColor(_T("UIBDR")), wxSOLID);

	m_tBrush = wxBrush(GetGlobalColor(_T("BLUE2")), wxSOLID); // Raster Chart unselected
	m_slBrush = wxBrush(GetGlobalColor(_T("BLUE1")), wxSOLID); // and selected

	m_vBrush = wxBrush(GetGlobalColor(_T("GREEN2")), wxSOLID); // Vector Chart unselected
	m_svBrush = wxBrush(GetGlobalColor(_T("GREEN1")), wxSOLID); // and selected

	m_cBrush = wxBrush(GetGlobalColor(_T("YELO2")), wxSOLID); // CM93 Chart unselected
	m_scBrush = wxBrush(GetGlobalColor(_T("YELO1")), wxSOLID); // and selected

	m_uvBrush = wxBrush(GetGlobalColor(_T("UINFD")), wxSOLID); // and unavailable
}

void PianoWin::OnPaint(wxPaintEvent&)
{
	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
	int width, height;
	GetClientSize(&width, &height);
	wxPaintDC dc(this);

	wxBitmap shape = wxBitmap(width, height);
	wxMemoryDC shapeDc(shape);
	shapeDc.SetBackground(*wxBLACK_BRUSH);
	shapeDc.SetBrush(*wxWHITE_BRUSH);
	shapeDc.SetPen(*wxWHITE_PEN);
	shapeDc.Clear();

	dc.SetBackground(m_backBrush);
	dc.Clear();

	// Create the Piano Keys

	int nKeys = m_key_array.size();

	if (nKeys) {
		wxPen ppPen(GetGlobalColor(_T("CHBLK")), 1, wxSOLID);
		dc.SetPen(ppPen);

		dc.SetBrush(m_tBrush);

		for (int i = 0; i < nKeys; i++) {
			int key_db_index = m_key_array.at(i);

			if (-1 == key_db_index)
				continue;

			if (ChartData->GetDBChartType(m_key_array.at(i)) == chart::CHART_TYPE_S57) {
				dc.SetBrush(m_vBrush);

				for (unsigned int ino = 0; ino < m_active_index_array.size(); ino++) {
					if (m_active_index_array.at(ino) == key_db_index) // chart is in the active list
						dc.SetBrush(m_svBrush);
				}
			} else if (ChartData->GetDBChartType(m_key_array.at(i)) == chart::CHART_TYPE_CM93) {
				dc.SetBrush(m_cBrush);

				for (unsigned int ino = 0; ino < m_active_index_array.size(); ino++) {
					if (m_active_index_array.at(ino) == key_db_index) // chart is in the active list
						dc.SetBrush(m_scBrush);
				}
			} else if (ChartData->GetDBChartType(m_key_array.at(i)) == chart::CHART_TYPE_CM93COMP) {
				dc.SetBrush(m_cBrush);

				for (unsigned int ino = 0; ino < m_active_index_array.size(); ino++) {
					if (m_active_index_array.at(ino) == key_db_index) // chart is in the active list
						dc.SetBrush(m_scBrush);
				}
			} else {
				dc.SetBrush(m_tBrush);

				for (unsigned int ino = 0; ino < m_active_index_array.size(); ino++) {
					if (m_active_index_array.at(ino) == key_db_index) // chart is in the active list
						dc.SetBrush(m_slBrush);
				}
			}

			// Check to see if this box appears in the sub_light array
			// If so, add a crosshatch pattern to the brush
			for (unsigned int ino = 0; ino < m_sublite_index_array.size(); ino++) {
				if (m_sublite_index_array.at(ino) == key_db_index) // chart is in the sublite list
				{
					wxBrush ebrush(dc.GetBrush().GetColour(), wxCROSSDIAG_HATCH);
					//                              dc.SetBrush(ebrush);
				}
			}

			wxRect box = KeyRegion.at(i).GetBox();

			if (m_brounded) {
				dc.DrawRoundedRectangle(box.x, box.y, box.width, box.height, 4);
				shapeDc.DrawRoundedRectangle(box.x, box.y, box.width, box.height, 4);
			} else {
				dc.DrawRectangle(box);
				shapeDc.DrawRectangle(box);
			}

			for (unsigned int ino = 0; ino < m_sublite_index_array.size(); ino++) {
				if (m_sublite_index_array.at(ino) == key_db_index) { // chart is in the sublite list
					dc.SetBrush(dc.GetBackground());
					int w = 3;
					dc.DrawRoundedRectangle(box.x + w, box.y + w, box.width - (2 * w),
											box.height - (2 * w), 3);
				}
			}

			//    Look in the current noshow array for this index
			for (unsigned int ino = 0; ino < m_noshow_index_array.size(); ino++) {
				if (m_noshow_index_array[ino] == key_db_index) { // chart is in the noshow list
					if (m_pInVizIconBmp && m_pInVizIconBmp->IsOk())
						dc.DrawBitmap(
							ocpnStyle::ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pInVizIconBmp),
							box.x + 4, box.y + 3, false);
					break;
				}
			}

			//    Look in the current skew array for this index
			for (unsigned int ino = 0; ino < m_skew_index_array.size(); ino++) {
				if (m_skew_index_array.at(ino) == key_db_index) { // chart is in the list
					if (m_pSkewIconBmp && m_pSkewIconBmp->IsOk())
						dc.DrawBitmap(
							ocpnStyle::ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pSkewIconBmp),
							box.x + box.width - m_pSkewIconBmp->GetWidth() - 4, box.y + 2, false);
					break;
				}
			}

			//    Look in the current tmerc array for this index
			for (unsigned int ino = 0; ino < m_tmerc_index_array.size(); ino++) {
				if (m_tmerc_index_array.at(ino) == key_db_index) { // chart is in the list
					if (m_pTmercIconBmp && m_pTmercIconBmp->IsOk())
						dc.DrawBitmap(
							ocpnStyle::ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pTmercIconBmp),
							box.x + box.width - m_pTmercIconBmp->GetWidth() - 4, box.y + 2, false);
					break;
				}
			}

			//    Look in the current poly array for this index
			for (unsigned int ino = 0; ino < m_poly_index_array.size(); ino++) {
				if (m_poly_index_array.at(ino) == key_db_index) { // chart is in the list
					if (m_pPolyIconBmp && m_pPolyIconBmp->IsOk())
						dc.DrawBitmap(
							ocpnStyle::ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pPolyIconBmp),
							box.x + box.width - m_pPolyIconBmp->GetWidth() - 4, box.y + 2, false);
					break;
				}
			}
		}
#ifndef __WXMAC__
		if (style->isChartStatusWindowTransparent())
			((wxDialog*) GetParent())->SetShape( wxRegion( shape, *wxBLACK, 0 ) );
	} else {
		// SetShape() with a completely empty shape doesn't work, and leaving the shape
		// but hiding the window causes artifacts when dragging in GL mode on MSW.
		// The best solution found so far is to show just a single pixel, this is less
		// disturbing than flashing piano keys when dragging. (wxWidgets 2.8)
		if (style->isChartStatusWindowTransparent())
			((wxDialog*) GetParent())->SetShape( wxRegion( wxRect(0,0,1,1) ));
#endif
	}
}

void PianoWin::SetKeyArray(std::vector<int> array)
{
	m_key_array = array;
	FormatKeys();
}

void PianoWin::SetNoshowIndexArray(std::vector<int> array)
{
	m_noshow_index_array = array;
}

void PianoWin::SetActiveKeyArray(std::vector<int> array)
{
	m_active_index_array = array;
}

void PianoWin::SetSubliteIndexArray(std::vector<int> array)
{
	m_sublite_index_array = array;
}

void PianoWin::SetSkewIndexArray(std::vector<int> array)
{
	m_skew_index_array = array;
}

void PianoWin::SetTmercIndexArray(std::vector<int> array)
{
	m_tmerc_index_array = array;
}

void PianoWin::SetPolyIndexArray(std::vector<int> array)
{
	m_poly_index_array = array;
}

void PianoWin::FormatKeys(void)
{
	int nKeys = m_key_array.size();
	if (nKeys) {
		ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
		int width;
		int height;
		GetClientSize(&width, &height);
		int kw = style->getChartStatusIconWidth();
		if (!kw)
			kw = width / nKeys;

		// Build the Key Regions

		KeyRegion.clear();
		KeyRegion.reserve(nKeys);
		for (int i = 0; i < nKeys; i++) {
			wxRegion r((i * kw) + 3, 2, kw - 6, height - 4);
			KeyRegion.push_back(r);
		}
	}
	m_nRegions = nKeys;
}

wxPoint PianoWin::GetKeyOrigin(int key_index)
{
	if ((key_index >= 0) && (key_index <= (int)m_key_array.size() - 1)) {
		wxRect box = KeyRegion.at(key_index).GetBox();
		return wxPoint(box.x, box.y);
	}

	return wxPoint(-1, -1);
}

void PianoWin::MouseEvent(wxMouseEvent& event)
{
	int x;
	int y;
	event.GetPosition(&x, &y);

	// Check the regions

	int sel_index = -1;
	int sel_dbindex = -1;

	for (int i = 0; i < m_nRegions; i++) {
		if (KeyRegion.at(i).Contains(x, y) == wxInRegion) {
			sel_index = i;
			sel_dbindex = m_key_array.at(i);
			break;
		}
	}

	if (event.LeftDown()) {
		if (-1 != sel_index) {
			gFrame->HandlePianoClick(sel_index, sel_dbindex);
			gFrame->Raise();
		}
	} else if (event.RightDown()) {
		if (-1 != sel_index) {
			gFrame->HandlePianoRClick(x, y, sel_index, sel_dbindex);
			gFrame->Raise();
		}
	} else if (!event.ButtonUp()) {
		if (sel_index != m_hover_last) {
			gFrame->HandlePianoRollover(sel_index, sel_dbindex);
			m_hover_last = sel_index;
		}
	}

	if (event.Leaving()) {
		gFrame->HandlePianoRollover(-1, -1);
		gFrame->HandlePianoRolloverIcon(-1, -1);

		m_index_last = -1;
		m_hover_icon_last = -1;
		m_hover_last = -1;
	}
}

void PianoWin::ResetRollover(void)
{
	m_index_last = -1;
	m_hover_icon_last = -1;
	m_hover_last = -1;
}

void PianoWin::SetVizIcon(wxBitmap* picon_bmp)
{
	if (m_pVizIconBmp)
		delete m_pVizIconBmp;
	m_pVizIconBmp = picon_bmp;
}

void PianoWin::SetInVizIcon(wxBitmap* picon_bmp)
{
	if (m_pInVizIconBmp)
		delete m_pInVizIconBmp;
	m_pInVizIconBmp = picon_bmp;
}

void PianoWin::SetSkewIcon(wxBitmap* picon_bmp)
{
	if (m_pSkewIconBmp)
		delete m_pSkewIconBmp;
	m_pSkewIconBmp = picon_bmp;
}

void PianoWin::SetTMercIcon(wxBitmap* picon_bmp)
{
	if (m_pTmercIconBmp)
		delete m_pTmercIconBmp;
	m_pTmercIconBmp = picon_bmp;
}

void PianoWin::SetPolyIcon(wxBitmap* picon_bmp)
{
	if (m_pPolyIconBmp)
		delete m_pPolyIconBmp;
	m_pPolyIconBmp = picon_bmp;
}

void PianoWin::SetActiveKey(int iactive)
{
	m_iactive = iactive;
}

void PianoWin::SetRoundedRectangles(bool val)
{
	m_brounded = val;
}

