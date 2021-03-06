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

#ifndef __STATWIN_H__
#define __STATWIN_H__

#include <wx/dialog.h>
#include <wx/brush.h>
#include <global/ColorScheme.h>

class PianoWin;
class TextStatWin;

class StatWin : public wxDialog
{
	DECLARE_EVENT_TABLE()

public:
	StatWin(wxWindow* win);
	virtual ~StatWin();
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	int GetFontHeight();
	int GetRows() const;
	void SetColorScheme(global::ColorScheme cs);
	void RePosition();
	void ReSize();

	void FormatStat(void);

	PianoWin* pPiano;

private:
	TextStatWin* pTStat1;
	TextStatWin* pTStat2;

	wxBrush m_backBrush;
	int m_rows;
};

#endif
