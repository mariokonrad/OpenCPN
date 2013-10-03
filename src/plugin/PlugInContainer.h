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

#ifndef __PLUGIN__PLUGINCONTAINER__H__
#define __PLUGIN__PLUGINCONTAINER__H__

#include <wx/string.h>
#include <plugin/ocpn_plugin.h>

class wxDynamicLibrary;
class wxBitmap;

class PlugInContainer
{
	public:
		PlugInContainer();

		opencpn_plugin * m_pplugin;
		bool m_bEnabled;
		bool m_bInitState;
		bool m_bToolboxPanel;
		int m_cap_flag;             // PlugIn Capabilities descriptor
		wxString m_plugin_file;          // The full file path
		destroy_t * m_destroy_fn;
		wxDynamicLibrary * m_plibrary;
		wxString m_common_name;            // A common name string for the plugin
		wxString m_short_description;
		wxString m_long_description;
		int m_api_version;
		int m_version_major;
		int m_version_minor;
		wxBitmap * m_bitmap;
};

WX_DEFINE_ARRAY_PTR(PlugInContainer *, ArrayOfPlugIns);

#endif
