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

#ifndef __PIANOWIN__H__
#define __PIANOWIN__H__

#include <global/ColorScheme.h>

#include <wx/window.h>
#include <wx/brush.h>
#include <wx/bitmap.h>

#include <vector>

class PianoWin : public wxWindow
{
	DECLARE_EVENT_TABLE()

public:
	PianoWin(wxFrame* frame);
	virtual ~PianoWin();

	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void FormatKeys(void);
	void MouseEvent(wxMouseEvent& event);
	void SetColorScheme(global::ColorScheme cs);
	void SetKeyArray(std::vector<int> piano_chart_index_array);
	void SetActiveKey(int iactive);
	void SetActiveKeyArray(std::vector<int> array);
	void SetNoshowIndexArray(std::vector<int> array);
	void SetSubliteIndexArray(std::vector<int> array);
	void SetSkewIndexArray(std::vector<int> array);
	void SetTmercIndexArray(std::vector<int> array);
	void SetPolyIndexArray(std::vector<int> array);

	void SetVizIcon(wxBitmap* picon_bmp);
	void SetInVizIcon(wxBitmap* picon_bmp);
	void SetSkewIcon(wxBitmap* picon_bmp);
	void SetTMercIcon(wxBitmap* picon_bmp);
	void SetPolyIcon(wxBitmap* picon_bmp);

	wxPoint GetKeyOrigin(int key_index);
	void ResetRollover(void);
	void SetRoundedRectangles(bool val);

private:
	wxBitmap ConvertTo24Bit(wxColor bgColor, wxBitmap front);

	int m_nRegions;
	int m_index_last;
	int m_hover_icon_last;
	int m_hover_last;

	wxBrush m_backBrush;
	wxBrush m_tBrush;
	wxBrush m_vBrush;
	wxBrush m_svBrush;
	wxBrush m_uvBrush;
	wxBrush m_slBrush;

	wxBrush m_cBrush;
	wxBrush m_scBrush;

	std::vector<int> m_key_array;
	std::vector<int> m_noshow_index_array;
	std::vector<int> m_active_index_array;
	std::vector<int> m_sublite_index_array;
	std::vector<int> m_skew_index_array;
	std::vector<int> m_tmerc_index_array;
	std::vector<int> m_poly_index_array;

	std::vector<wxRegion> KeyRegion;

	wxBitmap* m_pVizIconBmp;
	wxBitmap* m_pInVizIconBmp;
	wxBitmap* m_pTmercIconBmp;
	wxBitmap* m_pSkewIconBmp;
	wxBitmap* m_pPolyIconBmp;

	int m_iactive;
	bool m_brounded;
};

#endif
