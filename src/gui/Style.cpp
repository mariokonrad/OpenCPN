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

#include "Style.h"

#include <graphics/MergeBitmaps.h>

#include <gui/Icon.h>
#include <gui/Tool.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/dcmemory.h>
#include <wx/toolbar.h>

#include <cstdlib>

namespace gui {

Style::Style(const wxString& path)
	: graphics(NULL)
	, compassMarginTop(4)
	, compassMarginRight(0)
	, compassMarginBottom(4)
	, compassMarginLeft(4)
	, compasscornerRadius(3)
	, compassXoffset(0)
	, compassYoffset(0)
	, marginsInvisible(false)
	, chartStatusIconWidth(0)
	, chartStatusWindowTransparent(false)
	, embossHeight(40)
	, embossFont(wxEmptyString)
	, myConfigFileDir(path)
	, currentOrientation(0)
	, colorscheme(global::GLOBAL_COLOR_SCHEME_DAY)
	, hasBackground(false)
{
	for (int i = 0; i < 2; i++) {
		toolbarStartLoc[i] = wxPoint(0, 0);
		toolbarEndLoc[i] = wxPoint(0, 0);
		cornerRadius[i] = 0;
	}
}

Style::~Style()
{
	for (Tools::iterator tool = tools.begin(); tool != tools.end(); ++tool)
		delete *tool;
	tools.clear();

	for (Icons::iterator icon = icons.begin(); icon != icons.end(); ++icon)
		delete *icon;
	icons.clear();

	if (graphics) {
		delete graphics;
		graphics = NULL;
	}

	toolIndex.clear();
	iconIndex.clear();
}

bool Style::HasBackground() const
{
	return hasBackground;
}

int Style::GetTopMargin() const
{
	return toolMarginTop[currentOrientation];
}

int Style::GetRightMargin() const
{
	return toolMarginRight[currentOrientation];
}

int Style::GetBottomMargin() const
{
	return toolMarginBottom[currentOrientation];
}

int Style::GetLeftMargin() const
{
	return toolMarginLeft[currentOrientation];
}

int Style::GetCompassTopMargin() const
{
	return compassMarginTop;
}

int Style::GetCompassRightMargin() const
{
	return compassMarginRight;
}

int Style::GetCompassBottomMargin() const
{
	return compassMarginBottom;
}

int Style::GetCompassLeftMargin() const
{
	return compassMarginLeft;
}

int Style::GetCompassCornerRadius() const
{
	return compasscornerRadius;
}

int Style::GetCompassXOffset() const
{
	return compassXoffset;
}

int Style::GetCompassYOffset() const
{
	return compassYoffset;
}

int Style::GetToolSeparation() const
{
	return toolSeparation[currentOrientation];
}

wxSize Style::GetToolSize() const
{
	return toolSize[currentOrientation];
}

wxSize Style::GetToggledToolSize() const
{
	return toggledBGSize[currentOrientation];
}

bool Style::HasToolbarStart() const
{
	return toolbarStartLoc[currentOrientation] != wxPoint(0,0);
}

bool Style::HasToolbarEnd() const
{
	return toolbarEndLoc[currentOrientation] != wxPoint(0,0);
}

const wxBitmap* Style::getGraphics() const
{
	return graphics;
}

// Tools and Icons perform on-demand loading and dimming of bitmaps.
// Changing color scheme invalidatres all loaded bitmaps.

wxBitmap Style::GetIcon(const wxString& name)
{
	// FIXME: do not use lazy initialization, styles are initialized by StyleManager
	IndexMap::iterator index = iconIndex.find(name);

	if (index == iconIndex.end()) {
		wxLogMessage(_T("The requested icon was not found in the style: ") + name);
		return wxBitmap(GetToolSize().x, GetToolSize().y); // Prevents crashing.
	}

	Icon* icon = icons[index->second];

	if (icon->loaded)
		return icon->icon;

	// extract icon from bitmap
	if (icon->size.x == 0)
		icon->size = toolSize[currentOrientation];
	wxRect location(icon->iconLoc, icon->size);
	wxBitmap bm = graphics->GetSubBitmap(location);
	icon->icon = SetBitmapBrightness(bm);
	icon->loaded = true;

	return icon->icon;
}

wxBitmap Style::GetToolIcon(const wxString& toolname, int iconType, bool rollover)
{
	// FIXME: do not use lazy initialization, styles are initialized by StyleManager
	IndexMap::iterator index = toolIndex.find(toolname);

	if (index == toolIndex.end()) {
		return wxBitmap(GetToolSize().x, GetToolSize().y, 1);
	}

	Tool* tool = tools[index->second];

	switch (iconType) {
		case TOOLICON_NORMAL: {
			if (tool->iconLoaded && !rollover)
				return tool->icon;
			if (tool->rolloverLoaded && rollover)
				return tool->rollover;

			wxSize size = tool->customSize;
			if (size.x == 0)
				size = toolSize[currentOrientation];
			wxRect location(tool->iconLoc, size);

			// If rollover icon does not exist, use the defult icon
			if (rollover) {
				if ((tool->rolloverLoc.x != 0) || (tool->rolloverLoc.y != 0))
					location = wxRect(tool->rolloverLoc, size);
			}

			if (currentOrientation) {
				location.x -= verticalIconOffset.x;
				location.y -= verticalIconOffset.y;
			}

			wxBitmap bm = graphics->GetSubBitmap(location);
			if (hasBackground) {
				bm = graphics::MergeBitmaps(GetNormalBG(), bm, wxSize(0, 0));
			} else {
				wxBitmap bg(GetToolSize().x, GetToolSize().y);
				wxMemoryDC mdc(bg);
				mdc.SetBackground(wxBrush(global::OCPN::get().color().get_color(_T("GREY2")), wxSOLID));
				mdc.Clear();
				mdc.SelectObject(wxNullBitmap);
				bm = graphics::MergeBitmaps(bg, bm, wxSize(0, 0));
			}
			if (rollover) {
				tool->rollover = SetBitmapBrightness(bm);
				tool->rolloverLoaded = true;
				return tool->rollover;
			} else {
				if (toolname == _T("mob_btn")) {
					double dimLevel = 1.0;
					if (colorscheme == global::GLOBAL_COLOR_SCHEME_DUSK)
						dimLevel = 0.5;
					else if (colorscheme == global::GLOBAL_COLOR_SCHEME_NIGHT)
						dimLevel = 0.5;
					tool->icon = SetBitmapBrightnessAbs(bm, dimLevel);
				} else {
					tool->icon = SetBitmapBrightness(bm);
				}

				tool->iconLoaded = true;
				return tool->icon;
			}
		} break;

		case TOOLICON_TOGGLED: {
			if (tool->toggledLoaded && !rollover)
				return tool->toggled;
			if (tool->rolloverToggledLoaded && rollover)
				return tool->rolloverToggled;

			wxSize size = tool->customSize;
			if (size.x == 0)
				size = toolSize[currentOrientation];
			wxRect location(tool->iconLoc, size);
			if (rollover)
				location = wxRect(tool->rolloverLoc, size);
			wxSize offset(0, 0);
			if (GetToolSize() != GetToggledToolSize()) {
				offset = GetToggledToolSize() - GetToolSize();
				offset /= 2;
			}
			if (currentOrientation) {
				location.x -= verticalIconOffset.x;
				location.y -= verticalIconOffset.y;
			}
			wxBitmap bm = graphics::MergeBitmaps(GetToggledBG(), graphics->GetSubBitmap(location), offset);
			if (rollover) {
				tool->rolloverToggled = SetBitmapBrightness(bm);
				tool->rolloverToggledLoaded = true;
				return tool->rolloverToggled;
			} else {
				tool->toggled = SetBitmapBrightness(bm);
				tool->toggledLoaded = true;
				return tool->toggled;
			}
		} break;

		case TOOLICON_DISABLED: {
			if (tool->disabledLoaded)
				return tool->disabled;
			wxSize size = tool->customSize;
			if (size.x == 0)
				size = toolSize[currentOrientation];
			wxRect location(tool->disabledLoc, size);
			wxBitmap bm = graphics->GetSubBitmap(location);
			if (currentOrientation) {
				location.x -= verticalIconOffset.x;
				location.y -= verticalIconOffset.y;
			}
			if (hasBackground) {
				bm = graphics::MergeBitmaps(GetNormalBG(), bm, wxSize(0, 0));
			}
			tool->disabled = SetBitmapBrightness(bm);
			tool->disabledLoaded = true;
			return tool->disabled;
		} break;
	}
	wxLogMessage(_T("A requested icon type for this tool was not found in the style: ") + toolname);
	return wxBitmap(GetToolSize().x, GetToolSize().y); // Prevents crashing.
}

wxBitmap Style::BuildPluginIcon(const wxBitmap* bm, int iconType) const
{
	if (!bm || !bm->IsOk())
		return wxNullBitmap;

	wxBitmap iconbm;

	switch (iconType) {
		case TOOLICON_NORMAL:
			if (hasBackground) {
				wxBitmap bg = GetNormalBG();
				bg = SetBitmapBrightness(bg);
				wxSize offset
					= wxSize(bg.GetWidth() - bm->GetWidth(), bg.GetHeight() - bm->GetHeight());
				offset /= 2;
				iconbm = graphics::MergeBitmaps(bg, *bm, offset);
			} else {
				wxBitmap bg(GetToolSize().x, GetToolSize().y);
				wxMemoryDC mdc(bg);
				wxSize offset = GetToolSize() - wxSize(bm->GetWidth(), bm->GetHeight());
				offset /= 2;
				mdc.SetBackground(wxBrush(global::OCPN::get().color().get_color(_T("GREY2")), wxSOLID));
				mdc.Clear();
				mdc.SelectObject(wxNullBitmap);
				iconbm = graphics::MergeBitmaps(bg, *bm, offset);
			}
			break;

		case TOOLICON_TOGGLED:
			iconbm = graphics::MergeBitmaps(GetToggledBG(), *bm, wxSize(0, 0));
			break;
	}
	return iconbm;
}

wxBitmap Style::SetBitmapBrightness(wxBitmap& bitmap) const
{
	double dimLevel;
	switch (colorscheme) {
		case global::GLOBAL_COLOR_SCHEME_DUSK:
			dimLevel = 0.5;
			break;

		case global::GLOBAL_COLOR_SCHEME_NIGHT:
			dimLevel = 0.125;
			break;

		default:
			return bitmap;
	}

	return SetBitmapBrightnessAbs(bitmap, dimLevel);
}

wxBitmap Style::SetBitmapBrightnessAbs(wxBitmap& bitmap, double level) const
{
	wxImage image = bitmap.ConvertToImage();

	int gimg_width = image.GetWidth();
	int gimg_height = image.GetHeight();

	for (int iy = 0; iy < gimg_height; iy++) {
		for (int ix = 0; ix < gimg_width; ix++) {
			if (!image.IsTransparent(ix, iy, 30)) {
				wxImage::RGBValue rgb(image.GetRed(ix, iy), image.GetGreen(ix, iy),
									  image.GetBlue(ix, iy));
				wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
				hsv.value = hsv.value * level;
				wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
				image.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
			}
		}
	}
	return wxBitmap(image);
}

wxBitmap Style::GetNormalBG() const
{
	wxSize size = toolSize[currentOrientation];
	return graphics->GetSubBitmap(wxRect(normalBGlocation[currentOrientation].x,
										 normalBGlocation[currentOrientation].y, size.x, size.y));
}

wxBitmap Style::GetActiveBG() const
{
	return graphics->GetSubBitmap(
		wxRect(activeBGlocation[currentOrientation].x, activeBGlocation[currentOrientation].y,
			   toolSize[currentOrientation].x, toolSize[currentOrientation].y));
}

wxBitmap Style::GetToggledBG() const
{
	wxSize size = toolSize[currentOrientation];
	if (toggledBGSize[currentOrientation].x) {
		size = toggledBGSize[currentOrientation];
	}
	return graphics->GetSubBitmap(wxRect(toggledBGlocation[currentOrientation], size));
}

wxBitmap Style::GetToolbarStart() const
{
	wxSize size = toolbarStartSize[currentOrientation];
	if (toolbarStartSize[currentOrientation].x == 0) {
		size = toolbarStartSize[currentOrientation];
	}
	return graphics->GetSubBitmap(wxRect(toolbarStartLoc[currentOrientation], size));
}

wxBitmap Style::GetToolbarEnd() const
{
	wxSize size = toolbarEndSize[currentOrientation];
	if (toolbarEndSize[currentOrientation].x == 0) {
		size = toolbarEndSize[currentOrientation];
	}
	return graphics->GetSubBitmap(wxRect(toolbarEndLoc[currentOrientation], size));
}

int Style::GetToolbarCornerRadius() const
{
	return cornerRadius[currentOrientation];
}

void Style::DrawToolbarLineStart(wxBitmap& bmp) const
{
	if (!HasToolbarStart())
		return;
	wxMemoryDC dc(bmp);
	dc.DrawBitmap(GetToolbarStart(), 0, 0, true);
	dc.SelectObject(wxNullBitmap);
}

void Style::DrawToolbarLineEnd(wxBitmap& bmp) const
{
	if (!HasToolbarStart())
		return;
	wxMemoryDC dc(bmp);
	if (currentOrientation) {
		dc.DrawBitmap(GetToolbarEnd(), 0, bmp.GetHeight() - GetToolbarEnd().GetHeight(), true);
	} else {
		dc.DrawBitmap(GetToolbarEnd(), bmp.GetWidth() - GetToolbarEnd().GetWidth(), 0, true);
	}
	dc.SelectObject(wxNullBitmap);
}

void Style::SetOrientation(long orient)
{
	int newOrient = 0;
	if (orient == wxTB_VERTICAL)
		newOrient = 1;
	if (newOrient == currentOrientation)
		return;
	currentOrientation = newOrient;
	Unload();
}

int Style::GetOrientation() const
{
	return currentOrientation;
}

const wxString& Style::config_path() const
{
	return myConfigFileDir;
}

const wxString& Style::graphics_filename() const
{
	return graphicsFile;
}

void Style::SetColorScheme(global::ColorScheme cs)
{
	colorscheme = cs;
	Unload();

	if ((consoleTextBackgroundSize.x) && (consoleTextBackgroundSize.y)) {
		wxBitmap bm
			= graphics->GetSubBitmap(wxRect(consoleTextBackgroundLoc, consoleTextBackgroundSize));

		// The background bitmap in the icons file may be too small, so will grow it arbitrailly
		wxImage image = bm.ConvertToImage();
		image.Rescale(consoleTextBackgroundSize.GetX() * 2, consoleTextBackgroundSize.GetY() * 2,
					  wxIMAGE_QUALITY_NORMAL);
		wxBitmap bn(image);
		consoleTextBackground = SetBitmapBrightness(bn);
	}
}

void Style::Unload()
{
	for (Tools::iterator tool = tools.begin(); tool != tools.end(); ++tool)
		(*tool)->Unload();

	for (Icons::iterator icon = icons.begin(); icon != icons.end(); ++icon)
		(*icon)->Unload();
}

wxColour Style::getConsoleFontColor() const
{
	return consoleFontColor;
}

const wxBitmap & Style::getConsoleTextBackground() const
{
	return consoleTextBackground;
}

const wxString & Style::getEmbossFont() const
{
	return embossFont;
}

int Style::getEmbossHeight() const
{
	return embossHeight;
}

bool Style::isChartStatusWindowTransparent() const
{
	return chartStatusWindowTransparent;
}

int Style::getChartStatusIconWidth() const
{
	return chartStatusIconWidth;
}

const wxString & Style::getName() const
{
	return name;
}

bool Style::isMarginsInvisible() const
{
	return marginsInvisible;
}

bool Style::load_graphics()
{
	wxString path = config_path() + wxFileName::GetPathSeparator() + graphics_filename();

	if (!wxFileName::FileExists(path)) {
		wxLogMessage(_T("Styles graphics file not found: ") + path);
		return false;
	}

	wxImage img; // Only image does PNG LoadFile properly on GTK.
	if (!img.LoadFile(path, wxBITMAP_TYPE_PNG)) {
		wxLogMessage(_T("Styles graphics file failed to load: ") + path);
		return false;
	}

	graphics = new wxBitmap(img);
	set_console_background_image();

	// FIXME: load all icons of this style, should be done here not in GetIcon

	return true;
}

void Style::set_console_background_image()
{
	if ((consoleTextBackgroundSize.x) && (consoleTextBackgroundSize.y)) {
		consoleTextBackground = getGraphics()->GetSubBitmap(
			wxRect(consoleTextBackgroundLoc, consoleTextBackgroundSize));
	}
}

}

