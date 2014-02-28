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

#include "FontMgr.h"

#include <gui/FontDesc.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/gdicmn.h>
#include <wx/tokenzr.h>
#include <wx/font.h>
#include <wx/utils.h>

#include <locale>

FontMgr* FontMgr::instance = NULL;

FontMgr& FontMgr::Get()
{
	if (!instance)
		instance = new FontMgr;
	return *instance;
}

FontMgr::FontMgr()
{
}

FontMgr::~FontMgr()
{
	for (FontList::iterator i = fontlist.begin(); i != fontlist.end(); ++i)
		delete *i;
}

wxColour FontMgr::GetFontColor(const wxString& TextElement) const
{
	// Look thru the font list for a match

	for (FontList::const_iterator i = fontlist.begin(); i != fontlist.end(); ++i) {
		if ((*i)->m_dialogstring == TextElement)
			return (*i)->m_color;
	}

	return wxColour(0, 0, 0);
}

wxString FontMgr::GetFontConfigKey(const wxString& description)
{
	// Create the configstring by combining the locale with
	// a hash of the font description. Hash is used because the i18n
	// description can contain characters that mess up the config file.

	wxString configkey;
	configkey = global::OCPN::get().sys().data().locale;
	configkey.Append(_T("-"));

	using namespace std;
	locale loc;
	const collate<char>& coll = use_facet<collate<char> >(loc);
	char cFontDesc[101];
	wcstombs(cFontDesc, description.c_str(), 100);
	cFontDesc[100] = 0;
	int fdLen = strlen(cFontDesc);

	configkey.Append(wxString::Format(_T("%08lx"), coll.hash(cFontDesc, cFontDesc + fdLen)));
	return configkey;
}

wxFont* FontMgr::find_font(const wxString& text_element)
{
	// Look thru the font list for a match

	for (FontList::const_iterator i = fontlist.begin(); i != fontlist.end(); ++i) {
		if ((*i)->m_dialogstring == text_element)
			return (*i)->m_font;
	}
	return NULL;
}

wxFont* FontMgr::GetFont(const wxString& TextElement, int default_size)
{
	wxFont* font = find_font(TextElement);
	if (font)
		return font;

	// Found no font, so create a nice one and add to the list
	wxString configkey = GetFontConfigKey(TextElement);

	// Now create a benign, always present native string
	// Optional user requested default size
	int new_size = (default_size != 0) ? default_size : 12;

	wxString nativefont = GetSimpleNativeFont(new_size);
	wxFont* nf = wxFont::New(nativefont);

	gui::FontDesc* pnewfd = new gui::FontDesc(TextElement, configkey, nf, *wxBLACK);
	fontlist.push_back(pnewfd);

	return pnewfd->m_font;
}

wxString FontMgr::GetSimpleNativeFont(int size)
{
	// Now create a benign, always present native string
	wxString nativefont;

	// For those platforms which have no native font description string format
	nativefont.Printf(_T("%d;%d;%d;%d;%d;%d;%s;%d"), 0, // version
					  size, wxFONTFAMILY_DEFAULT, (int)wxFONTSTYLE_NORMAL, (int)wxFONTWEIGHT_NORMAL,
					  false, "", (int)wxFONTENCODING_DEFAULT);

// If we know of a detailed description string format, use it.
#ifdef __WXGTK__
	nativefont.Printf(_T("Fixed %2d"), size);
#endif

#ifdef __WXX11__
	nativefont = _T("0;-*-fixed-*-*-*-*-*-120-*-*-*-*-iso8859-1");
#endif

#ifdef __WXMSW__
	// nativefont = _T ( "0;-11;0;0;0;400;0;0;0;0;0;0;0;0;MS Sans Serif" );

	int h;
	int w;
	int hm;
	int wm;
	::wxDisplaySize(&w, &h); // pixels
	::wxDisplaySizeMM(&wm, &hm); // MM
	double pix_per_inch_v = wxMax(72.0, (h / hm) * 25.4);
	int lfHeight = -(int)((size * (pix_per_inch_v / 72.0)) + 0.5);

	// version, in case we want to change the format later
	nativefont.Printf( _T("%d;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%d;%d;%d;%d;"), 0,
			lfHeight, // lf.lfHeight
			0,        // lf.lfWidth,
			0,        // lf.lfEscapement,
			0,        // lf.lfOrientation,
			400,      // lf.lfWeight,
			0,        // lf.lfItalic,
			0,        // lf.lfUnderline,
			0,        // lf.lfStrikeOut,
			0,        // lf.lfCharSet,
			0,        // lf.lfOutPrecision,
			0,        // lf.lfClipPrecision,
			0,        // lf.lfQuality,
			0 );      // lf.lfPitchAndFamily,

	nativefont.Append(_T("Verdana"));
#endif

	return nativefont;
}

