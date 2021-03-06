/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __CM93DSLIDE__H__
#define __CM93DSLIDE__H__

#include <wx/dialog.h>

class wxSlider;

class CM93DSlide : public wxDialog
{
	DECLARE_EVENT_TABLE()

public:
	CM93DSlide(wxWindow* parent, wxWindowID id, int value, int minValue, int maxValue,
			   const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
			   long style = 0, const wxString& title = _T(""));

	virtual ~CM93DSlide(void);

	void Init(void);

	bool Create(wxWindow* parent, wxWindowID id, int value, int minValue, int maxValue,
				const wxPoint& pos, const wxSize& size, long style, const wxString& title);

private:
	void OnCancelClick(wxCommandEvent& event);
	void OnMove(wxMoveEvent& event);
	void OnChangeValue(wxScrollEvent& event);
	void OnClose(wxCloseEvent& event);

	wxSlider* m_pCM93DetailSlider;
	wxWindow* m_pparent;
};

#endif
