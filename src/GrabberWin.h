/****************************************************************************
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

#ifndef __GRABBERWIN__H__
#define __GRABBERWIN__H__

#include <wx/panel.h>
#include <wx/bitmap.h>
#include "navutil.h"
#include "styles.h"

class GrabberWin : public wxPanel
{
		DECLARE_EVENT_TABLE()

	public:
		GrabberWin(wxWindow * parent);
		void OnPaint(wxPaintEvent & event);
		void MouseEvent(wxMouseEvent & event);
		void SetColorScheme(ColorScheme cs);

		wxBitmap m_pbitmap;
		bool m_bLeftDown;
		bool m_bRightDown;
		ocpnStyle::Style * m_style;
};

#endif