bool FontMgr::SetFont(const wxString& TextElement, wxFont* pFont, wxColour color)
{
	// Look thru the font list for a match
	for (FontList::iterator i = fontlist.begin(); i != fontlist.end(); ++i) {
		gui::FontDesc* pmfd = *i;
		if (pmfd->m_dialogstring == TextElement) {
			// TODO Think about this
			//
			// Cannot delete the present font, since it may be in use elsewhere
			// This WILL leak....but only on font changes
			//   delete pmfd->m_font; // purge any old value

			pmfd->m_font = pFont;
			pmfd->m_nativeInfo = pFont->GetNativeFontInfoDesc();
			pmfd->m_color = color;
			return true;
		}
	}

	return false;
}

int FontMgr::GetNumFonts(void) const
{
	return static_cast<int>(fontlist.size());
}

const wxString& FontMgr::GetConfigString(int i) const
{
	return fontlist.at(i)->m_configstring;
}

const wxString& FontMgr::GetDialogString(int i) const
{
	return fontlist.at(i)->m_dialogstring;
}

const wxString& FontMgr::GetNativeDesc(int i) const
{
	return fontlist.at(i)->m_nativeInfo;
}

wxString FontMgr::GetFullConfigDesc(int i) const
{
	const gui::FontDesc* pfd = fontlist.at(i);
	wxString ret = pfd->m_dialogstring;
	ret.Append(_T(":"));
	ret.Append(pfd->m_nativeInfo);
	ret.Append(_T(":"));

	wxString cols(_T("rgb(0,0,0)"));
	if (pfd->m_color.IsOk())
		cols = pfd->m_color.GetAsString(wxC2S_CSS_SYNTAX);

	ret.Append(cols);
	return ret;
}

void FontMgr::LoadFontNative(const wxString& ConfigString, const wxString& NativeDesc)
{
	// Parse the descriptor string
	wxStringTokenizer tk(NativeDesc, _T(":"));
	wxString dialogstring = tk.GetNextToken();
	wxString nativefont = tk.GetNextToken();
	wxColour color(tk.GetNextToken()); // from string description

	// Search for a match in the list
	for (FontList::iterator i = fontlist.begin(); i != fontlist.end(); ++i) {
		gui::FontDesc* pmfd = *i;
		if (pmfd->m_configstring == ConfigString) {
			pmfd->m_nativeInfo = nativefont;
			wxFont* nf = pmfd->m_font->New(pmfd->m_nativeInfo);
			pmfd->m_font = nf;
			return;
		}
	}

	// Create and add the font to the list
	wxFont* nf0 = new wxFont();
	wxFont* nf = nf0->New(nativefont);

	// Scrub the native font string for bad unicode conversion
#ifdef __WXMSW__
	wxString face = nf->GetFaceName();
	const wxChar* t = face.c_str();
	if (*t > 255) {
		delete nf;
		wxString substitute_native = GetSimpleNativeFont(12);
		nf = nf0->New(substitute_native);
	}
#endif
	delete nf0;

	gui::FontDesc* pnewfd = new gui::FontDesc(dialogstring, ConfigString, nf, color);
	fontlist.push_back(pnewfd);
}

