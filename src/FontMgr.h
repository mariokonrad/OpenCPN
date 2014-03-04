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

#ifndef __FONTMGR__H__
#define __FONTMGR__H__

#include <gui/FontManager.h>

#include <vector>

namespace gui { class FontDesc; }

/// Manages the font list.
class FontMgr : public gui::FontManager
{
public:
	FontMgr();
	virtual ~FontMgr();

	virtual const wxString& GetConfigString(int i) const;
	virtual const wxString& GetDialogString(int i) const;
	virtual wxFont* GetFont(const wxString& TextElement, int default_size = 0);
	virtual wxColour GetFontColor(const wxString& TextElement) const;
	virtual wxString GetFullConfigDesc(int i) const;
	virtual int GetNumFonts(void) const;
	virtual void LoadFontNative(const wxString& ConfigString, const wxString& NativeDesc);
	virtual bool SetFont(const wxString& TextElement, wxFont* pFont, wxColour color);
	virtual wxString GetFontConfigKey(const wxString& description) const;

private:
	wxFont* find_font(const wxString& text_element);
	wxString GetSimpleNativeFont(int size);

	typedef std::vector<gui::FontDesc*> FontList;
	FontList fontlist;
};

#endif
