/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#ifndef __USERCOLORS__H__
#define __USERCOLORS__H__

#include <wx/colour.h>
#include <wx/string.h>

#include <global/ColorManager.h>

void setup_current_user_color(const wxString &);
void InitializeUserColors(void);
void DeInitializeUserColors(void);
wxColour GetGlobalColor(wxString);

class UserColors : public global::ColorManager
{
public:
	UserColors();
	virtual ~UserColors();

	virtual void inject_chart_color_provider(global::ColorProvider*);
	virtual wxColour get_color(const wxString& color_name) const;
	virtual void set_current(global::ColorScheme scheme);
	virtual global::ColorScheme get_current() const;

private:
	global::ColorScheme color_scheme;
	global::ColorProvider* chart_color_provider;
};

#endif
