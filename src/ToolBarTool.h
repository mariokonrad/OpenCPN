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

#ifndef __TOOLBARTOOL__H__
#define __TOOLBARTOOL__H__

#include <wx/toolbar.h>
#include <wx/tbarbase.h>

class ToolBarSimple;

class ToolBarTool : public wxToolBarToolBase
{
	public:
		ToolBarTool(
				ToolBarSimple * tbar,
				int id,
				const wxString & label,
				const wxBitmap & bmpNormal,
				const wxBitmap & bmpRollover,
				wxItemKind kind,
				wxObject * clientData,
				const wxString & shortHelp,
				const wxString & longHelp);

		void SetSize(const wxSize & size);
		wxCoord GetWidth() const;
		wxCoord GetHeight() const;
		wxString GetToolname() const;
		void SetIconName(const wxString & name);
		wxString GetIconName() const;

		wxCoord m_x;
		wxCoord m_y;
		wxCoord m_width;
		wxCoord m_height;
		wxRect trect;
		wxString toolname;
		wxString iconName;
		const wxBitmap * pluginNormalIcon;
		const wxBitmap * pluginRolloverIcon;
		bool firstInLine;
		bool lastInLine;
		bool rollover;
		bool bitmapOK;
		bool isPluginTool;
		bool b_hilite;
};

#endif
