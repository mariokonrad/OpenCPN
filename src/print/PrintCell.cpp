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

#include "PrintCell.h"
#include <vector>
#include <wx/dc.h>
#include <wx/font.h>
#include <wx/tokenzr.h>

namespace print {

PrintCell::PrintCell()
{
}

void PrintCell::Init(const wxString& _content, wxDC* _dc, int _width, int _cellpadding,
					 bool _bold_font)
{
	bold_font = _bold_font;
	dc = _dc;
	width = _width;
	cellpadding = _cellpadding;
	content = _content;
	page = 1;
	Adjust();
};

void PrintCell::Adjust()
{
	wxFont orig_font = dc->GetFont();
	wxFont _font = orig_font;
	if (bold_font) {
		_font.SetWeight(wxFONTWEIGHT_BOLD);
	}
	dc->SetFont(_font);
	std::vector<wxString> list;
	list.push_back(wxString());
	wxString separator = wxT(" ");
	wxStringTokenizer tokenizer(content, separator, wxTOKEN_RET_DELIMS);
	int words_number = 0;
	while (tokenizer.HasMoreTokens()) {
		wxString token = tokenizer.GetNextToken();
		wxCoord h = 0;
		wxCoord w = 0;
		wxString tmp = list[list.size() - 1];
		wxString tmp2 = tmp + token;
		words_number++;
		dc->GetMultiLineTextExtent(tmp2, &w, &h);
		if ((w < width - 2 * cellpadding) || words_number == 1) {
			list[list.size() - 1] = tmp2;
		} else {
			list.push_back(wxString());
		}
	}

	for (size_t i = 0; i < list.size() - 1; i++) {
		modified_content = modified_content + list[i] + _T('\n');
	}
	// now add last element without new line
	modified_content = modified_content + list[list.size() - 1];

	wxCoord h = 0;
	wxCoord w = 0;
	dc->GetMultiLineTextExtent(modified_content, &w, &h);
	SetHeight(h + 8);

	dc->SetFont(orig_font);
}

wxRect PrintCell::GetRect() const
{
	return rect;
}

// Returns modified cell content
wxString PrintCell::GetText() const
{
	return modified_content;
}

// Returns height of the cell
int PrintCell::GetHeight() const
{
	return height;
}

// Returns width of the cell
int PrintCell::GetWidth() const
{
	return width;
}

// sets the page to print
void PrintCell::SetPage(int _page)
{
	page = _page;
}

// sets the height
void PrintCell::SetHeight(int _height)
{
	height = _height;
}

// Returns the page, where this element should be painted
int PrintCell::GetPage() const
{
	return page;
}

}

