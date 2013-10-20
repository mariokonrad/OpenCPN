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
#include <Icon.h>
#include <Tool.h>
#include <UserColors.h>

#include <tinyxml/tinyxml.h>

#include <cstdlib>

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/dcmemory.h>
#include <wx/toolbar.h>

namespace ocpnStyle {

// This function can be used to create custom bitmap blending for platforms
// where 32 bit bitmap ops are broken. Can hopefully be removed for wxWidgets 3.0...

wxBitmap MergeBitmaps(wxBitmap back, wxBitmap front, wxSize offset)
{
	wxBitmap merged( back.GetWidth(), back.GetHeight(), back.GetDepth() );
#if (!wxCHECK_VERSION(2,9,4) && (defined(__WXGTK__) || defined(__WXMAC__)))

	// Manual alpha blending for broken wxWidgets platforms.
	merged.UseAlpha();
	back.UseAlpha();
	front.UseAlpha();

	wxImage im_front = front.ConvertToImage();
	wxImage im_back = back.ConvertToImage();
	wxImage im_result = back.ConvertToImage();// Only way to make result have alpha channel in wxW 2.8.

	unsigned char *presult = im_result.GetData();
	unsigned char *pback = im_back.GetData();
	unsigned char *pfront = im_front.GetData();

	unsigned char *afront = NULL;
	if( im_front.HasAlpha() )
		afront = im_front.GetAlpha();

	unsigned char *aback = NULL;
	if( im_back.HasAlpha() )
		aback = im_back.GetAlpha();

	unsigned char *aresult = NULL;
	if( im_result.HasAlpha() )
		aresult = im_result.GetAlpha();

	// Do alpha blending, associative version of "over" operator.

	for( int i = 0; i < back.GetHeight(); i++ ) {
		for( int j = 0; j < back.GetWidth(); j++ ) {

			int fX = j - offset.x;
			int fY = i - offset.y;

			bool inFront = true;
			if( fX < 0 || fY < 0 ) inFront = false;
			if( fX >= front.GetWidth() ) inFront = false;
			if( fY >= front.GetHeight() ) inFront = false;

			if( inFront ) {
				double alphaF = (double) ( *afront++ ) / 256.0;
				double alphaB = (double) ( *aback++ ) / 256.0;
				double alphaRes = alphaF + alphaB * ( 1.0 - alphaF );
				unsigned char a = alphaRes * 256;
				*aresult++ = a;
				unsigned char r = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
				*presult++ = r;
				unsigned char g = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
				*presult++ = g;
				unsigned char b = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
				*presult++ = b;
			} else {
				*aresult++ = *aback++;
				*presult++ = *pback++;
				*presult++ = *pback++;
				*presult++ = *pback++;
			}
		}
	}

	merged = wxBitmap( im_result );

#else
	wxMemoryDC mdc( merged );
	mdc.DrawBitmap( back, 0, 0, true );
	mdc.DrawBitmap( front, offset.x, offset.y, true );
	mdc.SelectObject( wxNullBitmap );
#endif

	return merged;
}

// The purpouse of ConvertTo24Bit is to take an icon with 32 bit depth and alpha
// channel and put it in a 24 bit deep bitmap with no alpha, that can be safely
// drawn in the crappy wxWindows implementations.

wxBitmap ConvertTo24Bit( wxColor bgColor, wxBitmap front )
{
	if( front.GetDepth() == 24 ) return front;

	wxBitmap result( front.GetWidth(), front.GetHeight(), 24 );
#if !wxCHECK_VERSION(2,9,4)
	front.UseAlpha();
#endif

	wxImage im_front = front.ConvertToImage();
	wxImage im_result = result.ConvertToImage();

	unsigned char *presult = im_result.GetData();
	unsigned char *pfront = im_front.GetData();

	unsigned char *afront = NULL;
	if( im_front.HasAlpha() )
		afront = im_front.GetAlpha();

	for( int i = 0; i < result.GetWidth(); i++ ) {
		for( int j = 0; j < result.GetHeight(); j++ ) {

			double alphaF = (double) ( *afront++ ) / 256.0;
			unsigned char r = *pfront++ * alphaF + bgColor.Red() * ( 1.0 - alphaF );
			*presult++ = r;
			unsigned char g = *pfront++ * alphaF + bgColor.Green() * ( 1.0 - alphaF );
			*presult++ = g;
			unsigned char b = *pfront++ * alphaF + bgColor.Blue() * ( 1.0 - alphaF );
			*presult++ = b;
		}
	}

	result = wxBitmap( im_result );
	return result;
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

// Tools and Icons perform on-demand loading and dimming of bitmaps.
// Changing color scheme invalidatres all loaded bitmaps.

wxBitmap Style::GetIcon(const wxString & name)
{
	if( iconIndex.find( name ) == iconIndex.end() ) {
		wxString msg( _T("The requested icon was not found in the style: ") );
		msg += name;
		wxLogMessage( msg );
		return wxBitmap( GetToolSize().x, GetToolSize().y ); // Prevents crashing.
	}

	int index = iconIndex[name]; // FIXME: this operation is not const but should be

	Icon * icon = (Icon*) icons.Item(index); // FIXME: do not use void* array for icons

	if( icon->loaded ) return icon->icon;
	if( icon->size.x == 0 ) icon->size = toolSize[currentOrientation];
	wxRect location( icon->iconLoc, icon->size );
	wxBitmap bm = graphics->GetSubBitmap( location );
	icon->icon = SetBitmapBrightness( bm );
	icon->loaded = true;
	return icon->icon;
}

wxBitmap Style::GetToolIcon(const wxString & toolname, int iconType, bool rollover)
{

	if( toolIndex.find( toolname ) == toolIndex.end() ) {
		//  This will produce a flood of log messages for some PlugIns, notably WMM_PI, and GRADAR_PI
		//        wxString msg( _T("The requested tool was not found in the style: ") );
		//        msg += toolname;
		//        wxLogMessage( msg );
		return wxBitmap( GetToolSize().x, GetToolSize().y, 1 );
	}

	int index = toolIndex[toolname]; // FIXME: do not use void* arrays for Tools
	Tool* tool = (Tool*) tools.Item( index );

	switch( iconType ){
		case TOOLICON_NORMAL:
			{
				if( tool->iconLoaded && !rollover )
					return tool->icon;
				if( tool->rolloverLoaded && rollover )
					return tool->rollover;

				wxSize size = tool->customSize;
				if( size.x == 0 )
					size = toolSize[currentOrientation];
				wxRect location( tool->iconLoc, size );

				//  If rollover icon does not exist, use the defult icon
				if( rollover ) {
					if( (tool->rolloverLoc.x != 0) || (tool->rolloverLoc.y != 0) )
						location = wxRect( tool->rolloverLoc, size );
				}

				if( currentOrientation ) {
					location.x -= verticalIconOffset.x;
					location.y -= verticalIconOffset.y;
				}

				wxBitmap bm = graphics->GetSubBitmap( location );
				if( hasBackground ) {
					bm = MergeBitmaps( GetNormalBG(), bm, wxSize( 0, 0 ) );
				} else {
					wxBitmap bg( GetToolSize().x, GetToolSize().y );
					wxMemoryDC mdc( bg );
					mdc.SetBackground( wxBrush( GetGlobalColor( _T("GREY2") ), wxSOLID ) );
					mdc.Clear();
					mdc.SelectObject( wxNullBitmap );
					bm = MergeBitmaps( bg, bm, wxSize( 0, 0 ) );
				}
				if( rollover ) {
					tool->rollover = SetBitmapBrightness( bm );
					tool->rolloverLoaded = true;
					return tool->rollover;
				} else {
					if( toolname == _T("mob_btn") ) {
						double dimLevel = 1.0;
						if(colorscheme ==  GLOBAL_COLOR_SCHEME_DUSK)
							dimLevel = 0.5;
						else if(colorscheme ==  GLOBAL_COLOR_SCHEME_NIGHT)
							dimLevel = 0.5;
						tool->icon = SetBitmapBrightnessAbs( bm, dimLevel );
					} else {
						tool->icon = SetBitmapBrightness( bm );
					}

					tool->iconLoaded = true;
					return tool->icon;
				}
			}
			break;

		case TOOLICON_TOGGLED:
			{
				if( tool->toggledLoaded && !rollover )
					return tool->toggled;
				if( tool->rolloverToggledLoaded && rollover )
					return tool->rolloverToggled;

				wxSize size = tool->customSize;
				if( size.x == 0 )
					size = toolSize[currentOrientation];
				wxRect location( tool->iconLoc, size );
				if( rollover )
					location = wxRect( tool->rolloverLoc, size );
				wxSize offset( 0, 0 );
				if( GetToolSize() != GetToggledToolSize() ) {
					offset = GetToggledToolSize() - GetToolSize();
					offset /= 2;
				}
				if( currentOrientation ) {
					location.x -= verticalIconOffset.x;
					location.y -= verticalIconOffset.y;
				}
				wxBitmap bm = MergeBitmaps(GetToggledBG(), graphics->GetSubBitmap(location), offset);
				if( rollover ) {
					tool->rolloverToggled = SetBitmapBrightness( bm );
					tool->rolloverToggledLoaded = true;
					return tool->rolloverToggled;
				} else {
					tool->toggled = SetBitmapBrightness( bm );
					tool->toggledLoaded = true;
					return tool->toggled;
				}
			}
			break;

		case TOOLICON_DISABLED:
			{
				if( tool->disabledLoaded )
					return tool->disabled;
				wxSize size = tool->customSize;
				if( size.x == 0 )
					size = toolSize[currentOrientation];
				wxRect location( tool->disabledLoc, size );
				wxBitmap bm = graphics->GetSubBitmap( location );
				if( currentOrientation ) {
					location.x -= verticalIconOffset.x;
					location.y -= verticalIconOffset.y;
				}
				if( hasBackground ) {
					bm = MergeBitmaps( GetNormalBG(), bm, wxSize( 0, 0 ) );
				}
				tool->disabled = SetBitmapBrightness( bm );
				tool->disabledLoaded = true;
				return tool->disabled;
			}
			break;
	}
	wxLogMessage(_T("A requested icon type for this tool was not found in the style: ") + toolname);
	return wxBitmap( GetToolSize().x, GetToolSize().y ); // Prevents crashing.
}

wxBitmap Style::BuildPluginIcon( const wxBitmap* bm, int iconType ) const
{
	if( ! bm || ! bm->IsOk() ) return wxNullBitmap;

	wxBitmap iconbm;

	switch( iconType ){
		case TOOLICON_NORMAL:
			if( hasBackground ) {
				wxBitmap bg = GetNormalBG();
				bg = SetBitmapBrightness( bg );
				wxSize offset = wxSize( bg.GetWidth() - bm->GetWidth(), bg.GetHeight() - bm->GetHeight() );
				offset /= 2;
				iconbm = MergeBitmaps( bg, *bm, offset );
			} else {
				wxBitmap bg( GetToolSize().x, GetToolSize().y );
				wxMemoryDC mdc( bg );
				wxSize offset = GetToolSize() - wxSize( bm->GetWidth(), bm->GetHeight() );
				offset /= 2;
				mdc.SetBackground( wxBrush( GetGlobalColor( _T("GREY2") ), wxSOLID ) );
				mdc.Clear();
				mdc.SelectObject( wxNullBitmap );
				iconbm = MergeBitmaps( bg, *bm, offset );
			}
			break;

		case TOOLICON_TOGGLED:
			iconbm = MergeBitmaps( GetToggledBG(), *bm, wxSize( 0, 0 ) );
			break;
	}
	return iconbm;
}

wxBitmap Style::SetBitmapBrightness( wxBitmap& bitmap ) const
{
	double dimLevel;
	switch( colorscheme ){
		case GLOBAL_COLOR_SCHEME_DUSK:
			dimLevel = 0.5;
			break;

		case GLOBAL_COLOR_SCHEME_NIGHT:
			dimLevel = 0.125;
			break;

		default:
			return bitmap;
	}

	return SetBitmapBrightnessAbs(bitmap, dimLevel);
}

wxBitmap Style::SetBitmapBrightnessAbs( wxBitmap& bitmap, double level ) const
{
	wxImage image = bitmap.ConvertToImage();

	int gimg_width = image.GetWidth();
	int gimg_height = image.GetHeight();

	for( int iy = 0; iy < gimg_height; iy++ ) {
		for( int ix = 0; ix < gimg_width; ix++ ) {
			if( !image.IsTransparent( ix, iy, 30 ) ) {
				wxImage::RGBValue rgb( image.GetRed( ix, iy ), image.GetGreen( ix, iy ),
						image.GetBlue( ix, iy ) );
				wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
				hsv.value = hsv.value * level;
				wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
				image.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
			}
		}
	}
	return wxBitmap( image );
}

wxBitmap Style::GetNormalBG() const
{
	wxSize size = toolSize[currentOrientation];
	return graphics->GetSubBitmap(
			wxRect( normalBGlocation[currentOrientation].x, normalBGlocation[currentOrientation].y,
				size.x, size.y ) );
}

wxBitmap Style::GetActiveBG() const
{
	return graphics->GetSubBitmap(
			wxRect( activeBGlocation[currentOrientation].x, activeBGlocation[currentOrientation].y,
				toolSize[currentOrientation].x, toolSize[currentOrientation].y ) );
}

wxBitmap Style::GetToggledBG() const
{
	wxSize size = toolSize[currentOrientation];
	if( toggledBGSize[currentOrientation].x ) {
		size = toggledBGSize[currentOrientation];
	}
	return graphics->GetSubBitmap( wxRect( toggledBGlocation[currentOrientation], size ) );
}

wxBitmap Style::GetToolbarStart() const
{
	wxSize size = toolbarStartSize[currentOrientation];
	if( toolbarStartSize[currentOrientation].x == 0 ) {
		size = toolbarStartSize[currentOrientation];
	}
	return graphics->GetSubBitmap( wxRect( toolbarStartLoc[currentOrientation], size ) );
}

wxBitmap Style::GetToolbarEnd() const
{
	wxSize size = toolbarEndSize[currentOrientation];
	if( toolbarEndSize[currentOrientation].x == 0 ) {
		size = toolbarEndSize[currentOrientation];
	}
	return graphics->GetSubBitmap( wxRect( toolbarEndLoc[currentOrientation], size ) );
}

int Style::GetToolbarCornerRadius() const
{
	return cornerRadius[currentOrientation];
}

void Style::DrawToolbarLineStart(wxBitmap & bmp) const
{
	if (!HasToolbarStart())
		return;
	wxMemoryDC dc( bmp );
	dc.DrawBitmap( GetToolbarStart(), 0, 0, true );
	dc.SelectObject(wxNullBitmap);
}

void Style::DrawToolbarLineEnd( wxBitmap& bmp ) const
{
	if (!HasToolbarStart())
		return;
	wxMemoryDC dc( bmp );
	if( currentOrientation ) {
		dc.DrawBitmap( GetToolbarEnd(), 0, bmp.GetHeight() - GetToolbarEnd().GetHeight(), true );
	} else {
		dc.DrawBitmap( GetToolbarEnd(), bmp.GetWidth() - GetToolbarEnd().GetWidth(), 0, true );
	}
	dc.SelectObject( wxNullBitmap );
}

void Style::SetOrientation( long orient )
{
	int newOrient = 0;
	if (orient == wxTB_VERTICAL)
		newOrient = 1;
	if( newOrient == currentOrientation )
		return;
	currentOrientation = newOrient;
	Unload();
}

int Style::GetOrientation() const
{
	return currentOrientation;
}

void Style::SetColorScheme( ColorScheme cs )
{
	colorscheme = cs;
	Unload();

	if( (consoleTextBackgroundSize.x) && (consoleTextBackgroundSize.y)) {
		wxBitmap bm = graphics->GetSubBitmap(
				wxRect( consoleTextBackgroundLoc, consoleTextBackgroundSize ) );

		// The background bitmap in the icons file may be too small, so will grow it arbitrailly
		wxImage image = bm.ConvertToImage();
		image.Rescale( consoleTextBackgroundSize.GetX() * 2, consoleTextBackgroundSize.GetY() * 2 , wxIMAGE_QUALITY_NORMAL );
		wxBitmap bn( image );
		consoleTextBackground = SetBitmapBrightness( bn );
	}
}

void Style::Unload()
{
	for( unsigned int i = 0; i < tools.Count(); i++ ) {
		Tool* tool = (Tool*) tools.Item( i );
		tool->Unload();
	}

	for( unsigned int i = 0; i < icons.Count(); i++ ) {
		Icon* icon = (Icon*) icons.Item( i );
		icon->Unload();
	}
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

Style::Style(void)
	: graphics(NULL)
{
	currentOrientation = 0;
	colorscheme = GLOBAL_COLOR_SCHEME_DAY;
	marginsInvisible = false;
	hasBackground = false;
	chartStatusIconWidth = 0;
	chartStatusWindowTransparent = false;
	embossHeight = 40;
	embossFont = wxEmptyString;

	//  Set compass window style defauilts
	compassMarginTop = 4;
	compassMarginRight = 0;
	compassMarginBottom = 4;
	compassMarginLeft = 4;
	compasscornerRadius = 3;
	compassXoffset = 0;
	compassYoffset = 0;

	for( int i = 0; i < 2; i++ ) {
		toolbarStartLoc[i] = wxPoint( 0, 0 );
		toolbarEndLoc[i] = wxPoint( 0, 0 );
		cornerRadius[i] = 0;
	}
}

Style::~Style(void)
{
	for( unsigned int i = 0; i < tools.Count(); i++ ) {
		delete (Tool*) ( tools.Item( i ) );
	}
	tools.Clear();

	for( unsigned int i = 0; i < icons.Count(); i++ ) {
		delete (Icon*) ( icons.Item( i ) );
	}
	icons.Clear();

	if (graphics) {
		delete graphics;
		graphics = NULL;
	}

	toolIndex.clear();
	iconIndex.clear();
}

}

