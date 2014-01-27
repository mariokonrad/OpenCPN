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

#ifndef __PRINT__PRINTCELL__H__
#define __PRINT__PRINTCELL__H__

#include <wx/string.h>
#include <wx/gdicmn.h>

class wxDC;

namespace print {

/// \brief This class takes multilined string and modifies it to fit into given width
/// for given device. If it is too wide for given DC (by class PrintTable )
/// it introduces new lines between words
class PrintCell
{
protected:
	// Copy of printing device
	wxDC* dc;

	// Target width
	int width;

	// Target height
	int height;

	// Cellpadding
	int cellpadding;

	// Content of a cell
	wxString content;

	// Result of modification
	wxString modified_content;

	// Rect for printing of modified string
	wxRect rect;

	// Stores page, where this cell will be printed
	int page;

	// Stores, if one has to ovveride property "weight" of the font with the value "bold" - used to
	// print header of the table.
	bool bold_font;

	// Adjust text
	void Adjust();

public:
	// Constructor with content to print and device
	PrintCell();

	// Constructor with content to print and device
	void Init(const wxString& _content, wxDC* _dc, int _width, int _cellpadding,
			  bool bold_font = false);

	wxRect GetRect() const;
	wxString GetText() const;
	int GetHeight() const;
	int GetWidth() const;
	void SetPage(int _page);
	;
	void SetHeight(int _height);
	int GetPage() const;
};

}

#endif
