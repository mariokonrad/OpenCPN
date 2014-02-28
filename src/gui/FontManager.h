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

#ifndef __GUI__FONTMANAGER__H__
#define __GUI__FONTMANAGER__H__

#include <wx/colour.h>
#include <wx/string.h>

class wxFont;

namespace gui {

/// Interface for font managers.
class FontManager
{
public:
	virtual ~FontManager()
	{}

	virtual const wxString& GetConfigString(int i) const = 0;
	virtual const wxString& GetDialogString(int i) const = 0;
	virtual wxFont* GetFont(const wxString& TextElement, int default_size = 0) = 0;
	virtual wxColour GetFontColor(const wxString& TextElement) const = 0;
	virtual wxString GetFullConfigDesc(int i) const = 0;
	virtual int GetNumFonts(void) const = 0;
	virtual void LoadFontNative(const wxString& ConfigString, const wxString& NativeDesc) = 0;
	virtual bool SetFont(const wxString& TextElement, wxFont* pFont, wxColour color) = 0;
	virtual wxString GetFontConfigKey(const wxString& description) const = 0;
};

}

#endif
