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

#ifndef __STYLES__H__
#define __STYLES__H__

#include <wx/bitmap.h>
#include <wx/string.h>
#include <global/ColorScheme.h>
#include <vector>

namespace ocpnStyle {

class Icon;
class Tool;

WX_DECLARE_STRING_HASH_MAP(int, intHash);

enum StyleToolIconTypes
{
	TOOLICON_NORMAL,
	TOOLICON_TOGGLED,
	TOOLICON_DISABLED,
	TOOLICON_ACTIVE
};

wxBitmap MergeBitmaps(wxBitmap back, wxBitmap front, wxSize offset);
wxBitmap ConvertTo24Bit(wxColor bgColor, wxBitmap front);

class Style
{
		// because StyleManager is doing the initialization, it has to be friend.
		friend class StyleManager;

	public:
		Style(void);
		~Style(void);

		wxBitmap GetNormalBG() const;
		wxBitmap GetActiveBG() const;
		wxBitmap GetToggledBG() const;
		wxBitmap GetToolbarStart() const;
		wxBitmap GetToolbarEnd() const;
		bool HasBackground() const;
		wxBitmap GetIcon(const wxString & name);
		wxBitmap GetToolIcon(const wxString & toolname, int iconType = TOOLICON_NORMAL, bool rollover = false);
		wxBitmap BuildPluginIcon(const wxBitmap * bm, int iconType) const;

		int GetTopMargin() const;
		int GetRightMargin() const;
		int GetBottomMargin() const;
		int GetLeftMargin() const;
		int GetToolbarCornerRadius() const;

		int GetCompassTopMargin() const;
		int GetCompassRightMargin() const;
		int GetCompassBottomMargin() const;
		int GetCompassLeftMargin() const;
		int GetCompassCornerRadius() const;
		int GetCompassXOffset() const;
		int GetCompassYOffset() const;

		int GetToolSeparation() const;
		wxSize GetToolSize() const;
		wxSize GetToggledToolSize() const;

		bool HasToolbarStart() const;
		bool HasToolbarEnd() const;
		void DrawToolbarLineStart(wxBitmap & bmp) const;
		void DrawToolbarLineEnd(wxBitmap & bmp) const;

		wxBitmap SetBitmapBrightness(wxBitmap& bitmap) const;
		wxBitmap SetBitmapBrightnessAbs(wxBitmap& bitmap, double level) const;

		int GetOrientation() const;

		void SetOrientation(long orient);
		void SetColorScheme(global::ColorScheme cs);
		void Unload();

		wxColour getConsoleFontColor() const;
		const wxBitmap & getConsoleTextBackground() const;
		const wxString & getEmbossFont() const;
		int getEmbossHeight() const;
		bool isChartStatusWindowTransparent() const;
		int getChartStatusIconWidth() const;
		const wxString & getName() const;
		bool isMarginsInvisible() const;

	private:
		typedef std::vector<Icon *> Icons;
		typedef std::vector<Tool *> Tools;

		wxString name;
		wxString description;
		wxString graphicsFile;
		int toolMarginTop[2];
		int toolMarginRight[2];
		int toolMarginBottom[2];
		int toolMarginLeft[2];
		int toolSeparation[2];
		int cornerRadius[2];
		int compassMarginTop;
		int compassMarginRight;
		int compassMarginBottom;
		int compassMarginLeft;
		int compasscornerRadius;
		int compassXoffset;
		int compassYoffset;

		wxSize toolSize[2];
		wxSize toggledBGSize[2];
		wxPoint toggledBGlocation[2];
		wxPoint activeBGlocation[2];
		wxPoint normalBGlocation[2];
		wxSize verticalIconOffset;
		intHash toolIndex;
		Icons icons;
		intHash iconIndex;
		wxBitmap * graphics;

		wxColor consoleFontColor;
		wxPoint consoleTextBackgroundLoc;
		wxSize consoleTextBackgroundSize;
		wxPoint toolbarStartLoc[2];
		wxSize toolbarStartSize[2];
		wxPoint toolbarEndLoc[2];
		wxSize toolbarEndSize[2];
		wxBitmap consoleTextBackground;
		wxBitmap toolbarStart[2];
		wxBitmap toolbarEnd[2];

		bool marginsInvisible;

		int chartStatusIconWidth;
		bool chartStatusWindowTransparent;

		wxString embossFont;
		int embossHeight;

		wxString myConfigFileDir;

		Tools tools;

		int currentOrientation;
		global::ColorScheme colorscheme;
		bool hasBackground;
};
}

#endif
