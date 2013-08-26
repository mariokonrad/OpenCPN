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

#ifndef __PLUGIN__PLUGINTOOLBARTOOLCONTAINER__H__
#define __PLUGIN__PLUGINTOOLBARTOOLCONTAINER__H__

#include <wx/string.h>

class wxBitmap;
class wxObject;
class opencpn_plugin;

class PlugInToolbarToolContainer
{
	public:
		PlugInToolbarToolContainer();
		~PlugInToolbarToolContainer();

		opencpn_plugin * m_pplugin;
		int id;
		wxString label;
		wxBitmap * bitmap_day;
		wxBitmap * bitmap_dusk;
		wxBitmap * bitmap_night;
		wxBitmap * bitmap_Rollover;

		wxItemKind kind;
		wxString shortHelp;
		wxString longHelp;
		wxObject * clientData;
		int position;
		bool b_viz;
		bool b_toggle;
		int tool_sel;
};

#endif
