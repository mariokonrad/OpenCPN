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

#ifndef __THUMBWIN_H__
#define __THUMBWIN_H__


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

class ChartBase;

class ThumbWin : public wxWindow
{
		DECLARE_EVENT_TABLE()

	private:
		wxSize m_max_size;

	public:
		wxBitmap * pThumbShowing;
		ChartBase * pThumbChart;

	private:
		void OnPaint(wxPaintEvent& event);

	public:
		ThumbWin();
		ThumbWin(wxWindow * parent);
		virtual ~ThumbWin();

		void Resize(void);
		void SetMaxSize(wxSize const & max_size);
};

#endif
