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

#ifndef __ROLLOVERWIN_H__
#define __ROLLOVERWIN_H__

#include <wx/window.h>
#include <wx/timer.h>
#include <global/ColorScheme.h>

class RolloverWin : public wxWindow
{
	DECLARE_EVENT_TABLE()

public:
	// constants for rollovers fonts
	enum {
		AIS_ROLLOVER = 1,
		LEG_ROLLOVER = 2,
		TC_ROLLOVER = 3
	};

public:
	RolloverWin(wxWindow* parent, int timeout = -1);
	virtual ~RolloverWin();

	void OnPaint(wxPaintEvent& event);
	void SetColorScheme(global::ColorScheme cs);
	void SetString(const wxString& s);
	void SetPosition(wxPoint pt);
	void SetBitmap(int rollover);
	wxBitmap* GetBitmap();
	void SetBestPosition(int x, int y, int off_x, int off_y, int rollover, wxSize parent_size);
	void OnTimer(wxTimerEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void SetMousePropogation(int level);
	bool IsActive() const;
	void IsActive(bool state);

private:
	wxString m_string;
	wxSize m_size;
	wxPoint m_position;
	wxBitmap* m_pbm;
	wxTimer m_timer_timeout;
	int m_timeout_sec;
	int m_mmouse_propogate;
	bool isActive;
	wxFont* m_plabelFont;
};

#endif
