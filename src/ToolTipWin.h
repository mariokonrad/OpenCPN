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

#ifndef __TOOLTIPWIN__H__
#define __TOOLTIPWIN__H__

#include <wx/dialog.h>
#include <ColorScheme.h>
#include "navutil.h"

class ToolTipWin : public wxDialog
{
		DECLARE_EVENT_TABLE()

	public:
		ToolTipWin(wxWindow *parent);
		~ToolTipWin();

		void OnPaint(wxPaintEvent& event);
		void SetColorScheme(ColorScheme cs);

		void SetString(const wxString & s)
		{
			m_string = s;
		}

		void SetPosition(wxPoint pt)
		{
			m_position = pt;
		}

		void SetBitmap(void);

	private:
		wxString m_string;
		wxSize m_size;
		wxPoint m_position;
		wxBitmap * m_pbm;
		wxColour m_back_color;
		wxColour m_text_color;
};

#endif
