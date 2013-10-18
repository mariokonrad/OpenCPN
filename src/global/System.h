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

#ifndef __GLOBAL__SYSTEM__H__
#define __GLOBAL__SYSTEM__H__

#include <wx/string.h>

namespace global {

class System
{
	public:
		virtual ~System() {}

	public:

		struct Data
		{
			wxString home_location;
			wxString private_data_dir;
			wxString tc_data_dir;
			wxString config_file;
			wxString log_file;
			wxString sound_data_location;
			wxString world_map_location;
		};

		virtual const Data & data() const = 0;
		virtual void set_home_location(const wxString &) = 0;
		virtual void set_private_data_dir(const wxString &) = 0;
		virtual void set_tc_data_dir(const wxString &) = 0;
		virtual void set_config_file(const wxString &) = 0;
		virtual void set_log_file(const wxString &) = 0;
		virtual void set_sound_data_location(const wxString &) = 0;
		virtual void set_world_map_location(const wxString &) = 0;

	public:

		struct Config
		{
			wxString version_string;
			bool nav_message_shown;
		};

		virtual const Config & config() const = 0;
		virtual void set_config_version_string(const wxString &) = 0;
		virtual void set_config_nav_message_shown(bool) = 0;
};

}

#endif
