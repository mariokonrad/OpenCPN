/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   bdbcat@yahoo.com                                                      *
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

#ifndef __GUI__TOOL__H__
#define __GUI__TOOL__H__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/gdicmn.h>

namespace gui {

class Tool
{
public:
	Tool(void);
	void Unload(void);

	wxString name;
	wxPoint iconLoc;
	wxPoint rolloverLoc;
	wxPoint disabledLoc;
	wxPoint activeLoc;
	wxBitmap icon;
	wxBitmap rollover;
	wxBitmap rolloverToggled;
	wxBitmap disabled;
	wxBitmap active;
	wxBitmap toggled;
	bool iconLoaded;
	bool rolloverLoaded;
	bool rolloverToggledLoaded;
	bool disabledLoaded;
	bool activeLoaded;
	bool toggledLoaded;
	wxSize customSize;
};

}

#endif
